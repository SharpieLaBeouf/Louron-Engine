#pragma once

// Louron Core Headers
#include "Spatial Partitioning/OctreeBounds.h"
#include "Components/Core Components.h"

#include "../Asset/Asset.h"
#include "../Core/Logging.h"
#include "../Physics/CollisionCallback.h"
#include "../Renderer/Camera.h"
#include "../OpenGL/Vertex Array.h"
#include "../OpenGL/Framebuffer.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <filesystem>

// External Vendor Library Headers
#include <entt/entt.hpp>
#include <PxPhysicsAPI.h>

constexpr int MAX_BONE_TRANSFORMATIONS = 204800; // 13.11 mega bytes of VRAM, can support generally near 1024 bones based on 200 bones per animated character

constexpr int MAX_DIRECTIONAL_LIGHTS = 10;
constexpr int MAX_POINT_LIGHTS = 1024;
constexpr int MAX_SPOT_LIGHTS = 1024;

constexpr int SHADOW_MAP_TEXTURE_RESOLUTION = 2048;
constexpr int MAX_POINT_LIGHT_SHADOW_MAPS = 5;
constexpr int MAX_SPOT_LIGHT_SHADOW_MAPS = 30;
constexpr int MAX_DIRE_LIGHT_SHADOW_MAPS = 5;

using namespace physx;

namespace Louron {

	// Forward Declarations
	class Entity;
	class Prefab;
	class RenderPipeline;
	class FrameBuffer;
	struct FrameBufferConfig;
	class UUID;
	class JobCounter;

	enum L_RENDER_PIPELINE : uint8_t;

	struct SceneConfig {

		std::string Name = "Untitled Scene";
		std::filesystem::path SceneFilePath;

		L_RENDER_PIPELINE ScenePipelineType;
	};

	class Scene : public Asset {

	public:

		Scene();
		Scene(L_RENDER_PIPELINE pipeline);
		~Scene() = default;

		bool LoadSceneFile(const std::filesystem::path& sceneFilePath);

	public:

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntity(UUID uuid, const std::string& name = "");

		Entity InstantiatePrefab(std::shared_ptr<Prefab> prefab, std::optional<TransformComponent> transform = std::nullopt, const UUID& parent_uuid = NULL_UUID);

		Entity DuplicateEntity(Entity entity, Entity parent, std::shared_ptr<std::unordered_map<UUID, UUID>> reference_map = nullptr);
		void DestroyEntity(const UUID& entity_uuid);
		void DestroyEntity(Entity entity, std::unique_lock<std::mutex>* parent_lock = nullptr);
		
		Entity FindEntityByName(std::string_view name);
		Entity FindEntityByUUID(UUID uuid);
		bool HasEntityByUUID(UUID uuid);

		bool HasEntity(const Entity& entity);
		bool HasEntity(const std::string& name);
		bool HasEntity(const UUID& uuid);

		bool ValidEntity(const Entity& entity);

	public:

		bool IsRunning() const { return m_IsRunning; }
		bool IsSimulating() const { return m_IsSimulating; }
		bool IsPaused() const { return m_IsPaused; }
		bool IsPhysicsCalculating() const { return m_IsPhysicsCalculating; }

		void OnStart();
		void OnStop();

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnPhysicsStart();
		void OnPhysicsStop();

		void OnUpdate();
		void OnRuntimeRender();
		void OnEditorRender(EditorCamera* editor_camera, const std::vector<Entity>& scene_cameras);

		void OnUpdateGUI();

		void OnFixedUpdate();
		void OnLateUpdate();

		void OnViewportResize(const glm::ivec2& new_size);

		void SetPaused(bool is_paused) { m_IsPaused = is_paused; }

	public:

		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }
		
		Entity GetPrimaryCameraEntity();

		void SetConfig(const SceneConfig& config) { m_SceneConfig = config; }
		const SceneConfig& GetConfig() const { return m_SceneConfig; }
		
		entt::registry* GetRegistry() { return &m_Registry; }
		bool CopyRegistry(std::shared_ptr<Scene> otherScene);

		PxScene* GetPhysScene() const { return m_PhysxScene; }
		void SetPhysScene(PxScene* physScene);

		virtual AssetType GetType() const override { return AssetType::Scene; }

		void CreateSceneFrameBuffer(const FrameBufferConfig& framebuffer_config);
		const std::unique_ptr<FrameBuffer>& GetSceneFrameBuffer() const;

		void SetSceneFilePath(const std::filesystem::path& path) { m_SceneConfig.SceneFilePath = path; }
		const std::filesystem::path& GetSceneFilePath() const { return m_SceneConfig.SceneFilePath; }

		void SetDisplayOctree(bool display) { m_DisplayOctree = display; }
		const bool& GetDisplayOctree() const { return m_DisplayOctree; }

		std::weak_ptr<OctreeBounds<Entity>> GetOctree() const { return m_Octree; }

		static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> source_scene);

	private:

		void CombineCameraTargets(const std::vector<std::tuple<FrameBuffer*, glm::vec4, uint8_t>>& camera_targets);
		void DrawCameraTextureToSceneFramebuffer();

		entt::registry m_Registry;
		std::shared_ptr<std::map<UUID, entt::entity>> m_EntityMap = std::make_shared<std::map<UUID, entt::entity>>();

		PxScene* m_PhysxScene = nullptr;
		std::unique_ptr<CollisionCallback> m_CollisionCallback = nullptr;

		std::unique_ptr<FrameBuffer> m_SceneFrameBuffer = nullptr;
		std::unique_ptr<VertexArray> m_ScreenQuadVAO = nullptr;

		bool m_IsRunning = false;
		bool m_IsSimulating = false;
		bool m_IsPaused = false;
		bool m_IsPhysicsCalculating = false;

		SceneConfig m_SceneConfig;

		std::shared_ptr<OctreeBounds<Entity>> m_Octree = nullptr;
		bool m_DisplayOctree = false;

		std::mutex m_BoneUpdateMutex = {};
		std::shared_ptr<JobCounter> m_LastFrameAnimationUpdateCounter = nullptr;
		std::unordered_map<UUID, glm::mat4> m_BoneUpdates = {};
		std::atomic<bool> m_AnimationThreadFinished = true;

		friend class Entity;
		friend class Project;
		friend class SceneSerializer;

		friend class TransformSystem;
		friend class PhysicsSystem;
	};
}