#pragma once

#include <memory>

#include "../Engine Callbacks.h"

#include "../Defines/LAssets.h"
#include "../Defines/LTypes.h"
#include "../Engine Util/LDebug.h"

namespace Louron
{
	class IScript;
	class Entity;

	namespace Components
	{
		class Component
		{

		public:

			Component() : m_EntityID(NULL_UUID) { }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::Unknown; }

		private:

			void SetEntity(uint32_t entity_uuid) { m_EntityID = entity_uuid; }
			friend class ::Louron::Entity;
		
		protected:

			Component(uint32_t entity_uuid) : m_EntityID(entity_uuid) {}
			
			uint32_t m_EntityID;
			
		};

		/**
		* @brief Component to access script information.
		*
		* This allows the user to access data associated with a script component.
		*/
		class ScriptComponent : public Component
		{

		public:

			/**
			* @brief Get the instance of the script.
			* 
			* If this ScriptComponent contains an active script with the specified name, this will return the instance of the script.
			* 
			* This is useful for inter-script between various systems and required behaviours.
			* 
			* The caller is responsible for casting this to the correct IScript type.
			* 
			* @param script_name The name of the script to get.
			* @return IScript* The instance of the script. If no script is found, this will return nullptr.
			* 
			* @code
			* 
			* // One Entity Contains Both a ScriptOne and ScriptTwo
			* // Script One Will Initialise, Then Script Two Will Set the bool to true.
			* 
			* class ScriptOne : public IScript {
			* 
			* public:
			*     
			*     bool my_bool = false;     
			* 
			* };
			* 
			* class ScriptTwo : public IScript {
			* 
			* public:
			* 
			*     void OnCreate() override
			*     {
			*         ScriptOne* script_one = reinterpret_cast<ScriptOne*>(GetComponent<ScriptComponent>().GetScriptInstance("ScriptOne"));
			*         if(script_one)
			*             script_one->my_bool = true;
			*     }
			* };
			* 
			* @endcode
			*/
			IScript* GetScriptInstance(const std::string& script_name) const;

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::ScriptComponent; }

		};

		/**
		* @brief Component to access camera information.
		*
		* This allows the user to access data associated with a camera component.
		*/
		class CameraComponent : public Component
		{

		public:

			/**
			* @brief Defines the method by which the camera clears the screen.
			*/
			enum class CameraClearFlags : uint8_t {
				COLOUR_ONLY = 0,  ///< Clear with a solid color.
				SKYBOX = 1        ///< Render the skybox instead of a clear color.
			};

			/**
			* @brief Gets the current clear flag mode.
			*
			* @return The clear flag mode set for the camera.
			*
			* @code
			* if (camera.GetClearFlag() == CameraClearFlags::SKYBOX)
			*     Debug::Log("Using skybox clear.");
			* @endcode
			*/
			CameraClearFlags GetClearFlag() const { return ENGINE_SAFE_CALL_RET(CameraClearFlags, CameraClearFlags(*)(uint32_t), CameraComponent_GetClearFlag, m_EntityID); }

			/**
			* @brief Sets the clear flag mode for the camera.
			*
			* @param flag The clear flag to use (e.g., COLOUR_ONLY or SKYBOX).
			*
			* @code
			* camera.SetClearFlag(CameraClearFlags::COLOUR_ONLY);
			* @endcode
			*/
			void SetClearFlag(CameraClearFlags flag) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), CameraComponent_SetClearFlag, m_EntityID, static_cast<uint8_t>(flag)); }

			/**
			* @brief Gets the camera's clear colour.
			*
			* @return The RGBA colour used to clear the screen.
			*
			* @code
			* Vectors::Vector4 colour = camera.GetClearColour();
			* @endcode
			*/
			Vectors::Vector4 GetClearColour() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector4, Vectors::Vector4(*)(uint32_t), CameraComponent_GetClearColour, m_EntityID); }

			/**
			* @brief Sets the camera's clear colour.
			*
			* @param colour The RGBA colour to use for clearing the screen. These values should be in the range [0.0f, 1.0f].
			*
			* @code
			* camera.SetClearColour({ 0.2f, 0.3f, 0.4f, 1.0f });
			* @endcode
			*/
			void SetClearColour(Vectors::Vector4 colour) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4&), CameraComponent_SetClearColour, m_EntityID, colour); }

			/**
			* @brief Gets the camera's render depth.
			*
			* Determines the render order when multiple cameras are active.
			*
			* @return The depth value (lower = rendered behind, higher = rendered in-front).
			*
			* @code
			* uint8_t depth = camera.GetCameraDepth();
			* @endcode
			*/
			uint8_t GetCameraDepth() const { return ENGINE_SAFE_CALL_RET(uint8_t, uint8_t(*)(uint32_t), CameraComponent_GetCameraDepth, m_EntityID); }
			
			/**
			* @brief Sets the camera's render depth.
			*
			* @param depth The depth value (lower = rendered behind, higher = rendered in-front).
			*
			* @code
			* camera.SetCameraDepth(2);
			* @endcode
			*/
			void SetCameraDepth(uint8_t depth) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), CameraComponent_SetCameraDepth, m_EntityID, depth); }

			/**
			* @brief Checks if the camera is rendering to the main viewport.
			*
			* @return True if the camera outputs to the screen, false otherwise.
			*
			* @code
			* if (camera.IsDisplayingToViewport())
			*     Debug::Log("This camera is shown on screen.");
			* @endcode
			*/
			bool IsDisplayingToViewport() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), CameraComponent_GetDisplayingToViewport, m_EntityID); }
			
			/**
			* @brief Sets whether the camera should render to the main viewport.
			*
			* @param display True to render to screen, false to render offscreen only.
			*
			* @code
			* camera.SetDisplayingToViewport(true);
			* @endcode
			*/
			void SetDisplayingToViewport(bool display) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), CameraComponent_SetDisplayingToViewport, m_EntityID, display); }

			/**
			* @brief Gets the camera's viewport rectangle.
			*
			* Returns the viewport as a Vector4: (x, y, width, height).
			*
			* @return The camera's normalized viewport dimensions.
			*
			* @code
			* auto viewport = camera.GetViewport(); // x, y, width, height
			* @endcode
			*/
			Vectors::Vector4 GetViewport() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector4, Vectors::Vector4(*)(uint32_t), CameraComponent_GetViewport, m_EntityID); }

			/**
			* @brief Sets the camera's viewport rectangle.
			*
			* The values are normalized from 0 to 1, relative to the screen dimensions.
			*
			* @param viewport_dimensions A Vector4 of (x, y, width, height).
			*
			* @code
			* camera.SetViewport({ 0.0f, 0.0f, 0.5f, 0.5f }); // Bottom-left quadrant
			* @endcode
			*/
			void SetViewport(Vectors::Vector4 viewport_dimensions) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*), CameraComponent_SetViewport, m_EntityID, &viewport_dimensions); }
			/**
			* @brief Gets the position of the viewport.
			*
			* @return The bottom-left corner of the viewport in normalized screen space.
			*/
			Vectors::Vector2 GetViewportPos() const { auto viewport = GetViewport(); return { viewport.x, viewport.y }; }

			/**
			* @brief Gets the size of the viewport.
			*
			* @return The width and height of the viewport in normalized screen space.
			*/
			Vectors::Vector2 GetViewportSize() const { auto viewport = GetViewport(); return { viewport.z, viewport.w }; }

			// TODO: Implement FrameBuffer, Getting Camera FrameBuffer, Blitting FrameBuffers, Writing to FrameBuffers, Reading from FrameBuffers, etc.

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::CameraComponent; }

		};

		/**
		* @brief Component to access audio listener information.
		*
		* This allows the user to access data associated with an audio listener component.
		*/
		class AudioListenerComponent : public Component
		{

		public:

			// TODO: Implement

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::AudioListenerComponent; }

		};

		/**
		* @brief Component to access audio emitter information.
		*
		* This allows the user to access data associated with an audio emitter component.
		*/
		class AudioEmitterComponent : public Component
		{

		public:

			// TODO: Implement

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::AudioEmitterComponent; }

		};

		/**
		* @brief Component to access mesh filter information.
		*
		* This allows the user to access data associated with a mesh filter component.
		*/
		class MeshFilterComponent : public Component
		{

		public:

			/**
			* @brief Gets the static mesh asset assigned to this entity.
			*
			* @return A handle-wrapped StaticMesh asset used for rendering.
			*
			* @code
			* Assets::StaticMesh mesh = mesh_filter.GetMeshAsset();
			* @endcode
			*/
			Assets::StaticMesh GetMeshAsset() const { Assets::StaticMesh asset = Assets::StaticMesh(false); asset.SetAssetHandle(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), MeshFilterComponent_GetMeshAssetHandle, m_EntityID)); return asset; }
			
			/**
			* @brief Sets the static mesh asset for this entity.
			*
			* @param static_mesh The mesh asset to assign to this component.
			*
			* @code
			* mesh_filter.SetMeshAsset(myMesh);
			* @endcode
			*/
			void SetMeshAsset(Assets::StaticMesh static_mesh) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t), MeshFilterComponent_SetMeshAssetHandle, m_EntityID, static_mesh.operator uint32_t()); }
			
			/**
			* @brief Gets the axis-aligned bounding box (AABB) of the mesh.
			*
			* @return The bounding box that encompasses the mesh.
			*
			* @code
			* Bounds_AABB bounds = mesh_filter.GetMeshBounds();
			* @endcode
			*/
			Partitions::Bounds_AABB GetMeshBounds() const { return ENGINE_SAFE_CALL_RET(Partitions::Bounds_AABB, Partitions::Bounds_AABB(*)(uint32_t), MeshFilterComponent_GetMeshBounds, m_EntityID);  }

			/**
			* @brief Sets the bounding box of the mesh manually.
			*
			* @param bounds The new axis-aligned bounding box to assign.
			*
			* @code
			* mesh_filter.SetMeshBounds({Vector3(-1.0f), Vector3(1.0f)});
			* @endcode
			*/
			void SetMeshBounds(const Partitions::Bounds_AABB& bounds) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Partitions::Bounds_AABB&), MeshFilterComponent_SetMeshBounds, m_EntityID, bounds); }

			/**
			* @brief Creates and sets a deep copy of the current mesh asset.
			*
			* @return A new instance of the StaticMesh asset.
			*
			* @note This is useful when you want to modify the mesh without affecting other entities using the original.
			*
			* @code
			* StaticMesh unique_mesh = mesh_filter.CopyAndSetMeshAsset();
			* uniqueMesh.SetVertices(...);
			* @endcode
			*/
			Assets::StaticMesh CopyAndSetMeshAsset() const { Assets::StaticMesh asset = Assets::StaticMesh(false); asset.SetAssetHandle(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), MeshFilterComponent_CopyMesh, m_EntityID)); return asset; }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::MeshFilterComponent; }

		};

		/**
		* @brief Component to access mesh renderer information.
		*
		* This allows the user to access data associated with a mesh renderer component.
		*/
		class MeshRendererComponent : public Component
		{

		public:

			/**
			* @brief Checks if this renderer is active.
			*
			* @return True if the renderer is enabled, false otherwise.
			*
			* @code
			* if (mesh_renderer.IsActive())
			*     Debug::Log("Renderer is enabled");
			* @endcode
			*/
			bool IsActive() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), MeshRendererComponent_GetIsActive, m_EntityID); }

			/**
			* @brief Enables or disables the mesh renderer.
			*
			* @param active Set to true to activate, false to disable.
			*
			* @code
			* mesh_renderer.SetActive(true);
			* @endcode
			*/
			void SetActive(bool active) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), MeshRendererComponent_SetIsActive, m_EntityID, active); }
			
			/**
			* @brief Checks if the renderer casts shadows.
			*
			* @return True if shadow casting is enabled.
			*
			* @code
			* if (mesh_renderer.IsCastingShadows())
			*     Debug::Log("Casting shadows!");
			* @endcode
			*/
			bool IsCastingShadows() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), MeshRendererComponent_GetIsCastingShadows, m_EntityID); }

			/**
			* @brief Enables or disables shadow casting for this renderer.
			*
			* @param casting_shadows True to enable shadow casting.
			*
			* @code
			* mesh_renderer.SetCastingShadows(false);
			* @endcode
			*/
			void SetCastingShadows(bool casting_shadows) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), MeshRendererComponent_SetCastingShadows, m_EntityID, casting_shadows); }

			/**
			* @brief Gets all materials currently assigned to the renderer.
			*
			* @return A vector of material asset handles.
			*
			* @code
			* auto materials = mesh_renderer.GetMaterials();
			* 
			* for(const auto& material : materials)
			* {
			*     Debug::Log("Material ID: " + std::to_string((uint32_t)material));
			* }
			* @endcode
			*/
			std::vector<Assets::Material> GetMaterials() const
			{
				std::vector<Assets::Material> material_vector{};

				size_t material_count = 0;
				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t*), MeshRendererComponent_GetAllMaterialsCount, m_EntityID, &material_count);

				if (material_count > 0)
				{
					std::unique_ptr<uint32_t[]> material_array(new uint32_t[material_count]);
					ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t*, size_t), MeshRendererComponent_GetAllMaterialsCopy, m_EntityID, material_array.get(), material_count);

					for (size_t i = 0; i < material_count; ++i)
					{
						Assets::Material material = Assets::Material(false);
						material.m_AssetHandle = material_array[i];
						material_vector.emplace_back(material);
					}
				}

				return material_vector;
			}

			/**
			* @brief Sets all materials for this mesh renderer.
			*
			* @param material_vector The new list of materials to assign.
			*
			* @code
			* mesh_renderer.SetMaterials({ material1, material2 });
			* @endcode
			*/
			void SetMaterials(const std::vector<Assets::Material>& material_vector) const
			{
				// Can't reinterpret_cast to const uint32_t* as there are vtable*'s that may lead to incorrect data alignment
				std::vector<uint32_t> material_array;
				material_array.reserve(material_vector.size());
				for (const auto& mat : material_vector)
					material_array.push_back((uint32_t)mat);

				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const uint32_t*, size_t), MeshRendererComponent_SetAllMaterials, m_EntityID, material_array.data(), material_vector.size());
			}

			/**
			* @brief Gets a specific material by index.
			*
			* @param material_index Index of the material (defaults to -1 = back of the material vector on the component).
			* @return The material at the specified index.
			*
			* @code
			* auto last_mat_in_renderer = mesh_renderer.GetMaterial();
			* auto first_mat_in_renderer = mesh_renderer.GetMaterial(0);
			* @endcode
			*/
			Assets::Material GetMaterial(size_t material_index = size_t(-1)) const { Assets::Material mat = Assets::Material(false); mat.m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t, size_t), MeshRendererComponent_GetMaterial, m_EntityID, material_index); return mat; }

			/**
			* @brief Sets a specific material at the given index.
			*
			* @param material The material to assign.
			* @param material_index Index of the slot to assign to (defaults to -1 = back of the material vector on the component).
			*
			* @code
			* mesh_renderer.SetMaterial(new_material);
			* @endcode
			*/
			void SetMaterial(const Assets::Material& material, size_t material_index = size_t(-1)) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t, uint32_t), MeshRendererComponent_SetMaterial, m_EntityID, material_index, material.m_AssetHandle); }

			/**
			* @brief Enables the material uniform block at a given index.
			* 
			* The uniform block is used to pass data to the shader for rendering.
			* 
			* For instance, you have a material asset which many objects use, but you want to set a particular uniform for just this one object without affecting the rest of the objects.
			*
			* @param material_index Index of the material whose uniform block to enable.
			*
			* @code
			* mesh_renderer.EnableUniformBlock(0);
			* @endcode
			*/
			void EnableUniformBlock(size_t material_index) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t), MeshRendererComponent_EnableUniformBlock, m_EntityID, material_index); }

			/**
			* @brief Disables the material uniform block at a given index.
			* 
			* The uniform block is used to pass data to the shader for rendering.
			* 
			* For instance, you have a material asset which many objects use, but you want to set a particular uniform for just this one object without affecting the rest of the objects.
			*
			* @param material_index Index of the material whose uniform block to disable.
			*
			* @code
			* mesh_renderer.DisableUniformBlock(0);
			* @endcode
			*/
			void DisableUniformBlock(size_t material_index) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t), MeshRendererComponent_DisableUniformBlock, m_EntityID, material_index); }


			/**
			* @brief Enables all material uniform blocks.
			* 
			* The uniform block is used to pass data to the shader for rendering.
			* 
			* For instance, you have a material asset which many objects use, but you want to set a particular uniform for just this one object without affecting the rest of the objects.
			*
			* @code
			* mesh_renderer.EnableAllUniformBlocks();
			* @endcode
			*/
			void EnableAllUniformBlocks() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), MeshRendererComponent_EnableAllUniformBlocks, m_EntityID); }

			/**
			* @brief Disables all material uniform blocks.
			* 
			* The uniform block is used to pass data to the shader for rendering.
			* 
			* For instance, you have a material asset which many objects use, but you want to set a particular uniform for just this one object without affecting the rest of the objects.
			*
			* @code
			* mesh_renderer.DisableAllUniformBlocks();
			* @endcode
			*/
			void DisableAllUniformBlocks() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), MeshRendererComponent_DisableAllUniformBlocks, m_EntityID); }


			/**
			* @brief Retrieves the uniform block for the given material index.
			* 
			* The uniform block is used to pass data to the shader for rendering.
			* 
			* For instance, you have a material asset which many objects use, but you want to set a particular uniform for just this one object without affecting the rest of the objects.
			* 
			* @param material_index The index of the material (defaults to -1, meaning the last material in the mesh renderer's materials).
			* @return The uniform block associated with the material.
			*
			* @code
			* auto block = mesh_renderer.GetUniformBlock();
			* block.SetFloat("alpha_fade_factor", 0.5f); // Ghostly!
			* @endcode
			*/
			Assets::MaterialUniformBlock GetUniformBlock(uint32_t material_index = -1) const { return Assets::MaterialUniformBlock(ENGINE_SAFE_CALL_RET_PTR(void*, void*(*)(uint32_t, uint32_t), MeshRendererComponent_GetUniformBlock, m_EntityID, material_index)); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::MeshRendererComponent; }

		};

		/**
		* @brief Component to access LOD mesh information.
		*
		* This allows the user to access data associated with a LOD mesh component.
		*/
		class LODMeshComponent : public Component
		{

		public:

			/**
			* @brief A single Level of Detail (LOD) element.
			*
			* Each element specifies a distance threshold and a set of mesh renderer entities to activate when that threshold is met.
			*/
			struct LODElement
			{
				/**
				* @brief Distance threshold from the camera, normalized between 0 and 1.
				*
				* 0 = near plane, 1 = far plane. If set to 0.5, this element is active when the object is halfway between near and far.
				*/
				float DistanceThresholdNormalised;

				/**
				* @brief Vector of entities with MeshRenderer components for this LOD level.
				*/
				std::vector<Entity> MeshRendererEntities;
			};


			/**
			* @brief Determines if the maximum LOD distance should be normalised to the camera's near-far plane, or a custom distance.
			*
			* @return True if normalised to custom distance, false if normalised within the near-far plane.
			*
			* @code
			* if (lod_component.IsMaxDistanceOverFarPlane())
			*     Debug::Log("Max distance exceeds far plane");
			* @endcode
			*/
			bool IsMaxDistanceOverFarPlane() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), LODMeshComponent_GetUseMaxDistanceOverFarPlane, m_EntityID); }

			/**
			* @brief Sets whether the maximum LOD distance should be normalised to the camera's near-far plane, or a custom distance.
			*
			* @param use_max_distance True if normalised to custom distance, false if normalised within the near-far plane.
			*
			* @code
			* lod_component.SetMaxDistanceOverFarPlane(true);
			* @endcode
			*/
			void SetMaxDistanceOverFarPlane(bool use_max_distance) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), LODMeshComponent_SetMaxDistanceOverFarPlane, m_EntityID, use_max_distance); }

			/**
			* @brief Gets the maximum distance at which LODs are normalised.
			*
			* @return Maximum distance in world units.
			*
			* @code
			* float max_dist = lod_component.GetMaxDistance();
			* @endcode
			*/
			float GetMaxDistance() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), LODMeshComponent_GetMaxDistance, m_EntityID); }

			/**
			* @brief Sets the maximum distance at which LODs are normalised.
			*
			* @param distance Maximum distance in world units.
			*
			* @code
			* lod_component.SetMaxDistance(150.0f);
			* @endcode
			*/
			void SetMaxDistance(float distance) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), LODMeshComponent_SetMaxDistance, m_EntityID, distance); }

			/**
			* @brief Gets all LOD elements associated with this component.
			*
			* @return Vector of LOD elements.
			*
			* @code
			* auto lods = lod_component.GetLODElements();
			* @endcode
			*/
			std::vector<LODElement> GetLODElements() const;

			/**
			* @brief Sets the full list of LOD elements for this component.
			*
			* @param lod_elements Vector of LOD element structs.
			*
			* @code
			* lod_component.SetLODElements({ lod_0, lod_1, lod_2 });
			* @endcode
			*/
			void SetLODElements(const std::vector<LODElement>& lod_elements) const;

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::LODMeshComponent; }

		};

		/**
		* @brief Represents a component for rendering and animating skinned meshes.
		*
		* This component holds a skinned mesh, skeleton, and materials, and supports GPU skinning with bone mapping.
		*/
		class SkinnedMeshComponent : public Component
		{

		public:

			/**
			* @brief Checks whether the skinned mesh renderer is active.
			*
			* @return True if active, false if disabled.
			*
			* @code
			* if (skinned_mesh.IsActive())
			*     Debug::Log("Skinned mesh is active.");
			* @endcode
			*/
			bool IsActive() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), SkinnedMeshComponent_GetIsActive, m_EntityID); }

			/**
			* @brief Enables or disables the skinned mesh renderer.
			*
			* @param active True to activate the renderer.
			*
			* @code
			* skinned_mesh.SetActive(true);
			* @endcode
			*/
			void SetActive(bool active) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), SkinnedMeshComponent_SetIsActive, m_EntityID, active); }

			/**
			* @brief Checks whether the skinned mesh casts shadows.
			*
			* @return True if it casts shadows.
			*/
			bool IsCastingShadows() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), SkinnedMeshComponent_GetIsCastingShadows, m_EntityID); }
			/**
			* @brief Enables or disables shadow casting.
			*
			* @param casting_shadows True to enable shadow casting.
			*/
			void SetCastingShadows(bool casting_shadows) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), SkinnedMeshComponent_SetCastingShadows, m_EntityID, casting_shadows); }

			/**
			* @brief Retrieves the skinned mesh asset.
			*
			* @return The static mesh asset used for skinning.
			*
			* @code
			* auto mesh = skinned_mesh.GetMeshAsset();
			* @endcode
			*/
			Assets::StaticMesh GetMeshAsset() const { Assets::StaticMesh asset = Assets::StaticMesh(false); asset.SetAssetHandle(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), SkinnedMeshComponent_GetMeshAssetHandle, m_EntityID)); return asset; }


			/**
			* @brief Sets the skinned mesh asset.
			*
			* @param static_mesh The mesh asset to assign.
			*/
			void SetMeshAsset(Assets::StaticMesh static_mesh) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t), SkinnedMeshComponent_SetMeshAssetHandle, m_EntityID, static_mesh.operator uint32_t()); }

			/**
			 * @brief Retrieves the skeleton asset assigned to this component.
			 *
			 * @return The skeleton asset.
			 */
			Assets::Skeleton GetSkeleton() const { Assets::Skeleton asset = {}; asset.SetAssetHandle(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), SkinnedMeshComponent_GetSkeletonAssetHandle, m_EntityID)); return asset; }

			/**
			* @brief Assigns a skeleton asset for this component.
			*
			* @param skeleton The skeleton asset to assign.
			*/
			void SetSkeletonAsset(Assets::Skeleton skeleton) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t), SkinnedMeshComponent_SetSkeletonAssetHandle, m_EntityID, skeleton.operator uint32_t()); }

			/**
			* @brief Gets the axis-aligned bounding box for the mesh.
			*
			* @return The bounding box of the mesh.
			*/
			Partitions::Bounds_AABB GetMeshBounds() const { return ENGINE_SAFE_CALL_RET(Partitions::Bounds_AABB, Partitions::Bounds_AABB(*)(uint32_t), SkinnedMeshComponent_GetMeshBounds, m_EntityID); }

			/**
			* @brief Sets the bounding box of the mesh.
			*
			* @param bounds The bounding box to assign.
			*/
			void SetMeshBounds(const Partitions::Bounds_AABB& bounds) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Partitions::Bounds_AABB&), SkinnedMeshComponent_SetMeshBounds, m_EntityID, bounds); }

			/**
			* @brief Gets the list of materials assigned to the mesh.
			*
			* @return A vector of material assets.
			*
			* @code
			* auto materials = skinned_mesh.GetMaterials();
			* @endcode
			*/
			std::vector<Assets::Material> GetMaterials() const
			{
				std::vector<Assets::Material> material_vector{};

				size_t material_count = 0;
				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t*), SkinnedMeshComponent_GetAllMaterialsCount, m_EntityID, &material_count);

				if (material_count > 0)
				{
					std::unique_ptr<uint32_t[]> material_array(new uint32_t[material_count]);
					ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t*, size_t), SkinnedMeshComponent_GetAllMaterialsCopy, m_EntityID, material_array.get(), material_count);

					for (size_t i = 0; i < material_count; ++i)
					{
						Assets::Material material = Assets::Material(false);
						material.m_AssetHandle = material_array[i];
						material_vector.emplace_back(material);
					}
				}

				return material_vector;
			}

			/**
			* @brief Sets the list of materials used for the skinned mesh.
			*
			* @param material_vector The materials to assign.
			*/
			void SetMaterials(const std::vector<Assets::Material>& material_vector) const
			{
				// Can't reinterpret_cast to const uint32_t* as there are vtable*'s that may lead to incorrect data alignment
				std::vector<uint32_t> handles;
				handles.reserve(material_vector.size());
				for (const auto& mat : material_vector)
					handles.push_back((uint32_t)mat);

				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const uint32_t*, size_t), SkinnedMeshComponent_SetAllMaterials, m_EntityID, handles.data(), material_vector.size());
			}

			/**
			* @brief Gets a specific material at the given index.
			*
			* @param material_index The material index (defaults to -1 = last material in material's vector).
			* @return The material at the given index.
			*/
			Assets::Material GetMaterial(size_t material_index = size_t(-1)) const { Assets::Material mat = Assets::Material(false); mat.m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t, size_t), SkinnedMeshComponent_GetMaterial, m_EntityID, material_index); return mat; }

			/**
			* @brief Sets a material at the given index.
			*
			* @param material The material to assign.
			* @param material_index The index at which to assign it.
			*/
			void SetMaterial(const Assets::Material& material, size_t material_index = size_t(-1)) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t, uint32_t), SkinnedMeshComponent_SetMaterial, m_EntityID, material_index, material.m_AssetHandle); }

			/**
			* @brief Gets the bone index to entity mapping for GPU skinning.
			*
			* @return A map of bone index to entity (used to get transforms).
			*
			* @code
			* auto bone_map = skinned_mesh.GetBoneMapping();
			* @endcode
			*/
			std::unordered_map<uint32_t, Entity> GetBoneMapping() const;

			/**
			* @brief Sets the bone mapping used for GPU skinning.
			*
			* @param bone_map Map of bone indices to entity transforms.
			*
			* @code
			* skinned_mesh.SetBoneMapping(bone_map);
			* @endcode
			*/
			void SetBoneMapping(const std::unordered_map<uint32_t, Entity>& bone_map) const;

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::SkinnedMeshComponent; }

		};

		class AnimatorComponent : public Component
		{

		public:

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::AnimatorComponent; }

			void SetBool(const Animation::StringHash& param_hash, bool value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, bool), AnimatorComponent_SetBool, m_EntityID, param_hash, value); }
			void SetBool(const std::string& param_name, bool value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, bool), AnimatorComponent_SetBool, m_EntityID, Utils::fnv1a_hash(param_name), value); }
			
			void SetFloat(const Animation::StringHash& param_hash, float value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, float), AnimatorComponent_SetFloat, m_EntityID, param_hash, value); }
			void SetFloat(const std::string& param_name, float value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, float), AnimatorComponent_SetFloat, m_EntityID, Utils::fnv1a_hash(param_name), value); }
			
			void SetUInt(const Animation::StringHash& param_hash, uint32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, uint32_t), AnimatorComponent_SetUInt, m_EntityID, param_hash, value); }
			void SetUInt(const std::string& param_name, uint32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, uint32_t), AnimatorComponent_SetUInt, m_EntityID, Utils::fnv1a_hash(param_name), value); }
			
			void SetInt(const Animation::StringHash& param_hash, int32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, int32_t), AnimatorComponent_SetBool, m_EntityID, param_hash, value); }
			void SetInt(const std::string& param_name, int32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, int32_t), AnimatorComponent_SetBool, m_EntityID, Utils::fnv1a_hash(param_name), value); }

			void ResetMachine() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), AnimatorComponent_ResetMachine, m_EntityID); }

		};

		/**
		* @brief Component to access animator information.
		*
		* This allows the user to access data associated with an animator component.
		*/
		class BasicAnimationComponent : public Component
		{
			// Leaving this for now as I need to rethink the animation system

		public:

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::BasicAnimationComponent; }

			/**
			 * @brief Play an animation.
			 * 
			 * @param clip_index The index of the clip to play on the animator component.
			 * @param should_loop Should the animation loop indefinetly
			 * 
			 * @code
			 * animator_component.PlayAnimation(0, false); // This will play animation at index 0 on the animator component animations.
			 * @endcode
			 */
			void PlayAnimation(int clip_index, bool should_loop = true) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, int32_t, bool), BasicAnimationComponent_PlayAnimation_Index, m_EntityID, clip_index, should_loop); }
			
			/**
			 * @brief Play an animation.
			 * 
			 * @param clip_name The name of the clip to play on the animator component. If this animation is not present on the component , it will do nothing.
			 * @param should_loop Should the animation loop indefinetly
			 * 
			 * @code
			 * animator_component.PlayAnimation("Taunt_Dance_01", false); // This will play the "Taunt_Dance_01" animation on animator component if exists.
			 * @endcode
			 */
			void PlayAnimation(const char* clip_name, bool should_loop = true) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, bool), BasicAnimationComponent_PlayAnimation_Name, m_EntityID, clip_name, should_loop); }
			
			/**
			 * @brief Pause any currently running animations.
			 * 
			 * @code
			 * if (character_frozen)
			 * {
			 *     animator_component.PauseAnimation();
			 * }
			 * @endcode
			 */
			void PauseAnimation() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), BasicAnimationComponent_PauseAnimation, m_EntityID); }
			
			/**
			 * @brief Resume animator component.
			 * 
			 * @code
			 * if (character_frozen && character_to_be_unfrozen)
			 * {
			 *     animator_component.ResumeAnimation();
			 *     character_frozen = false;
			 *     character_to_be_unfrozen = false;
			 * }
			 * @endcode
			 */
			void ResumeAnimation() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), BasicAnimationComponent_ResumeAnimation, m_EntityID); }
			
			/**
			 * @brief Stop any currently running animation, and reset the current timestep to reset the animation.
			 */
			void StopAnimation() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), BasicAnimationComponent_StopAnimation, m_EntityID); }

			/**
			 * @brief Checks if the animator is currently playing an animation.
			 * 
			 * @return True if playing, false if not. 
			 */
			bool IsPlaying() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), BasicAnimationComponent_IsPlaying, m_EntityID); }
			
			/**
			 * @brief Checks if the animator is currently looping an animation.
			 * 
			 * @return True if looping, false if not. 
			 */
			bool IsLooping() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), BasicAnimationComponent_IsLooping, m_EntityID); }
			
			/**
			 * @brief Checks if the animator is currently looping an animation.
			 * 
			 * @param should_loop Should the animation loop indefinetly
			 */
			void SetIsLooping(bool should_loop) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), BasicAnimationComponent_SetIsLooping, m_EntityID, should_loop); }

			/**
			 * @brief Gets the playback speed of the Animator Component.
			 * 
			 * @return The playback speed.
			 */
			float GetPlaybackSpeed() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), BasicAnimationComponent_GetPlaybackSpeed, m_EntityID); }
			
			/**
			 * @brief Sets the playback speed of the Animator Component.
			 * 
			 * @param playback_speed The speed at which the animation should play. E.g., 1.0f being normal speed, 2.0f being double speed, and 0.5f being half speed.
			 * 
			 * @code
			 * if (super_fast_speed)
			 * {
			 *     animator_component.SetPlaybackSpeed(2.0f); // Runs animation at x2 speed.
			 * }
			 * @endcode
			 */
			void SetPlaybackSpeed(float playback_speed) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), BasicAnimationComponent_SetPlaybackSpeed, m_EntityID, playback_speed); }

			/**
			 * @brief Gets the current normalised time step of the animation clip.
			 * 
			 * This is normalised between 0.0f -> 1.0f with 0.0f being the start of the animation and 1.0f being the end of the animation.
			 * 
			 * @return The normalised timestep of the animation.
			 */
			float GetCurrentTimestep() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), BasicAnimationComponent_GetCurrentTimestep, m_EntityID); }
			
			/**
			 * @brief Sets the current normalised time step of the animation clip.
			 * 
			 * This is normalised between 0.0f -> 1.0f with 0.0f being the start of the animation and 1.0f being the end of the animation.
			 * 
			 * @param normalised_time_step The timestep you wish to set the animation to
			 */
			void SetCurrentTimestep(float normalised_time_step) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), BasicAnimationComponent_SetCurrentTimestep, m_EntityID, normalised_time_step); }

			/**
			 * @brief Gets the index of the current playing animation.
			 * 
			 * @return The index of the current playing animation or -1 if no animation playing.
			 */
			uint32_t GetCurrentClipIndex() const { return ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), BasicAnimationComponent_GetCurrentClipIndex, m_EntityID); }
			
			/**
			 * @brief Gets the name of the current playing animation.
			 * 
			 * @return The name of the current playing animation or "" if no animation playing.
			 */
			std::string GetCurrentClipName() const {
				return std::string(ENGINE_SAFE_CALL_RET_PTR(const char*, const char*(*)(uint32_t), BasicAnimationComponent_GetCurrentClipName, m_EntityID)); 
			}
		};

		/**
		* @brief Represents a skybox component that renders an environment background.
		*
		* This component should be attached to an entity responsible for skybox rendering. It uses a cube map material.
		*/
		class SkyboxComponent : public Component
		{

		public:

			/**
			* @brief Gets the skybox material asset assigned to this component.
			*
			* @return A reference to the assigned skybox material.
			*
			* @code
			* auto skybox_material = skybox_component.GetSkyboxMaterialAsset();
			* @endcode
			*/
			Assets::SkyboxMaterial GetSkyboxMaterialAsset() const { Assets::SkyboxMaterial asset = {}; asset.SetAssetHandle(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), SkyboxComponent_GetSkyboxMaterialAssetHandle, m_EntityID)); return asset; }

			/**
			* @brief Sets the skybox material asset for this component.
			*
			* @param skybox_material The material to assign.
			*
			* @code
			* skybox_component.SetSkyboxMaterialAsset(my_skybox_material);
			* @endcode
			*/
			void SetSkyboxMaterialAsset(Assets::SkyboxMaterial skybox_material) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t), SkyboxComponent_SetSkyboxMaterialAssetHandle, m_EntityID, skybox_material.operator uint32_t()); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::SkyboxComponent; }

		};

		enum class ShadowTypeFlag : uint8_t
		{
			NoShadows = 0,
			HardShadows = 1,
			SoftShadows = 2,
		};

		/**
		* @brief Component to access point light information.
		*
		* Allows configuration of radius, intensity, colour, and shadowing behavior for a point light.
		*/
		class PointLightComponent : public Component
		{

		public:

			/**
			* @brief Checks if the point light is currently active.
			* @return True if the light is on.
			*/
			bool IsActive() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), PointLightComponent_GetIsActive, m_EntityID); }

			/**
			* @brief Enables or disables the point light.
			* @param active True to turn on the light.
			*/
			void SetActive(bool active) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), PointLightComponent_SetIsActive, m_EntityID, active); }

			/**
			* @brief Gets the effective radius of the point light.
			* @return Radius in world units.
			*/
			float GetRadius() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), PointLightComponent_GetRadius, m_EntityID); }

			/**
			* @brief Sets the radius of the point light.
			* @param radius Radius in world units.
			*/
			void SetRadius(float radius) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), PointLightComponent_SetRadius, m_EntityID, radius); }

			/**
			* @brief Gets the light intensity.
			* @return Intensity multiplier.
			*/
			float GetIntensity() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), PointLightComponent_GetIntensity, m_EntityID); }

			/**
			* @brief Sets the light intensity.
			* @param intensity Intensity multiplier.
			*/
			void SetIntensity(float intensity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), PointLightComponent_SetIntensity, m_EntityID, intensity); }

			/**
			* @brief Gets the light colour.
			* @return RGBA colour vector.
			*/
			Vectors::Vector4 GetColour() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector4, Vectors::Vector4(*)(uint32_t), PointLightComponent_GetColour, m_EntityID); }

			/**
			* @brief Sets the light colour.
			* @param colour RGBA colour.
			*/
			void SetColour(Vectors::Vector4 colour) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*), PointLightComponent_SetColour, m_EntityID, &colour); }
			
			/**
			* @brief Gets the shadow flag for this light.
			* @return Shadow casting mode.
			*/
			ShadowTypeFlag GetShadowFlag() const { return ENGINE_SAFE_CALL_RET(ShadowTypeFlag, ShadowTypeFlag(*)(uint32_t), PointLightComponent_GetShadowFlag, m_EntityID); }

			/**
			* @brief Sets the shadow flag.
			* @param flag Shadow casting mode.
			*/
			void SetShadowFlag(ShadowTypeFlag flag) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), PointLightComponent_SetShadowFlag, m_EntityID, static_cast<uint8_t>(flag)); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::PointLightComponent; }

		};

		/**
		* @brief Component to access point light information.
		*
		* Allows configuration of range, angle, intensity, colour, and shadowing behavior for a spot light.
		*/
		class SpotLightComponent : public Component
		{

		public:

			/**
			* @brief Checks if the spot light is currently active.
			* @return True if the light is on.
			*/
			bool IsActive() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), SpotLightComponent_GetIsActive, m_EntityID); }

			/**
			* @brief Enables or disables the spot light.
			* @param active True to turn on the light.
			*/
			void SetActive(bool active) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), SpotLightComponent_SetIsActive, m_EntityID, active); }

			/**
			* @brief Gets the effective range of the spot light.
			* @return Range in world units.
			*/
			float GetRange() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), SpotLightComponent_GetRange, m_EntityID); }

			/**
			* @brief Sets the range of the spot light.
			* @param range Range in world units.
			*/
			void SetRange(float range) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), SpotLightComponent_SetRange, m_EntityID, range); }

			/**
			* @brief Gets the cone angle in degrees.
			* @return Cone angle in degrees.
			*/
			float GetAngle() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), SpotLightComponent_GetAngle, m_EntityID); }

			/**
			* @brief Sets the cone angle in degrees.
			* @param range Cone angle in degrees.
			*/
			void SetAngle(float angle) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), SpotLightComponent_SetAngle, m_EntityID, angle); }

			/**
			* @brief Gets the light intensity.
			* @return Intensity multiplier.
			*/
			float GetIntensity() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), SpotLightComponent_GetIntensity, m_EntityID); }

			/**
			* @brief Sets the light intensity.
			* @param intensity Intensity multiplier.
			*/
			void SetIntensity(float intensity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), SpotLightComponent_SetIntensity, m_EntityID, intensity); }

			/**
			* @brief Gets the light colour.
			* @return RGBA colour vector.
			*/
			Vectors::Vector4 GetColour() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector4, Vectors::Vector4(*)(uint32_t), SpotLightComponent_GetColour, m_EntityID); }

			/**
			* @brief Sets the light colour.
			* @param colour RGBA colour.
			*/
			void SetColour(Vectors::Vector4 colour) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*), SpotLightComponent_SetColour, m_EntityID, &colour); }

			/**
			* @brief Gets the shadow flag for this light.
			* @return Shadow casting mode.
			*/
			ShadowTypeFlag GetShadowFlag() const { return ENGINE_SAFE_CALL_RET(ShadowTypeFlag, ShadowTypeFlag(*)(uint32_t), SpotLightComponent_GetShadowFlag, m_EntityID); }

			/**
			* @brief Sets the shadow flag.
			* @param flag Shadow casting mode.
			*/
			void SetShadowFlag(ShadowTypeFlag flag) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), SpotLightComponent_SetShadowFlag, m_EntityID, static_cast<uint8_t>(flag)); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::SpotLightComponent; }

		};

		/**
		* @brief Component to access point light information.
		*
		* Allows configuration of intensity, colour, and shadowing behavior for a directional light.
		*/
		class DirectionalLightComponent : public Component
		{

		public:

			/**
			* @brief Checks if the directional light is currently active.
			* @return True if the light is on.
			*/
			bool IsActive() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), DirectionalLightComponent_GetIsActive, m_EntityID); }

			/**
			* @brief Enables or disables the directional light.
			* @param active True to turn on the light.
			*/
			void SetActive(bool active) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), DirectionalLightComponent_SetIsActive, m_EntityID, active); }

			/**
			* @brief Gets the light intensity.
			* @return Intensity multiplier.
			*/
			float GetIntensity() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), DirectionalLightComponent_GetIntensity, m_EntityID); }

			/**
			* @brief Sets the light intensity.
			* @param intensity Intensity multiplier.
			*/
			void SetIntensity(float intensity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), DirectionalLightComponent_SetIntensity, m_EntityID, intensity); }

			/**
			* @brief Gets the maximum distance shadows will be rendered to from this light source in cascaded shadow maps.
			* @return Maximum distance shadows will be rendered to from this light source in cascaded shadow maps.
			*/
			float GetMaxShadowDistance() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), DirectionalLightComponent_GetMaxShadowDistance, m_EntityID); }

			/**
			* @brief Sets the maximum distance shadows will be rendered to from this light source in cascaded shadow maps.
			* @param float Maximum distance shadows will be rendered to from this light source in cascaded shadow maps.
			*/
			void SetMaxShadowDistance(float max_shadow_distance) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), DirectionalLightComponent_SetMaxShadowDistance, m_EntityID, max_shadow_distance); }

			/**
			* @brief Gets the light colour.
			* @return RGBA colour vector.
			*/
			Vectors::Vector4 GetColour() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector4, Vectors::Vector4(*)(uint32_t), DirectionalLightComponent_GetColour, m_EntityID); }

			/**
			* @brief Sets the light colour.
			* @param colour RGBA colour.
			*/
			void SetColour(Vectors::Vector4 colour) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*), DirectionalLightComponent_SetColour, m_EntityID, &colour); }

			/**
			* @brief Gets the shadow flag for this light.
			* @return Shadow casting mode.
			*/
			ShadowTypeFlag GetShadowFlag() const { return ENGINE_SAFE_CALL_RET(ShadowTypeFlag, ShadowTypeFlag(*)(uint32_t), DirectionalLightComponent_GetShadowFlag, m_EntityID); }

			/**
			* @brief Sets the shadow flag.
			* @param flag Shadow casting mode.
			*/
			void SetShadowFlag(ShadowTypeFlag flag) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), DirectionalLightComponent_SetShadowFlag, m_EntityID, static_cast<uint8_t>(flag)); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::DirectionalLightComponent; }

		};

		struct PhysicsMaterial
		{
			float dynamic_friction;
			float static_friction;
			float bounciness;
		};

		/**
		* @brief Component that adds physical behavior to an entity.
		*
		* This component handles physics simulation properties such as mass, gravity, constraints, and force application.
		*/
		class RigidbodyComponent : public Component
		{

		public:

			/**
			* @brief Gets the mass of the rigidbody.
			* @return Mass in kilograms.
			*/
			float GetMass() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), RigidbodyComponent_GetMass, m_EntityID); }

			/**
			* @brief Gets the linear drag of the rigidbody.
			* @return Drag factor.
			*/
			float GetDrag() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), RigidbodyComponent_GetDrag, m_EntityID); }

			/**
			* @brief Gets the angular drag of the rigidbody.
			* @return Angular drag factor.
			*/
			float GetAngularDrag() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), RigidbodyComponent_GetAngularDrag, m_EntityID); }

			/**
			* @brief Gets the linear velocity of the rigidbody.
			* @return Linear velocity of the rigidbody.
			*/
			Vectors::Vector3 GetLinearVelocity() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), RigidbodyComponent_GetLinearVelocity, m_EntityID); }

			/**
			* @brief Gets the angular velocity of the rigidbody.
			* @return Angular velocity of the rigidbody.
			*/
			Vectors::Vector3 GetAngularVelocity() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), RigidbodyComponent_GetAngularVelocity, m_EntityID); }

			/**
			* @brief Checks whether the automatic center of mass is enabled.
			* @return True if automatic center of mass is used.
			*/
			bool IsAutomaticCentreOfMassEnabled() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), RigidbodyComponent_GetAutomaticCentreOfMass, m_EntityID); }

			/**
			* @brief Checks whether gravity is enabled on this rigidbody.
			* @return True if gravity affects the rigidbody.
			*/
			bool IsGravityEnabled() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), RigidbodyComponent_GetUseGravity, m_EntityID); }

			/**
			* @brief Checks whether the rigidbody is kinematic.
			* @return True if the rigidbody is kinematic.
			*/
			bool IsKinematicEnabled() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), RigidbodyComponent_GetIsKinematic, m_EntityID); }

			/**
			* @brief Gets the position constraint bitmask.
			* @return Vector indicating locked axes (true = locked).
			*/
			Vectors::BVector3 GetPositionConstraint() const { return ENGINE_SAFE_CALL_RET(Vectors::BVector3, Vectors::BVector3(*)(uint32_t), RigidbodyComponent_GetPositionConstraint, m_EntityID); }

			/**
			* @brief Gets the rotation constraint bitmask.
			* @return Vector indicating locked rotation axes (true = locked).
			*/
			Vectors::BVector3 GetRotationConstraint() const { return ENGINE_SAFE_CALL_RET(Vectors::BVector3, Vectors::BVector3(*)(uint32_t), RigidbodyComponent_GetRotationConstraint, m_EntityID); }

			/**
			* @brief Sets the mass of the rigidbody.
			* @param mass The new mass in kilograms.
			*/
			void SetMass(float mass) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), RigidbodyComponent_SetMass, m_EntityID, mass); }

			/**
			* @brief Sets the linear drag.
			* @param drag The new drag value.
			*/
			void SetDrag(float drag) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), RigidbodyComponent_SetDrag, m_EntityID, drag); }

			/**
			* @brief Sets the linear velocity of the rigidbody.
			* @param linear_velocity The new linear velocity of the rigidbody.
			*/
			void SetLinearVelocity(const Vectors::Vector3& linear_velocity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), RigidbodyComponent_SetLinearVelocity, m_EntityID, &linear_velocity); }

			/**
			* @brief Sets the angular velocity of the rigidbody.
			* @param angular_velocity The new angular velocity of the rigidbody.
			*/
			void SetAngularVelocity(const Vectors::Vector3& angular_velocity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), RigidbodyComponent_SetAngularVelocity, m_EntityID, &angular_velocity); }

			/**
			* @brief Sets the angular drag.
			* @param angular_drag The new angular drag value.
			*/
			void SetAngularDrag(float angularDrag) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), RigidbodyComponent_SetAngularDrag, m_EntityID, angularDrag); }

			/**
			* @brief Enables or disables automatic center of mass calculation.
			* @param automatic True to enable automatic center of mass.
			*/
			void SetAutomaticCentreOfMass(const bool& automaticCentreOfMass) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), RigidbodyComponent_SetAutomaticCentreOfMass, m_EntityID, automaticCentreOfMass); }

			/**
			* @brief Enables or disables gravity for this rigidbody.
			* @param use_gravity True to enable gravity.
			*/
			void SetGravity(bool useGravity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), RigidbodyComponent_SetUseGravity, m_EntityID, useGravity); }

			/**
			* @brief Enables or disables kinematic mode.
			* @param is_kinematic True to make the rigidbody kinematic.
			*/
			void SetKinematic(bool isKinematic) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), RigidbodyComponent_SetIsKinematic, m_EntityID, isKinematic); }

			/**
			* @brief Locks movement along specific axes.
			* @param position_constraint Boolean vector (x, y, z) where true = locked.
			*/
			void SetPositionConstraint(const Vectors::BVector3& positionConstraint) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::BVector3*), RigidbodyComponent_SetPositionConstraint, m_EntityID, &positionConstraint); }

			/**
			* @brief Locks rotation along specific axes.
			* @param rotation_constraint Boolean vector (x, y, z) where true = locked.
			*/
			void SetRotationConstraint(const Vectors::BVector3& rotationConstraint) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::BVector3*), RigidbodyComponent_SetRotationConstraint, m_EntityID, &rotationConstraint); }

			/**
			* @brief Defines how forces are applied to the rigidbody.
			*/
			enum class ForceMode : uint8_t
			{
				Force,             ///< Continuous force using mass
				Impulse,           ///< Instantaneous force using mass
				Velocity_Change,   ///< Instantaneous force ignoring mass
				Acceleration       ///< Continuous force ignoring mass
			};

			/**
			* @brief Applies a force to the rigidbody.
			* @param force The force vector in world space.
			* @param force_mode The mode to apply the force in (default = Force).
			*/
			void ApplyForce(const Vectors::Vector3& force, ForceMode forceMode = ForceMode::Force) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*, uint8_t), RigidbodyComponent_ApplyForce, m_EntityID, &force, static_cast<uint8_t>(forceMode)); }

			/**
			* @brief Applies a torque to the rigidbody.
			* @param torque The torque vector in world space.
			*/
			void ApplyTorque(const Vectors::Vector3& torque) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), RigidbodyComponent_ApplyTorque, m_EntityID, &torque); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::RigidbodyComponent; }

		};

		/**
		* @brief Component to access box collider information.
		*
		* Used to define axis-aligned box colliders with size, center, material and trigger settings.
		*/
		class BoxColliderComponent : public Component
		{

		public:

			/**
			* @brief Checks if the box collider is a trigger.
			* @return True if it is a trigger.
			*/
			bool IsTrigger() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), BoxColliderComponent_GetIsTrigger, m_EntityID); }

			/**
			* @brief Gets the physics material used by the box collider.
			* @return The assigned physics material.
			*/
			PhysicsMaterial GetMaterial() const { return ENGINE_SAFE_CALL_RET(PhysicsMaterial, PhysicsMaterial(*)(uint32_t), BoxColliderComponent_GetMaterial, m_EntityID); }

			/**
			* @brief Gets the center offset of the box collider.
			* @return A 3D vector representing the center position.
			*/
			Vectors::Vector3 GetCentre() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), BoxColliderComponent_GetCentre, m_EntityID); }

			/**
			* @brief Gets the size of the box collider.
			* @return A 3D vector representing the box's size.
			*/
			Vectors::Vector3 GetSize() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), BoxColliderComponent_GetSize, m_EntityID); }

			/**
			* @brief Sets whether the collider is a trigger.
			* @param is_trigger True to make it a trigger.
			*/
			void SetIsTrigger(bool isTrigger) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), BoxColliderComponent_SetIsTrigger, m_EntityID, isTrigger); }

			/**
			* @brief Sets the physics material for this collider.
			* @param material The physics material to assign.
			*/
			void SetMaterial(const PhysicsMaterial& material) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const PhysicsMaterial*), BoxColliderComponent_SetMaterial, m_EntityID, &material); }

			/**
			* @brief Sets the center of the box collider.
			* @param centre New center position as a 3D vector.
			*/
			void SetCentre(const Vectors::Vector3& centre) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), BoxColliderComponent_SetCentre, m_EntityID, &centre); }

			/**
			* @brief Sets the size of the box collider.
			* @param size New size as a 3D vector.
			*/
			void SetSize(const Vectors::Vector3& size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), BoxColliderComponent_SetSize, m_EntityID, &size); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::BoxColliderComponent; }

		};

		/**
		* @brief Component to access sphere collider information.
		*
		* Used to define spherical colliders with center, radius, material and trigger settings.
		*/
		class SphereColliderComponent : public Component
		{

		public:

			/**
			* @brief Checks if the sphere collider is a trigger.
			* @return True if it is a trigger.
			*/
			bool IsTrigger() const { return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), SphereColliderComponent_GetIsTrigger, m_EntityID); }

			/**
			* @brief Gets the physics material used by the sphere collider.
			* @return The assigned physics material.
			*/
			PhysicsMaterial GetMaterial() const { return ENGINE_SAFE_CALL_RET(PhysicsMaterial, PhysicsMaterial(*)(uint32_t), SphereColliderComponent_GetMaterial, m_EntityID); }

			/**
			* @brief Gets the center offset of the sphere collider.
			* @return A 3D vector representing the center position.
			*/
			Vectors::Vector3 GetCentre() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), SphereColliderComponent_GetCentre, m_EntityID); }

			/**
			* @brief Gets the radius of the sphere collider.
			* @return Radius as a float.
			*/
			float GetRadius() const { return ENGINE_SAFE_CALL_RET(float, float(*)(uint32_t), SphereColliderComponent_GetRadius, m_EntityID); }

			/**
			* @brief Sets whether the collider is a trigger.
			* @param is_trigger True to make it a trigger.
			*/
			void SetIsTrigger(bool isTrigger) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), SphereColliderComponent_SetIsTrigger, m_EntityID, isTrigger); }

			/**
			* @brief Sets the physics material for this collider.
			* @param material The physics material to assign.
			*/
			void SetMaterial(const PhysicsMaterial& material) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const PhysicsMaterial*), SphereColliderComponent_SetMaterial, m_EntityID, &material); }

			/**
			* @brief Sets the center of the sphere collider.
			* @param centre New center position as a 3D vector.
			*/
			void SetCentre(const Vectors::Vector3& centre) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), SphereColliderComponent_SetCentre, m_EntityID, &centre); }

			/**
			* @brief Sets the radius of the sphere collider.
			* @param radius New radius as a float.
			*/
			void SetRadius(float radius) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float), SphereColliderComponent_SetRadius, m_EntityID, radius); }

			static BackEndAPI::FieldType GetType() { return BackEndAPI::FieldType::SphereColliderComponent; }

		};


	}
}