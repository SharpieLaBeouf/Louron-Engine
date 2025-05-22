#pragma once

#include "Script Defines.h"
#include "../Core/Logging.h"
#include "../Core/KeyCodes.h"
#include "../Core/UUID.h"

#include "../OpenGL/Material.h"
#include "../OpenGL/Compute Shader Asset.h"

#include "../Scene/Entity.h"

#include "../Animation/Animation Base Types.h"

#include "glm/glm.hpp"

namespace Louron
{

    class ScriptRegister
    {

    public:

        using RegisterHostFunctionFn = void(*)(const char* name, void* fn);

        static bool RegisterAll(void* script_assembly);

    private:

#pragma region Debug

        static void Debug_NativeLogMessage(const char* message, uint8_t type);

#pragma endregion

#pragma region Time

        static float Time_GetDeltaTime();
        static float Time_GetFixedDeltaTime();
        static float Time_GetUnscaledDeltaTime();
        static float Time_GetUnscaledFixedDeltaTime();
        static float Time_GetTimeScale();
        static void Time_SetTimeScale(float time_scale);
        static int Time_GetFrameRate();
        static float Time_GetCurrTime();

        static void Time_SetFixedDeltaTime(float fixed_time);

#pragma endregion

#pragma region Input

        static bool Input_GetKey(KeyCode key_code);
        static bool Input_GetKeyDown(KeyCode key_code);
        static bool Input_GetKeyUp(KeyCode key_code);

        static bool Input_GetMouseButton(MouseButtonCode button_code);
        static bool Input_GetMouseButtonDown(MouseButtonCode button_code);
        static bool Input_GetMouseButtonUp(MouseButtonCode button_code);
        static glm::vec2 Input_GetMousePosition();

#pragma endregion

#pragma region Entity

        struct _Transform
        {
            glm::vec3 position;
            glm::vec3 rotation;
            glm::vec3 scale;
        };

        static uint32_t Entity_Instantiate(uint32_t prefab_handle);
        static uint32_t Entity_Instantiate_Transform(uint32_t prefab_handle, const _Transform* transform);
		
        static uint32_t Entity_Create(const char* name);
        static void Entity_Destroy(UUID entity_uuid);

        static uint32_t Entity_FindByName(const char* name);
        static uint32_t Entity_FindByUUID(uint32_t entity_uuid);

        static UUID Entity_GetParent(UUID entity_uuid);
        static void Entity_SetParent(UUID entity_uuid, UUID parent_uuid);

        static bool Entity_CheckValidEntity(UUID entity_uuid);

        static void Entity_AddComponent(UUID entity_uuid, ScriptFieldType component_type);
        static void Entity_RemoveComponent(UUID entity_uuid, ScriptFieldType component_type);
        static bool Entity_HasComponent(UUID entity_uuid, ScriptFieldType component_type);

        static uint32_t Entity_GetComponentInParent(UUID entity_uuid, ScriptFieldType component_type);
        static uint32_t Entity_GetComponentInChildren(UUID entity_uuid, ScriptFieldType component_type);
        
        // We cannot allocate array on APP side. We need to get count, allocate array 
        // on DLL side, pass array back to APP, then APP fills in array.

        static inline std::unordered_map<uint64_t, std::vector<Entity>> s_CachedFindHierarchyComponents;

        static uint64_t Entity_GetComponentsInParentCount(UUID entity_uuid, ScriptFieldType component_type, size_t* count);
        static uint64_t Entity_GetComponentsInChildrenCount(UUID entity_uuid, ScriptFieldType component_type, size_t* count);

        static void Entity_GetComponentsInHierarchyCopy(UUID* entity_array, size_t count, uint64_t cached_key);

        template<typename T>
        static uint64_t CacheAndReturn(Entity& entity, uint64_t base_key, size_t* count, bool get_parents)
        {
            auto& result = get_parents
                ? s_CachedFindHierarchyComponents[base_key] = entity.GetComponentsInParents<T>()
                : s_CachedFindHierarchyComponents[base_key] = entity.GetComponentsInChildren<T>();

            *count = result.size();

            if (*count == 0)
            {
                s_CachedFindHierarchyComponents.erase(base_key);
                return -1;
            }

            return base_key;
        }

#pragma endregion

#pragma region Tag Component

        static const char* TagComponent_GetTag(UUID entity_uuid);
        static void TagComponent_SetTag(UUID entity_uuid, const char* value);

#pragma endregion

#pragma region TransformComponent

        static _Transform TransformComponent_GetTransform(UUID entity_uuid);
        static void TransformComponent_SetTransform(UUID entity_uuid, const _Transform* value);

        static _Transform TransformComponent_GetGlobalTransform(UUID entity_uuid);
        static void TransformComponent_SetGlobalTransform(UUID entity_uuid, const _Transform* value);

        static glm::vec3 TransformComponent_GetPosition(UUID entity_uuid);
        static void TransformComponent_SetPosition(UUID entity_uuid, const glm::vec3* value);
        
        static glm::vec3 TransformComponent_GetGlobalPosition(UUID entity_uuid);
        static void TransformComponent_SetGlobalPosition(UUID entity_uuid, const glm::vec3* value);

        static glm::vec3 TransformComponent_GetRotation(UUID entity_uuid);
        static void TransformComponent_SetRotation(UUID entity_uuid, const glm::vec3* value);
        
        static glm::vec3 TransformComponent_GetGlobalRotation(UUID entity_uuid);
        static void TransformComponent_SetGlobalRotation(UUID entity_uuid, const glm::vec3* value);

        static glm::vec3 TransformComponent_GetScale(UUID entity_uuid);
        static void TransformComponent_SetScale(UUID entity_uuid, const glm::vec3* value);

        static glm::vec3 TransformComponent_GetGlobalScale(UUID entity_uuid);
        static void TransformComponent_SetGlobalScale(UUID entity_uuid, const glm::vec3* value);
        
        static glm::vec3 TransformComponent_GetFront(UUID entity_uuid);
        static void TransformComponent_SetFront(UUID entity_uuid, const glm::vec3* value);

        static glm::vec3 TransformComponent_GetUp(UUID entity_uuid);
        static glm::vec3 TransformComponent_GetRight(UUID entity_uuid);

#pragma endregion

#pragma region Camera Component

        static uint8_t CameraComponent_GetClearFlag(UUID entity_uuid);
        static void CameraComponent_SetClearFlag(UUID entity_uuid, CameraClearFlags value);

        static glm::vec4 CameraComponent_GetClearColour(UUID entity_uuid);
        static void CameraComponent_SetClearColour(UUID entity_uuid, glm::vec4 value);

        static uint8_t CameraComponent_GetCameraDepth(UUID entity_uuid);
        static void CameraComponent_SetCameraDepth(UUID entity_uuid, uint8_t value);

        static bool CameraComponent_GetDisplayingToViewport(UUID entity_uuid);
        static void CameraComponent_SetDisplayingToViewport(UUID entity_uuid, bool value);

        static glm::vec4 CameraComponent_GetViewport(UUID entity_uuid);
        static void CameraComponent_SetViewport(UUID entity_uuid, const glm::vec4* value);

#pragma endregion

#pragma region MeshFilter

        // MeshFilterComponent Functions
        static uint32_t MeshFilterComponent_GetMeshAssetHandle(uint32_t entity_uuid);
        static void MeshFilterComponent_SetMeshAssetHandle(uint32_t entity_uuid, uint32_t asset_handle);

        static Bounds_AABB MeshFilterComponent_GetMeshBounds(uint32_t entity_uuid);
        static void MeshFilterComponent_SetMeshBounds(uint32_t entity_uuid, Bounds_AABB value);

        static uint32_t MeshFilterComponent_CopyMesh(uint32_t entity_uuid);

#pragma endregion

#pragma region MeshRendererComponent

        static bool MeshRendererComponent_GetIsActive(UUID entity_uuid);
        static void MeshRendererComponent_SetIsActive(UUID entity_uuid, bool value);

        static bool MeshRendererComponent_GetIsCastingShadows(UUID entity_uuid);
        static void MeshRendererComponent_SetCastingShadows(UUID entity_uuid, bool value);

        static void MeshRendererComponent_GetAllMaterialsCount(UUID entity_uuid, size_t* material_count);
        static void MeshRendererComponent_GetAllMaterialsCopy(UUID entity_uuid, uint32_t* material_array, size_t material_count);
        static void MeshRendererComponent_SetAllMaterials(UUID entity_uuid, const uint32_t* material_array, size_t material_count);

        static uint32_t MeshRendererComponent_GetMaterial(UUID entity_uuid, size_t material_index);
        static void MeshRendererComponent_SetMaterial(UUID entity_uuid, size_t material_index, UUID material_handle);

        static void MeshRendererComponent_EnableUniformBlock(UUID entity_uuid, uint32_t material_index);
        static void MeshRendererComponent_DisableUniformBlock(UUID entity_uuid, uint32_t material_index);

        static void MeshRendererComponent_EnableAllUniformBlocks(UUID entity_uuid);
        static void MeshRendererComponent_DisableAllUniformBlocks(UUID entity_uuid);

        static MaterialUniformBlock* MeshRendererComponent_GetUniformBlock(UUID entity_uuid, uint32_t material_index);

#pragma endregion

#pragma region LOD Component

        static bool LODMeshComponent_GetUseMaxDistanceOverFarPlane(UUID entity_uuid);
        static void LODMeshComponent_SetMaxDistanceOverFarPlane(UUID entity_uuid, bool value);

        static float LODMeshComponent_GetMaxDistance(UUID entity_uuid);
        static void LODMeshComponent_SetMaxDistance(UUID entity_uuid, float value);

        static size_t LODMeshComponent_GetElementCount(UUID entity_uuid);
        static void LODMeshComponent_GetDistances(UUID entity_uuid, float* distance_array, size_t element_count);
        static size_t LODMeshComponent_GetEntityCount(UUID entity_uuid, size_t element_index);
        static void LODMeshComponent_GetEntityArray(UUID entity_uuid, uint32_t* entity_array, size_t entity_count, size_t element_index);

        static void LODMeshComponent_SetLODElements(UUID entity_uuid, const float* distance_array, const size_t* entity_counts, const uint32_t* entity_array, size_t element_count);

#pragma endregion

#pragma region Skinned Mesh Component

        static bool SkinnedMeshComponent_GetIsActive(UUID entity_uuid);
        static void SkinnedMeshComponent_SetIsActive(UUID entity_uuid, bool value);

        static bool SkinnedMeshComponent_GetIsCastingShadows(UUID entity_uuid);
        static void SkinnedMeshComponent_SetCastingShadows(UUID entity_uuid, bool value);

        static uint32_t SkinnedMeshComponent_GetMeshAssetHandle(UUID entity_uuid);
        static void SkinnedMeshComponent_SetMeshAssetHandle(UUID entity_uuid, AssetHandle value);

        static uint32_t SkinnedMeshComponent_GetSkeletonAssetHandle(UUID entity_uuid);
        static void SkinnedMeshComponent_SetSkeletonAssetHandle(UUID entity_uuid, AssetHandle value);

        static Bounds_AABB SkinnedMeshComponent_GetMeshBounds(uint32_t entity_uuid);
        static void SkinnedMeshComponent_SetMeshBounds(uint32_t entity_uuid, Bounds_AABB value);

        static void SkinnedMeshComponent_GetAllMaterialsCount(UUID entity_uuid, size_t* material_count);
        static void SkinnedMeshComponent_GetAllMaterialsCopy(UUID entity_uuid, uint32_t* material_array, size_t material_count);
        static void SkinnedMeshComponent_SetAllMaterials(UUID entity_uuid, const uint32_t* material_array, size_t material_count);

        static uint32_t SkinnedMeshComponent_GetMaterial(UUID entity_uuid, size_t material_index);
        static void SkinnedMeshComponent_SetMaterial(UUID entity_uuid, size_t material_index, UUID material_handle);

        static void SkinnedMeshComponent_GetBoneMapCount(UUID entity_uuid, size_t* bone_count);
        static void SkinnedMeshComponent_GetBoneMapEntities(UUID entity_uuid, uint32_t* key_array, uint32_t* entity_array, size_t bone_count);
        static void SkinnedMeshComponent_SetBoneMapEntities(UUID entity_uuid, const uint32_t* key_array, const uint32_t* entity_array, size_t bone_count);

#pragma endregion

#pragma region Skybox

        static uint32_t SkyboxComponent_GetSkyboxMaterialAssetHandle(UUID entity_uuid);
        static void SkyboxComponent_SetSkyboxMaterialAssetHandle(UUID entity_uuid, AssetHandle value);

#pragma endregion

#pragma region Point Light Component

        static bool PointLightComponent_GetIsActive(UUID entity_uuid);
        static void PointLightComponent_SetIsActive(UUID entity_uuid, bool value);

        static float PointLightComponent_GetRadius(UUID entity_uuid);
        static void PointLightComponent_SetRadius(UUID entity_uuid, float value);

        static float PointLightComponent_GetIntensity(UUID entity_uuid);
        static void PointLightComponent_SetIntensity(UUID entity_uuid, float value);

        static glm::vec4 PointLightComponent_GetColour(UUID entity_uuid);
        static void PointLightComponent_SetColour(UUID entity_uuid, const glm::vec4* value);

        static uint8_t PointLightComponent_GetShadowFlag(UUID entity_uuid);
        static void PointLightComponent_SetShadowFlag(UUID entity_uuid, ShadowTypeFlag value);

#pragma endregion

#pragma region Spot Light Component

        static bool SpotLightComponent_GetIsActive(UUID entity_uuid);
        static void SpotLightComponent_SetIsActive(UUID entity_uuid, bool value);

        static glm::vec4 SpotLightComponent_GetColour(UUID entity_uuid);
        static void SpotLightComponent_SetColour(UUID entity_uuid, const glm::vec4* value);

        static float SpotLightComponent_GetRange(UUID entity_uuid);
        static void SpotLightComponent_SetRange(UUID entity_uuid, float value);

        static float SpotLightComponent_GetAngle(UUID entity_uuid);
        static void SpotLightComponent_SetAngle(UUID entity_uuid, float value);

        static float SpotLightComponent_GetIntensity(UUID entity_uuid);
        static void SpotLightComponent_SetIntensity(UUID entity_uuid, float value);

        static uint8_t SpotLightComponent_GetShadowFlag(UUID entity_uuid);
        static void SpotLightComponent_SetShadowFlag(UUID entity_uuid, ShadowTypeFlag value);

#pragma endregion

#pragma region Directional Light Component

        static bool DirectionalLightComponent_GetIsActive(UUID entity_uuid);
        static void DirectionalLightComponent_SetIsActive(UUID entity_uuid, bool value);

        static float DirectionalLightComponent_GetIntensity(UUID entity_uuid);
        static void DirectionalLightComponent_SetIntensity(UUID entity_uuid, float value);

        static float DirectionalLightComponent_GetMaxShadowDistance(UUID entity_uuid);
        static void DirectionalLightComponent_SetMaxShadowDistance(UUID entity_uuid, float value);

        static glm::vec4 DirectionalLightComponent_GetColour(UUID entity_uuid);
        static void DirectionalLightComponent_SetColour(UUID entity_uuid, const glm::vec4* value);

        static uint8_t DirectionalLightComponent_GetShadowFlag(UUID entity_uuid);
        static void DirectionalLightComponent_SetShadowFlag(UUID entity_uuid, uint8_t value);

#pragma endregion

#pragma region Physics

        struct _PhysicsMaterial {
            float m_DynamicFriction;
            float m_StaticFriction;
            float m_Bounciness;
        };

#pragma region RigidbodyComponent Component

        static float RigidbodyComponent_GetMass(UUID entity_uuid);
        static void RigidbodyComponent_SetMass(UUID entity_uuid, float value);

        static float RigidbodyComponent_GetDrag(UUID entity_uuid);
        static void RigidbodyComponent_SetDrag(UUID entity_uuid, float value);

        static float RigidbodyComponent_GetAngularDrag(UUID entity_uuid);
        static void RigidbodyComponent_SetAngularDrag(UUID entity_uuid, float value);

        static bool RigidbodyComponent_GetAutomaticCentreOfMass(UUID entity_uuid);
        static void RigidbodyComponent_SetAutomaticCentreOfMass(UUID entity_uuid, bool value);

        static bool RigidbodyComponent_GetUseGravity(UUID entity_uuid);
        static void RigidbodyComponent_SetUseGravity(UUID entity_uuid, bool value);

        static bool RigidbodyComponent_GetIsKinematic(UUID entity_uuid);
        static void RigidbodyComponent_SetIsKinematic(UUID entity_uuid, bool value);

        static glm::bvec3 RigidbodyComponent_GetPositionConstraint(UUID entity_uuid);
        static void RigidbodyComponent_SetPositionConstraint(UUID entity_uuid, const glm::bvec3* value);

        static glm::bvec3 RigidbodyComponent_GetRotationConstraint(UUID entity_uuid);
        static void RigidbodyComponent_SetRotationConstraint(UUID entity_uuid, const glm::bvec3* value);

        static glm::vec3 RigidbodyComponent_GetLinearVelocity(UUID entity_uuid);
        static void RigidbodyComponent_SetLinearVelocity(UUID entity_uuid, const glm::vec3* value);

        static glm::vec3 RigidbodyComponent_GetAngularVelocity(UUID entity_uuid);
        static void RigidbodyComponent_SetAngularVelocity(UUID entity_uuid, const glm::vec3* value);

        static void RigidbodyComponent_ApplyForce(UUID entity_uuid, const glm::vec3* force, uint8_t forceMode);
        static void RigidbodyComponent_ApplyTorque(UUID entity_uuid, const glm::vec3* torque);

#pragma endregion

#pragma region Box Collider Component

        static bool BoxColliderComponent_GetIsTrigger(UUID entity_uuid);
        static void BoxColliderComponent_SetIsTrigger(UUID entity_uuid, bool value);

        static glm::vec3 BoxColliderComponent_GetCentre(UUID entity_uuid);
        static void BoxColliderComponent_SetCentre(UUID entity_uuid, const glm::vec3* value);

        static glm::vec3 BoxColliderComponent_GetSize(UUID entity_uuid);
        static void BoxColliderComponent_SetSize(UUID entity_uuid, const glm::vec3* value);

        static _PhysicsMaterial BoxColliderComponent_GetMaterial(UUID entity_uuid);
        static void BoxColliderComponent_SetMaterial(UUID entity_uuid, const _PhysicsMaterial* value);

#pragma endregion

#pragma region Sphere Collider Component

        static bool SphereColliderComponent_GetIsTrigger(UUID entity_uuid);
        static void SphereColliderComponent_SetIsTrigger(UUID entity_uuid, bool value);

        static glm::vec3 SphereColliderComponent_GetCentre(UUID entity_uuid);
        static void SphereColliderComponent_SetCentre(UUID entity_uuid, const glm::vec3* value);

        static float SphereColliderComponent_GetRadius(UUID entity_uuid);
        static void SphereColliderComponent_SetRadius(UUID entity_uuid, float value);

        static _PhysicsMaterial SphereColliderComponent_GetMaterial(UUID entity_uuid);
        static void SphereColliderComponent_SetMaterial(UUID entity_uuid, const _PhysicsMaterial* value);

#pragma endregion

#pragma endregion

#pragma region Animator

        static void BasicAnimationComponent_PlayAnimation_Index(UUID entity_uuid, int32_t clip_index, bool should_loop);
        static void BasicAnimationComponent_PlayAnimation_Name(UUID entity_uuid, const char* clip_name, bool should_loop);

        static void BasicAnimationComponent_PauseAnimation(UUID entity_uuid);
        static void BasicAnimationComponent_ResumeAnimation(UUID entity_uuid);
        static void BasicAnimationComponent_StopAnimation(UUID entity_uuid);

        static bool BasicAnimationComponent_IsPlaying(UUID entity_uuid);
        static bool BasicAnimationComponent_IsLooping(UUID entity_uuid);
        static void BasicAnimationComponent_SetIsLooping(UUID entity_uuid, bool should_loop);

        static float BasicAnimationComponent_GetPlaybackSpeed(UUID entity_uuid);
        static void BasicAnimationComponent_SetPlaybackSpeed(UUID entity_uuid, float playback_speed);
        
        static float BasicAnimationComponent_GetCurrentTimestep(UUID entity_uuid);
        static void BasicAnimationComponent_SetCurrentTimestep(UUID entity_uuid, float normalised_time_step);
        
        static uint32_t BasicAnimationComponent_GetCurrentClipIndex(UUID entity_uuid);
        static const char* BasicAnimationComponent_GetCurrentClipName(UUID entity_uuid);

        static void AnimatorComponent_SetBool(UUID entity_uuid, Animation::StringHash param_hash, bool value);
        static void AnimatorComponent_SetFloat(UUID entity_uuid, Animation::StringHash param_hash, float value);
        static void AnimatorComponent_SetUInt(UUID entity_uuid, Animation::StringHash param_hash, uint32_t value);
        static void AnimatorComponent_SetInt(UUID entity_uuid, Animation::StringHash param_hash, int32_t value);

        static void AnimatorComponent_ResetMachine(UUID entity_uuid);

#pragma endregion

#pragma region Compute Shader

        static void ComputeShader_SetBuffer(UUID asset_handle, ComputeBuffer* buffer, uint32_t binding_index);
        static void ComputeShader_Dispatch(UUID asset_handle, uint32_t x, uint32_t y, uint32_t z);
        static void ComputeShader_SetBool(UUID asset_handle, const char* name, bool value);
        static void ComputeShader_SetInt(UUID asset_handle, const char* name, int32_t value);
        static void ComputeShader_SetUInt(UUID asset_handle, const char* name, uint32_t value);
        static void ComputeShader_SetFloat(UUID asset_handle, const char* name, float value);
        static void ComputeShader_SetVector2(UUID asset_handle, const char* name, glm::vec2 value);
        static void ComputeShader_SetVector3(UUID asset_handle, const char* name, glm::vec3 value);
        static void ComputeShader_SetVector4(UUID asset_handle, const char* name, glm::vec4 value);
        static void ComputeShader_SetMat3(UUID asset_handle, const char* name, const float* value);
        static void ComputeShader_SetMat4(UUID asset_handle, const char* name, const float* value);

#pragma endregion

#pragma region Compute Buffer

        static ComputeBuffer* ComputeBuffer_Create(size_t element_count, size_t element_size);
        static void ComputeBuffer_SetData(ComputeBuffer* buffer, const void* data, size_t element_count, size_t size_of_elements);
        static void ComputeBuffer_GetData(ComputeBuffer* buffer, void* output, size_t element_count, size_t size_of_elements);
        static void ComputeBuffer_Release(ComputeBuffer* buffer);

#pragma endregion

#pragma region Material

        static uint32_t Material_Create(const char* material_name);
        static void Material_SetShader(UUID asset_handle, UUID shader_handle);
        static uint32_t Material_GetShader(UUID asset_handle);
        static void Material_Destroy(UUID asset_handle);

#pragma endregion

#pragma region MaterialUniformBlock

        static void MaterialUniformBlock_SetUniform(MaterialUniformBlock* uniform_block, const char* uniform_name, GLSLType type, void* value);

        static void MaterialUniformBlock_OverrideAlbedoMap(MaterialUniformBlock* uniform_block, UUID asset_handle);
        static void MaterialUniformBlock_OverrideMetallicMap(MaterialUniformBlock* uniform_block, UUID asset_handle);
        static void MaterialUniformBlock_OverrideNormalMap(MaterialUniformBlock* uniform_block, UUID asset_handle);

        static void MaterialUniformBlock_OverrideAlbedoTint(MaterialUniformBlock* uniform_block, const glm::vec4* value);
        static void MaterialUniformBlock_OverrideMetallic(MaterialUniformBlock* uniform_block, float value);
        static void MaterialUniformBlock_OverrideRoughness(MaterialUniformBlock* uniform_block, float value);

#pragma endregion

#pragma region Texture2D

        static uint32_t Texture2D_Create(int width, int height, uint8_t internal_format);
        static uint32_t Texture2D_CreateWithData(unsigned char* data, int width, int height, uint8_t internal_format, uint8_t data_format);

        static void Texture2D_SetPixel(uint32_t asset_handle, const glm::vec4* colour, const glm::ivec2* pixel_coord);
        static void Texture2D_SetPixelData(uint32_t asset_handle, unsigned char* pixel_data, size_t pixel_data_size, uint8_t pixel_data_format);
        static void Texture2D_SubmitTextureChanges(uint32_t asset_handle);

        static void Texture2D_Destroy(AssetHandle handle);

#pragma endregion

#pragma region Mesh

        // Mesh Functions
        static uint32_t Mesh_CreateNewMesh();
        static void Mesh_Destroy(AssetHandle handle);

        static void Mesh_SubmitChanges(UUID asset_handle, bool clearCPUData);
        static void Mesh_CopyBufferDataToCPU(UUID asset_handle);
        static void Mesh_ClearBufferDataFromCPU(UUID asset_handle);

        static void Mesh_SetVertices(UUID asset_handle, const float* data, size_t data_size);
        static void Mesh_SetNormals(UUID asset_handle, const float* data, size_t data_size);
        static void Mesh_SetTextureCoords(UUID asset_handle, const float* data, size_t data_size);
        static void Mesh_SetTangents(UUID asset_handle, const float* data, size_t data_size);
        static void Mesh_SetBitangents(UUID asset_handle, const float* data, size_t data_size);
        static void Mesh_SetBoneIDs(UUID asset_handle, const glm::ivec4* data, size_t data_size);
        static void Mesh_SetBoneWeights(UUID asset_handle, const glm::vec4* data, size_t data_size);

        // pointer to const float*, pointer to data size_t
        static void Mesh_GetVertices(UUID asset_handle, const float** data, size_t* data_size);
        static void Mesh_GetNormals(UUID asset_handle, const float** data, size_t* data_size);
        static void Mesh_GetTextureCoords(UUID asset_handle, const float** data, size_t* data_size);
        static void Mesh_GetTangents(UUID asset_handle, const float** data, size_t* data_size);
        static void Mesh_GetBitangents(UUID asset_handle, const float** data, size_t* data_size);
		static void Mesh_GetBoneIDs(UUID asset_handle, const glm::ivec4** data, size_t* data_size);
		static void Mesh_GetBoneWeights(UUID asset_handle, const glm::vec4** data, size_t* data_size);

        static void Mesh_SetTriangles(UUID asset_handle, const uint32_t* data, size_t data_size);
        static void Mesh_GetTriangles(UUID asset_handle, const uint32_t** data, size_t* data_size);

        static void Mesh_RecalculateNormals(UUID asset_handle);

#pragma endregion

    };

}