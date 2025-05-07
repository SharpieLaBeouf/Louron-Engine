#pragma once

// Louron Core Headers
#include "../Scene/Entity.h"
#include "../Scene/Components/Core Components.h"
#include "../Scene/Components/Light Components.h"
#include "../Scene/Components/Mesh Components.h"

#include "../OpenGL/Material.h"
#include "../OpenGL/Query.h"
#include "../OpenGL/Vertex Array.h"
#include "../OpenGL/Framebuffer.h"
#include "../Scene/Spatial Partitioning/Frustum.h"
#include "../Scene/Spatial Partitioning/OctreeBounds.h"

// C++ Standard Library Headers
#include <memory>
#include <unordered_set>
#include <thread>
#include <mutex>

// External Vendor Library Headers
#include <glad/glad.h>

// So we can Key a Pair of Material and Uniform Block
struct _MaterialWrapper {
	std::shared_ptr<Louron::Material> material;
	std::shared_ptr<Louron::MaterialUniformBlock> uniform_block;

	// Compare the actual contents, not just pointers
	bool operator==(const _MaterialWrapper& other) const {
		return material.get() == other.material.get() && uniform_block.get() == other.uniform_block.get();
	}
};

namespace std {
	template <>
	struct hash<_MaterialWrapper> {
		std::size_t operator()(const _MaterialWrapper& mw) const {
			return std::hash<std::shared_ptr<Louron::Material>>{}(mw.material) ^ (std::hash<std::shared_ptr<Louron::MaterialUniformBlock>>{}(mw.uniform_block) << 1);
		}
	};
}

namespace Louron {

	enum L_RENDER_PIPELINE : uint8_t {
		FORWARD = 0,
		FORWARD_PLUS = 1,
		DEFERRED = 2
	};

	class Scene;
	class CameraBase;

	struct StaticMesh;
	struct SubMesh;
	struct Bounds_AABB;
	struct Bounds_Sphere;

	using DepthRenderQueue = std::vector<std::tuple<float, Entity>>;
	using OpaqueRenderQueue = std::unordered_map<_MaterialWrapper, std::unordered_map<std::shared_ptr<SubMesh>, std::vector<Entity>>>;
	using TransparentRenderQueue = std::vector<std::tuple<float, _MaterialWrapper, std::shared_ptr<SubMesh>, Entity>>;
	
	using GeometryQueryMap = std::unordered_map<UUID, Query>;
	using GeometryQueryHistory = std::unordered_map<UUID, uint8_t>;

	class ForwardPlusPipeline {

	private:

		struct SceneContext;
		struct CameraContext;

	public:

		/// <summary>
		/// Set OpenGL state configuration required by renderer and FP_Data and Light SSBOs.
		/// </summary>
		static void OnStartPipeline(Scene* scene);

		/// <summary>
		/// Reset OpenGL state configuration required by renderer and clean FP_Data and Light SSBOs.
		/// </summary>
		static void OnStopPipeline(Scene* scene);

		/// <summary>
		/// This is the main loop for rendering logic
		/// in the Forward+ Pipeline for all Scene
		/// Cameras that are visible.
		/// </summary>
		static void OnRenderScene(Scene* scene);

		/// <summary>
		/// This is the main loop for rendering logic
		/// in the Forward+ Pipeline for the Editor 
		/// Camera.
		/// 
		/// You can also pass a list of scene camera 
		/// entities you want to render aswell as the 
		/// Editor.
		/// </summary>
		static void OnRenderEditorScene(Scene* scene, const std::vector<Entity>& scene_cameras, EditorCamera* editor_camera);

		/// <summary>
		/// This will update required FP_Data for any viewport resizing.
		/// </summary>
		static void OnViewportResize(Scene* scene, const glm::uvec2& viewport_size);

		/// <summary>
		/// Get vector of camera entity UUIDs that are active for rendering this frame.
		/// </summary>
		static std::vector<Entity> GetActiveCameras(Scene* scene);

	private:

		struct CameraContext
		{

			//
			//	RENDER QUEUES
			//	 - Render Queues are prepared from prior frame
			//
			OpaqueRenderQueue		Opaque_RenderQueue = {};											// Final Opaque Render Queue to be Executed in Following Frame - sorted by material, then sub mesh, with buckets of entities for render
			TransparentRenderQueue	Transparent_RenderQueue = {};										// Final Transparent Render Queue to be Executed in Following Frame - sorted back to front
			DepthRenderQueue		Depth_RenderQueue = {};												// Final Depth Render Queue to be Executed in Following Frame - sorted front to back
			std::vector<glm::mat4>	Debug_RenderAABBs = {};												// Transforms of Debug AABBs to Render

			//
			//	Camera Information
			//
			Frustum		CameraFrustum = {};																// Camera Frustum
			UUID		CameraEntityUUID = NULL_UUID;													// Reference of Camera Entity UUID
			glm::uvec2	TileWorkGroups = { -1, -1 };													// Screen Space Tile WorkGroups

			glm::mat4 Projection_Matrix = glm::mat4(1.0f);
			glm::mat4 View_Matrix = glm::mat4(1.0f);
			glm::vec3 Position = glm::vec3(0.0f);
			float NearClip = 0.01f;
			float FarClip = 1000.0f;

			glm::uvec2 FrameBufferSize = { -1, -1 };

			//
			//	Occlusion Culling - Queries and History of Successful Queries
			//
			GeometryQueryMap		EntityOcclusionQueries = {};										// Current Query Object per Renderable Entity
			GeometryQueryHistory	EntityOcclusionHistory = {};										// How Long Do We Keep Visible Before Querying Again?

			//
			//	Base Frustum Visibilty - Lights & Renderable Entities
			//
			std::vector<Entity>	Entities_Visible = {};													// All Renderable Entities in Frustum && Not Occluded
			std::vector<Entity>	PointLights_FrustumVisible = {};										// All Point Lights Intersecting Frustum
			std::vector<Entity>	SpotLights_FrustumVisible = {};											// All Spot Lights Intersecting Frustum
			std::vector<Entity>	DirectionalLights = {};													// All Directional Lights

			//
			//	Light Indice SSBOs
			//
			GLuint PointLight_Indices_Buffer = -1;														// Indices of Point Lights for Camera Tiles
			GLuint SpotLight_Indices_Buffer = -1;														// Indices of Spot Lights for Camera Tiles
			
			//
			//	Directional Cascaded Shadow Mapping
			//
			GLuint DirectionalLight_Shadow_FrameBuffer = -1;											// Directional Light Shadow Framebuffer	
			GLuint DirectionalLight_Shadow_Texture_Array = -1;											// Directional Light Texture Array
			GLuint DirectionalLight_Shadow_Matrix_Buffer = -1;											// Buffer that holds light space matrices for each directional light cascade that is dependent on the camera's frustum

			//
			//	General
			//
			std::unordered_map<UUID, uint32_t>				DirectionalLight_Shadow_LightIndices = {};	// Key = Light Entity UUID, Value = Index of Light in Shadow Matrix Buffer
			std::unordered_map<UUID, std::array<float, 5>>	DirectionalLight_Shadow_CascadeIndices = {};// Key = Light Entity UUID, Value = Array of Cascade Plane Distances
			

			//
			//	Constructors and Operators
			//
			CameraContext() = default;
			CameraContext(Scene* scene, EditorCamera* editor_camera);
			CameraContext(Scene* scene, const Entity* camera_entity, const FrameBuffer& frame_buffer);
			~CameraContext();
			CameraContext(const CameraContext& other) = delete;
			CameraContext(CameraContext&& other) noexcept = default;
			CameraContext& operator=(const CameraContext& other) = delete;
			CameraContext& operator=(CameraContext&& other) noexcept = default;

			//
			//	Functions
			//
			void ValidateSceneCameraContext(Scene* scene);
			void ValidateEditorCameraContext(Scene* scene, EditorCamera* editor_camera);
			void OnRender(Scene* scene, FrameBuffer* camera_framebuffer);

		private:

			void InitCameraContext(const FrameBuffer& frame_buffer);

			void DepthPass(Scene* scene, FrameBuffer* camera_framebuffer);
			void RenderPass(Scene* scene, FrameBuffer* camera_framebuffer);

			/// <summary>
			/// Frustum Culls All Lights in a Camera.
			/// </summary>
			void Light_FrustumBasedCull(Scene* scene, SceneContext* scene_context);

			/// <summary>
			/// Conducts main tiled rendering algorithm, split screen into tiles
			/// and determine which lights impact each tile frustum.
			/// </summary>
			void Light_ComputeTileBasedCull(Scene* scene, FrameBuffer* camera_framebuffer) const;

			/// <summary>
			/// Updates the Directional Light Data SSBO per Camera.
			/// </summary>
			void Light_UpdateCameraContextSSBOs(Scene* scene);

			/// <summary>
			/// Frustum Culls All Objects in a Camera.
			/// </summary>
			void Renderable_FrustumCull(Scene* scene, SceneContext* scene_context);

			/// <summary>
			/// Uses Occlusion Culling to Remove Occluded Objects in Camera.
			/// </summary>
			void Renderable_OcclusionCull(Scene* scene, SceneContext* scene_context);

			/// <summary>
			/// Sorts and prepares the Depth Pass.
			/// </summary>
			void PrepareRenderPass_Depth(Scene* scene, SceneContext* scene_context);

			/// <summary>
			/// Sorts and prepares the Colour Passes.
			/// </summary>
			void PrepareRenderPass_Colour(Scene* scene, SceneContext* scene_context);

			friend struct SceneContext;

		};

		struct SceneContext
		{
			//
			//	SSBOs
			//
			GLuint PointLight_Data_Buffer = -1;													// SSBO that holds all point light data
			GLuint SpotLight_Data_Buffer = -1;													// SSBO that holds all spot light data
			GLuint DirectionalLight_Data_Buffer = -1;											// SSBO that holds all directional light data

			GLuint BoneTransform_Data_Buffer = -1;												// SSBO that holds all bone transform matrices

			//
			//	Worker Threads
			//
			std::thread OctreeUpdate_Thread;													// Transfers State Changes from ECS -> Octree

			//
			//	Shadow Mapping
			//
			GLuint PointLight_Shadow_FrameBuffer = -1;											// Point Light Shadow Framebuffer
			GLuint PointLight_Shadow_CubeMapArray = -1;											// Point Light CubeMap Texture Array
			std::unordered_map<UUID, uint32_t> PointLight_Shadow_LightIndices = {};				// Key = Light Entity UUID, Value = Index of Light in PL CubeMap Texture Array

			GLuint SpotLight_Shadow_FrameBuffer = -1;											// Spot Light Shadow Framebuffer
			GLuint SpotLight_Shadow_TextureArray = -1;											// Spot Light Texture Array
			GLuint SpotLight_Shadow_Matrix_Buffer = -1;											// Buffer that holds light space matrice for each spot light
			std::unordered_map<UUID, uint32_t> SpotLight_Shadow_LightIndices = {};				// Key = Light Entity UUID, Value = Index of Light in SL Texture Array

			//
			//	General
			//
			std::unordered_set<UUID> Entities_OverallVisible = {};								// Collection of all cameras visible point lights
			std::unordered_set<UUID> PointLight_OverallVisible = {};							// Collection of all cameras visible point lights
			std::unordered_set<UUID> SpotLight_OverallVisible = {};								// Collection of all cameras visible spot lights
			std::unordered_map<UUID, uint32_t> BoneTransform_Offset = {};						// Offset of Bone Transforms for Entity in SSBO. This is for the shader to index the correct transforms for the appropriate skinned mesh render commands.
			std::unordered_map<UUID, std::unique_ptr<CameraContext>> CameraContexts = {};		// Key = Camera Entity UUID, Value = Camera Context for Specific Camera
			std::unordered_map<AssetHandle, std::weak_ptr<StaticMesh>> CachedMeshes		= {};	// Fast Caching of Meshes Easing Off AssetManager::GetAsset calls
			std::unordered_map<AssetHandle, std::weak_ptr<Material>> CachedMaterials	= {};	// Fast Caching of Materials Easing Off AssetManager::GetAsset calls

			//
			//	Debug
			//
			bool Debug_ShowLightComplexity	= false;
			bool Debug_RenderWireframe		= false;

			//
			//	Constructors and Operators
			//
			SceneContext() = delete;
			SceneContext(Scene* scene);
			~SceneContext();
			SceneContext(const SceneContext& other) = delete;
			SceneContext(SceneContext&& other) noexcept = delete;
			SceneContext& operator=(const SceneContext& other) = delete;
			SceneContext& operator=(SceneContext&& other) noexcept = delete;

			//
			//	Scene Functions
			//

			/// <summary>
			/// Prepares the Render Queue for Depth and Colour Passes
			/// </summary>
			void SortRenderQueue(Scene* scene, const std::vector<CameraContext*>& active_cameras);

			/// <summary>
			/// When viewport resizes, tiled indice buffers 
			/// for cameras will need to be updated.
			/// </summary>
			void UpdateComputeData(Scene* scene);

			/// <summary>
			/// Updates Point and Spot light global SSBO, as 
			/// these are independent from cameras unlike 
			/// directional lights which have shadow mapping
			/// tied to the camera's cascades.
			/// </summary>
			void UpdateGlobalLightSSBOs(Scene* scene);

			/// <summary>
			/// This will clear and fill the Bone transformation SSBO
			/// </summary>
			void UpdateGlobalBoneSSBO(Scene* scene);

			/// <summary>
			/// Perform shadow mapping for all light sources
			/// with shadow casting enabled.
			/// 
			/// This will be for global lights (point & shadow)
			/// and camera dependent lights (directional).
			/// 
			/// This will only perform shadow mapping for visible
			/// cameras on the final viewport.
			/// </summary>
			void UpdateGlobalShadowMaps(Scene* scene, const std::vector<CameraContext*>& active_cameras);

			//
			//	Thread Functions
			//

			/// <summary>
			/// Updates the Octree with Changes in Scene ECS.
			/// </summary>
			void OnOctreeUpdate(Scene* scene);

		private:

			friend class ForwardPlusPipeline;

		};

	public:

		static SceneContext* GetSceneContext(Scene* scene);

	private:

		static inline std::unordered_map<Scene*, std::unique_ptr<SceneContext>> s_SceneContext;
	};


	class ForwardPipeline {

	public:

		ForwardPipeline() = default;

		static void OnRenderScene(Scene* scene);
		static void OnRenderEditorScene(Scene* scene, const std::vector<Entity>& scene_cameras, EditorCamera* editor_camera);
		static void OnStartPipeline(Scene* scene);
		static void OnStopPipeline(Scene* scene);

		static void OnViewportResize(Scene* scene, const glm::uvec2& viewport_size);

		static std::vector<Entity> GetActiveCameras(Scene* scene);

	private:

	};

	class DeferredRenderPipeline {

	public:

		DeferredRenderPipeline() = default;

		static void OnRenderScene(Scene* scene);
		static void OnRenderEditorScene(Scene* scene, const std::vector<Entity>& scene_cameras, EditorCamera* editor_camera);
		static void OnStartPipeline(Scene* scene);
		static void OnStopPipeline(Scene* scene);

		static void OnViewportResize(Scene* scene, const glm::uvec2& viewport_size);

		static std::vector<Entity> GetActiveCameras(Scene* scene);

	private:

	};

}