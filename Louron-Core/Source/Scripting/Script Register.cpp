#include "Script Register.h"

#include "../Scene/Scene.h"
#include "../Scene/Entity.h"
#include "../Project/Project.h"
#include "../Core/Time.h"
#include "../Core/Input.h"
#include "../Core/Engine.h"
#include "../Animation/Animations.h"

#include <glm/gtx/string_cast.hpp>

namespace Louron
{

    bool ScriptRegister::RegisterAll(void* script_assembly)
    {

    #if defined(L_PLATFORM_WINDOWS)
        
        auto register_function = (RegisterHostFunctionFn)GetProcAddress((HMODULE)script_assembly, "RegisterHostFunction");
    
    #elif defined(L_PLATFORM_LINUX)
    
        auto register_function = (RegisterHostFunctionFn)dlsym(script_assembly, "RegisterHostFunction");
        
    #endif

        if (!register_function)
        {
            L_CORE_ERROR("Could Not Find RegisterHostFunction in ScriptDLL.");
            return false;
        }

        // Register Functions   

#pragma region Debug

        register_function("Debug_NativeLogMessage", reinterpret_cast<void*>(&Debug_NativeLogMessage));

#pragma endregion

#pragma region Time

        register_function("Time_GetDeltaTime", reinterpret_cast<void*>(&Time_GetDeltaTime));
        register_function("Time_GetFixedDeltaTime", reinterpret_cast<void*>(&Time_GetFixedDeltaTime));
        register_function("Time_GetUnscaledDeltaTime", reinterpret_cast<void*>(&Time_GetUnscaledDeltaTime));
        register_function("Time_GetUnscaledFixedDeltaTime", reinterpret_cast<void*>(&Time_GetUnscaledFixedDeltaTime));
        register_function("Time_GetTimeScale", reinterpret_cast<void*>(&Time_GetTimeScale));
        register_function("Time_SetTimeScale", reinterpret_cast<void*>(&Time_SetTimeScale));
        register_function("Time_GetFrameRate", reinterpret_cast<void*>(&Time_GetFrameRate));
        register_function("Time_GetCurrTime", reinterpret_cast<void*>(&Time_GetCurrTime));
		register_function("Time_SetFixedDeltaTime", reinterpret_cast<void*>(&Time_SetFixedDeltaTime));

#pragma endregion

#pragma region Input

        register_function("Input_GetKey", reinterpret_cast<void*>(&Input_GetKey));
        register_function("Input_GetKeyDown", reinterpret_cast<void*>(&Input_GetKeyDown));
        register_function("Input_GetKeyUp", reinterpret_cast<void*>(&Input_GetKeyUp));
        register_function("Input_GetMouseButton", reinterpret_cast<void*>(&Input_GetMouseButton));
        register_function("Input_GetMouseButtonDown", reinterpret_cast<void*>(&Input_GetMouseButtonDown));
        register_function("Input_GetMouseButtonUp", reinterpret_cast<void*>(&Input_GetMouseButtonUp));
        register_function("Input_GetMousePosition", reinterpret_cast<void*>(&Input_GetMousePosition));

#pragma endregion

#pragma region Entity

        register_function("Entity_Instantiate", reinterpret_cast<void*>(&Entity_Instantiate));
        register_function("Entity_Instantiate_Transform", reinterpret_cast<void*>(&Entity_Instantiate_Transform));

        register_function("Entity_Create", reinterpret_cast<void*>(&Entity_Create));
        register_function("Entity_Destroy", reinterpret_cast<void*>(&Entity_Destroy));

        register_function("Entity_FindByName", reinterpret_cast<void*>(&Entity_FindByName));
        register_function("Entity_FindByUUID", reinterpret_cast<void*>(&Entity_FindByUUID));

        register_function("Entity_GetParent", reinterpret_cast<void*>(&Entity_GetParent));
        register_function("Entity_SetParent", reinterpret_cast<void*>(&Entity_SetParent));

        register_function("Entity_CheckValidEntity", reinterpret_cast<void*>(&Entity_CheckValidEntity));

        register_function("Entity_AddComponent", reinterpret_cast<void*>(&Entity_AddComponent));
        register_function("Entity_RemoveComponent", reinterpret_cast<void*>(&Entity_RemoveComponent));
        register_function("Entity_HasComponent", reinterpret_cast<void*>(&Entity_HasComponent));

        register_function("Entity_GetComponentInParent", reinterpret_cast<void*>(&Entity_GetComponentInParent));
        register_function("Entity_GetComponentInChildren", reinterpret_cast<void*>(&Entity_GetComponentInChildren));

        register_function("Entity_GetComponentsInParentCount", reinterpret_cast<void*>(&Entity_GetComponentsInParentCount));
        register_function("Entity_GetComponentsInChildrenCount", reinterpret_cast<void*>(&Entity_GetComponentsInChildrenCount));
        register_function("Entity_GetComponentsInHierarchyCopy", reinterpret_cast<void*>(&Entity_GetComponentsInHierarchyCopy));

#pragma endregion

#pragma region TagComponent

        register_function("TagComponent_GetTag", reinterpret_cast<void*>(&TagComponent_GetTag));
        register_function("TagComponent_SetTag", reinterpret_cast<void*>(&TagComponent_SetTag));

#pragma endregion

#pragma region Transform

        register_function("TransformComponent_GetTransform", reinterpret_cast<void*>(&TransformComponent_GetTransform));
        register_function("TransformComponent_SetTransform", reinterpret_cast<void*>(&TransformComponent_SetTransform));
        register_function("TransformComponent_GetPosition", reinterpret_cast<void*>(&TransformComponent_GetPosition));
        register_function("TransformComponent_SetPosition", reinterpret_cast<void*>(&TransformComponent_SetPosition));
        register_function("TransformComponent_GetRotation", reinterpret_cast<void*>(&TransformComponent_GetRotation));
        register_function("TransformComponent_SetRotation", reinterpret_cast<void*>(&TransformComponent_SetRotation));
        register_function("TransformComponent_GetScale", reinterpret_cast<void*>(&TransformComponent_GetScale));
        register_function("TransformComponent_SetScale", reinterpret_cast<void*>(&TransformComponent_SetScale));
        register_function("TransformComponent_GetFront", reinterpret_cast<void*>(&TransformComponent_GetFront));
        register_function("TransformComponent_SetFront", reinterpret_cast<void*>(&TransformComponent_SetFront));
        register_function("TransformComponent_GetUp", reinterpret_cast<void*>(&TransformComponent_GetUp));
        register_function("TransformComponent_GetRight", reinterpret_cast<void*>(&TransformComponent_GetRight));

#pragma endregion

#pragma region Camera Component

        register_function("CameraComponent_GetClearFlag", reinterpret_cast<void*>(&CameraComponent_GetClearFlag));
        register_function("CameraComponent_SetClearFlag", reinterpret_cast<void*>(&CameraComponent_SetClearFlag));
        register_function("CameraComponent_GetClearColour", reinterpret_cast<void*>(&CameraComponent_GetClearColour));
        register_function("CameraComponent_SetClearColour", reinterpret_cast<void*>(&CameraComponent_SetClearColour));
        register_function("CameraComponent_GetCameraDepth", reinterpret_cast<void*>(&CameraComponent_GetCameraDepth));
        register_function("CameraComponent_SetCameraDepth", reinterpret_cast<void*>(&CameraComponent_SetCameraDepth));
        register_function("CameraComponent_GetDisplayingToViewport", reinterpret_cast<void*>(&CameraComponent_GetDisplayingToViewport));
        register_function("CameraComponent_SetDisplayingToViewport", reinterpret_cast<void*>(&CameraComponent_SetDisplayingToViewport));
        register_function("CameraComponent_GetViewport", reinterpret_cast<void*>(&CameraComponent_GetViewport));
        register_function("CameraComponent_SetViewport", reinterpret_cast<void*>(&CameraComponent_SetViewport));

#pragma endregion

#pragma region MeshFilterComponent

        register_function("MeshFilterComponent_GetMeshAssetHandle", reinterpret_cast<void*>(&MeshFilterComponent_GetMeshAssetHandle));
        register_function("MeshFilterComponent_SetMeshAssetHandle", reinterpret_cast<void*>(&MeshFilterComponent_SetMeshAssetHandle));

        register_function("MeshFilterComponent_GetMeshBounds", reinterpret_cast<void*>(&MeshFilterComponent_GetMeshBounds));
        register_function("MeshFilterComponent_SetMeshBounds", reinterpret_cast<void*>(&MeshFilterComponent_SetMeshBounds));

        register_function("MeshFilterComponent_CopyMesh", reinterpret_cast<void*>(&MeshFilterComponent_CopyMesh));

#pragma endregion

#pragma region MeshRendererComponent

        register_function("MeshRendererComponent_GetIsActive", reinterpret_cast<void*>(&MeshRendererComponent_GetIsActive));
        register_function("MeshRendererComponent_SetIsActive", reinterpret_cast<void*>(&MeshRendererComponent_SetIsActive));

        register_function("MeshRendererComponent_GetIsCastingShadows", reinterpret_cast<void*>(&MeshRendererComponent_GetIsCastingShadows));
        register_function("MeshRendererComponent_SetCastingShadows", reinterpret_cast<void*>(&MeshRendererComponent_SetCastingShadows));

        register_function("MeshRendererComponent_GetAllMaterialsCount", reinterpret_cast<void*>(&MeshRendererComponent_GetAllMaterialsCount));
        register_function("MeshRendererComponent_GetAllMaterialsCopy", reinterpret_cast<void*>(&MeshRendererComponent_GetAllMaterialsCopy));
        register_function("MeshRendererComponent_SetAllMaterials", reinterpret_cast<void*>(&MeshRendererComponent_SetAllMaterials));

        register_function("MeshRendererComponent_GetMaterial", reinterpret_cast<void*>(&MeshRendererComponent_GetMaterial));
        register_function("MeshRendererComponent_SetMaterial", reinterpret_cast<void*>(&MeshRendererComponent_SetMaterial));

        register_function("MeshRendererComponent_EnableUniformBlock", reinterpret_cast<void*>(&MeshRendererComponent_EnableUniformBlock));
        register_function("MeshRendererComponent_DisableUniformBlock", reinterpret_cast<void*>(&MeshRendererComponent_DisableUniformBlock));
        register_function("MeshRendererComponent_EnableAllUniformBlocks", reinterpret_cast<void*>(&MeshRendererComponent_EnableAllUniformBlocks));
        register_function("MeshRendererComponent_DisableAllUniformBlocks", reinterpret_cast<void*>(&MeshRendererComponent_DisableAllUniformBlocks));
        register_function("MeshRendererComponent_GetUniformBlock", reinterpret_cast<void*>(&MeshRendererComponent_GetUniformBlock));

#pragma endregion

#pragma region LOD Component

        register_function("LODMeshComponent_GetUseMaxDistanceOverFarPlane", reinterpret_cast<void*>(&LODMeshComponent_GetUseMaxDistanceOverFarPlane));
        register_function("LODMeshComponent_SetMaxDistanceOverFarPlane", reinterpret_cast<void*>(&LODMeshComponent_SetMaxDistanceOverFarPlane));

        register_function("LODMeshComponent_GetMaxDistance", reinterpret_cast<void*>(&LODMeshComponent_GetMaxDistance));
        register_function("LODMeshComponent_SetMaxDistance", reinterpret_cast<void*>(&LODMeshComponent_SetMaxDistance));

        register_function("LODMeshComponent_GetElementCount", reinterpret_cast<void*>(&LODMeshComponent_GetElementCount));
        register_function("LODMeshComponent_GetDistances", reinterpret_cast<void*>(&LODMeshComponent_GetDistances));
        register_function("LODMeshComponent_GetEntityCount", reinterpret_cast<void*>(&LODMeshComponent_GetEntityCount));
        register_function("LODMeshComponent_GetEntityArray", reinterpret_cast<void*>(&LODMeshComponent_GetEntityArray));

        register_function("LODMeshComponent_SetLODElements", reinterpret_cast<void*>(&LODMeshComponent_SetLODElements));
        
#pragma endregion

#pragma region Skinned Mesh

        register_function("SkinnedMeshComponent_GetIsActive", reinterpret_cast<void*>(&SkinnedMeshComponent_GetIsActive));
        register_function("SkinnedMeshComponent_SetIsActive", reinterpret_cast<void*>(&SkinnedMeshComponent_SetIsActive));

        register_function("SkinnedMeshComponent_GetIsCastingShadows", reinterpret_cast<void*>(&SkinnedMeshComponent_GetIsCastingShadows));
        register_function("SkinnedMeshComponent_SetCastingShadows", reinterpret_cast<void*>(&SkinnedMeshComponent_SetCastingShadows));

        register_function("SkinnedMeshComponent_GetMeshAssetHandle", reinterpret_cast<void*>(&SkinnedMeshComponent_GetMeshAssetHandle));
        register_function("SkinnedMeshComponent_SetMeshAssetHandle", reinterpret_cast<void*>(&SkinnedMeshComponent_SetMeshAssetHandle));

        register_function("SkinnedMeshComponent_GetSkeletonAssetHandle", reinterpret_cast<void*>(&SkinnedMeshComponent_GetSkeletonAssetHandle));
        register_function("SkinnedMeshComponent_SetSkeletonAssetHandle", reinterpret_cast<void*>(&SkinnedMeshComponent_SetSkeletonAssetHandle));

        register_function("SkinnedMeshComponent_GetMeshBounds", reinterpret_cast<void*>(&SkinnedMeshComponent_GetMeshBounds));
        register_function("SkinnedMeshComponent_SetMeshBounds", reinterpret_cast<void*>(&SkinnedMeshComponent_SetMeshBounds));

        register_function("SkinnedMeshComponent_GetAllMaterialsCount", reinterpret_cast<void*>(&SkinnedMeshComponent_GetAllMaterialsCount));
        register_function("SkinnedMeshComponent_GetAllMaterialsCopy", reinterpret_cast<void*>(&SkinnedMeshComponent_GetAllMaterialsCopy));
        register_function("SkinnedMeshComponent_SetAllMaterials", reinterpret_cast<void*>(&SkinnedMeshComponent_SetAllMaterials));

        register_function("SkinnedMeshComponent_GetMaterial", reinterpret_cast<void*>(&SkinnedMeshComponent_GetMaterial));
        register_function("SkinnedMeshComponent_SetMaterial", reinterpret_cast<void*>(&SkinnedMeshComponent_SetMaterial));

        register_function("SkinnedMeshComponent_GetBoneMapCount", reinterpret_cast<void*>(&SkinnedMeshComponent_GetBoneMapCount));
        register_function("SkinnedMeshComponent_GetBoneMapEntities", reinterpret_cast<void*>(&SkinnedMeshComponent_GetBoneMapEntities));
        register_function("SkinnedMeshComponent_SetBoneMapEntities", reinterpret_cast<void*>(&SkinnedMeshComponent_SetBoneMapEntities));

#pragma endregion

#pragma region Skybox

        register_function("SkyboxComponent_GetSkyboxMaterialAssetHandle", reinterpret_cast<void*>(&SkyboxComponent_GetSkyboxMaterialAssetHandle));
        register_function("SkyboxComponent_SetSkyboxMaterialAssetHandle", reinterpret_cast<void*>(&SkyboxComponent_SetSkyboxMaterialAssetHandle));

#pragma endregion

#pragma region Point Light Component

        register_function("PointLightComponent_GetIsActive", reinterpret_cast<void*>(&PointLightComponent_GetIsActive));
        register_function("PointLightComponent_SetIsActive", reinterpret_cast<void*>(&PointLightComponent_SetIsActive));

        register_function("PointLightComponent_GetRadius", reinterpret_cast<void*>(&PointLightComponent_GetRadius));
        register_function("PointLightComponent_SetRadius", reinterpret_cast<void*>(&PointLightComponent_SetRadius));

        register_function("PointLightComponent_GetIntensity", reinterpret_cast<void*>(&PointLightComponent_GetIntensity));
        register_function("PointLightComponent_SetIntensity", reinterpret_cast<void*>(&PointLightComponent_SetIntensity));

        register_function("PointLightComponent_GetColour", reinterpret_cast<void*>(&PointLightComponent_GetColour));
        register_function("PointLightComponent_SetColour", reinterpret_cast<void*>(&PointLightComponent_SetColour));

        register_function("PointLightComponent_GetShadowFlag", reinterpret_cast<void*>(&PointLightComponent_GetShadowFlag));
        register_function("PointLightComponent_SetShadowFlag", reinterpret_cast<void*>(&PointLightComponent_SetShadowFlag));

#pragma endregion

#pragma region Spot Light Component

        register_function("SpotLightComponent_GetIsActive", reinterpret_cast<void*>(&SpotLightComponent_GetIsActive));
        register_function("SpotLightComponent_SetIsActive", reinterpret_cast<void*>(&SpotLightComponent_SetIsActive));

        register_function("SpotLightComponent_GetRange", reinterpret_cast<void*>(&SpotLightComponent_GetRange));
        register_function("SpotLightComponent_SetRange", reinterpret_cast<void*>(&SpotLightComponent_SetRange));

        register_function("SpotLightComponent_GetAngle", reinterpret_cast<void*>(&SpotLightComponent_GetAngle));
        register_function("SpotLightComponent_SetAngle", reinterpret_cast<void*>(&SpotLightComponent_SetAngle));

        register_function("SpotLightComponent_GetIntensity", reinterpret_cast<void*>(&SpotLightComponent_GetIntensity));
        register_function("SpotLightComponent_SetIntensity", reinterpret_cast<void*>(&SpotLightComponent_SetIntensity));

        register_function("SpotLightComponent_GetColour", reinterpret_cast<void*>(&SpotLightComponent_GetColour));
        register_function("SpotLightComponent_SetColour", reinterpret_cast<void*>(&SpotLightComponent_SetColour));

        register_function("SpotLightComponent_GetShadowFlag", reinterpret_cast<void*>(&SpotLightComponent_GetShadowFlag));
        register_function("SpotLightComponent_SetShadowFlag", reinterpret_cast<void*>(&SpotLightComponent_SetShadowFlag));

#pragma endregion

#pragma region Entity

        register_function("DirectionalLightComponent_GetIsActive", reinterpret_cast<void*>(&DirectionalLightComponent_GetIsActive));
        register_function("DirectionalLightComponent_SetIsActive", reinterpret_cast<void*>(&DirectionalLightComponent_SetIsActive));

        register_function("DirectionalLightComponent_GetIntensity", reinterpret_cast<void*>(&DirectionalLightComponent_GetIntensity));
        register_function("DirectionalLightComponent_SetIntensity", reinterpret_cast<void*>(&DirectionalLightComponent_SetIntensity));

        register_function("DirectionalLightComponent_GetMaxShadowDistance", reinterpret_cast<void*>(&DirectionalLightComponent_GetMaxShadowDistance));
        register_function("DirectionalLightComponent_SetMaxShadowDistance", reinterpret_cast<void*>(&DirectionalLightComponent_SetMaxShadowDistance));

        register_function("DirectionalLightComponent_GetColour", reinterpret_cast<void*>(&DirectionalLightComponent_GetColour));
        register_function("DirectionalLightComponent_SetColour", reinterpret_cast<void*>(&DirectionalLightComponent_SetColour));

        register_function("DirectionalLightComponent_GetShadowFlag", reinterpret_cast<void*>(&DirectionalLightComponent_GetShadowFlag));
        register_function("DirectionalLightComponent_SetShadowFlag", reinterpret_cast<void*>(&DirectionalLightComponent_SetShadowFlag));

#pragma endregion

#pragma region RigidbodyComponent

        register_function("RigidbodyComponent_GetMass", reinterpret_cast<void*>(&RigidbodyComponent_GetMass));
        register_function("RigidbodyComponent_SetMass", reinterpret_cast<void*>(&RigidbodyComponent_SetMass));

        register_function("RigidbodyComponent_GetDrag", reinterpret_cast<void*>(&RigidbodyComponent_GetDrag));
        register_function("RigidbodyComponent_SetDrag", reinterpret_cast<void*>(&RigidbodyComponent_SetDrag));

        register_function("RigidbodyComponent_GetAngularDrag", reinterpret_cast<void*>(&RigidbodyComponent_GetAngularDrag));
        register_function("RigidbodyComponent_SetAngularDrag", reinterpret_cast<void*>(&RigidbodyComponent_SetAngularDrag));

        register_function("RigidbodyComponent_GetAutomaticCentreOfMass", reinterpret_cast<void*>(&RigidbodyComponent_GetAutomaticCentreOfMass));
        register_function("RigidbodyComponent_SetAutomaticCentreOfMass", reinterpret_cast<void*>(&RigidbodyComponent_SetAutomaticCentreOfMass));

        register_function("RigidbodyComponent_GetUseGravity", reinterpret_cast<void*>(&RigidbodyComponent_GetUseGravity));
        register_function("RigidbodyComponent_SetUseGravity", reinterpret_cast<void*>(&RigidbodyComponent_SetUseGravity));

        register_function("RigidbodyComponent_GetIsKinematic", reinterpret_cast<void*>(&RigidbodyComponent_GetIsKinematic));
        register_function("RigidbodyComponent_SetIsKinematic", reinterpret_cast<void*>(&RigidbodyComponent_SetIsKinematic));

        register_function("RigidbodyComponent_GetPositionConstraint", reinterpret_cast<void*>(&RigidbodyComponent_GetPositionConstraint));
        register_function("RigidbodyComponent_SetPositionConstraint", reinterpret_cast<void*>(&RigidbodyComponent_SetPositionConstraint));

        register_function("RigidbodyComponent_GetRotationConstraint", reinterpret_cast<void*>(&RigidbodyComponent_GetRotationConstraint));
        register_function("RigidbodyComponent_SetRotationConstraint", reinterpret_cast<void*>(&RigidbodyComponent_SetRotationConstraint));

        register_function("RigidbodyComponent_GetLinearVelocity", reinterpret_cast<void*>(&RigidbodyComponent_GetLinearVelocity));
        register_function("RigidbodyComponent_SetLinearVelocity", reinterpret_cast<void*>(&RigidbodyComponent_SetLinearVelocity));

        register_function("RigidbodyComponent_GetAngularVelocity", reinterpret_cast<void*>(&RigidbodyComponent_GetAngularVelocity));
        register_function("RigidbodyComponent_SetAngularVelocity", reinterpret_cast<void*>(&RigidbodyComponent_SetAngularVelocity));

        register_function("RigidbodyComponent_ApplyForce", reinterpret_cast<void*>(&RigidbodyComponent_ApplyForce));
        register_function("RigidbodyComponent_ApplyTorque", reinterpret_cast<void*>(&RigidbodyComponent_ApplyTorque));

#pragma endregion

#pragma region BoxColliderComponent

        register_function("BoxColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&BoxColliderComponent_GetIsTrigger));
        register_function("BoxColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&BoxColliderComponent_SetIsTrigger));

        register_function("BoxColliderComponent_GetCentre", reinterpret_cast<void*>(&BoxColliderComponent_GetCentre));
        register_function("BoxColliderComponent_SetCentre", reinterpret_cast<void*>(&BoxColliderComponent_SetCentre));

        register_function("BoxColliderComponent_GetSize", reinterpret_cast<void*>(&BoxColliderComponent_GetSize));
        register_function("BoxColliderComponent_SetSize", reinterpret_cast<void*>(&BoxColliderComponent_SetSize));

        register_function("BoxColliderComponent_GetMaterial", reinterpret_cast<void*>(&BoxColliderComponent_GetMaterial));
        register_function("BoxColliderComponent_SetMaterial", reinterpret_cast<void*>(&BoxColliderComponent_SetMaterial));

#pragma endregion

#pragma region SphereColliderComponent

        register_function("SphereColliderComponent_GetIsTrigger", reinterpret_cast<void*>(&SphereColliderComponent_GetIsTrigger));
        register_function("SphereColliderComponent_SetIsTrigger", reinterpret_cast<void*>(&SphereColliderComponent_SetIsTrigger));

        register_function("SphereColliderComponent_GetCentre", reinterpret_cast<void*>(&SphereColliderComponent_GetCentre));
        register_function("SphereColliderComponent_SetCentre", reinterpret_cast<void*>(&SphereColliderComponent_SetCentre));

        register_function("SphereColliderComponent_GetRadius", reinterpret_cast<void*>(&SphereColliderComponent_GetRadius));
        register_function("SphereColliderComponent_SetRadius", reinterpret_cast<void*>(&SphereColliderComponent_SetRadius));

        register_function("SphereColliderComponent_GetMaterial", reinterpret_cast<void*>(&SphereColliderComponent_GetMaterial));
        register_function("SphereColliderComponent_SetMaterial", reinterpret_cast<void*>(&SphereColliderComponent_SetMaterial));

#pragma endregion

#pragma region Animator

        register_function("AnimatorComponent_PlayAnimation_Index", reinterpret_cast<void*>(&AnimatorComponent_PlayAnimation_Index));
        register_function("AnimatorComponent_PlayAnimation_Name", reinterpret_cast<void*>(&AnimatorComponent_PlayAnimation_Name));

        register_function("AnimatorComponent_PauseAnimation", reinterpret_cast<void*>(&AnimatorComponent_PauseAnimation));
        register_function("AnimatorComponent_ResumeAnimation", reinterpret_cast<void*>(&AnimatorComponent_ResumeAnimation));
        register_function("AnimatorComponent_StopAnimation", reinterpret_cast<void*>(&AnimatorComponent_StopAnimation));

        register_function("AnimatorComponent_IsPlaying", reinterpret_cast<void*>(&AnimatorComponent_IsPlaying));
        register_function("AnimatorComponent_IsLooping", reinterpret_cast<void*>(&AnimatorComponent_IsLooping));
        register_function("AnimatorComponent_SetIsLooping", reinterpret_cast<void*>(&AnimatorComponent_SetIsLooping));

        register_function("AnimatorComponent_GetPlaybackSpeed", reinterpret_cast<void*>(&AnimatorComponent_GetPlaybackSpeed));
        register_function("AnimatorComponent_SetPlaybackSpeed", reinterpret_cast<void*>(&AnimatorComponent_SetPlaybackSpeed));

        register_function("AnimatorComponent_GetCurrentTimestep", reinterpret_cast<void*>(&AnimatorComponent_GetCurrentTimestep));
        register_function("AnimatorComponent_SetCurrentTimestep", reinterpret_cast<void*>(&AnimatorComponent_SetCurrentTimestep));
        
        register_function("AnimatorComponent_GetCurrentClipIndex", reinterpret_cast<void*>(&AnimatorComponent_GetCurrentClipIndex));
        register_function("AnimatorComponent_GetCurrentClipName", reinterpret_cast<void*>(&AnimatorComponent_GetCurrentClipName));

#pragma endregion

#pragma region ComputeShader

        register_function("ComputeShader_SetBuffer", reinterpret_cast<void*>(&ComputeShader_SetBuffer));
        register_function("ComputeShader_Dispatch", reinterpret_cast<void*>(&ComputeShader_Dispatch));
        register_function("ComputeShader_SetBool", reinterpret_cast<void*>(&ComputeShader_SetBool));
        register_function("ComputeShader_SetInt", reinterpret_cast<void*>(&ComputeShader_SetInt));
        register_function("ComputeShader_SetUInt", reinterpret_cast<void*>(&ComputeShader_SetUInt));
        register_function("ComputeShader_SetFloat", reinterpret_cast<void*>(&ComputeShader_SetFloat));
        register_function("ComputeShader_SetVector2", reinterpret_cast<void*>(&ComputeShader_SetVector2));
        register_function("ComputeShader_SetVector3", reinterpret_cast<void*>(&ComputeShader_SetVector3));
        register_function("ComputeShader_SetVector4", reinterpret_cast<void*>(&ComputeShader_SetVector4));
        register_function("ComputeShader_SetMat3", reinterpret_cast<void*>(&ComputeShader_SetMat3));
        register_function("ComputeShader_SetMat4", reinterpret_cast<void*>(&ComputeShader_SetMat4));

#pragma endregion

#pragma region ComputeBuffer

        register_function("ComputeBuffer_Create", reinterpret_cast<void*>(&ComputeBuffer_Create));
        register_function("ComputeBuffer_SetData", reinterpret_cast<void*>(&ComputeBuffer_SetData));
        register_function("ComputeBuffer_GetData", reinterpret_cast<void*>(&ComputeBuffer_GetData));
        register_function("ComputeBuffer_Release", reinterpret_cast<void*>(&ComputeBuffer_Release));

#pragma endregion

#pragma region Material

        register_function("Material_Create", reinterpret_cast<void*>(&Material_Create));
        register_function("Material_SetShader", reinterpret_cast<void*>(&Material_SetShader));
        register_function("Material_GetShader", reinterpret_cast<void*>(&Material_GetShader));
        register_function("Material_Destroy", reinterpret_cast<void*>(&Material_Destroy));

#pragma endregion

#pragma region MaterialUniformBlock

        register_function("MaterialUniformBlock_SetUniform", reinterpret_cast<void*>(&MaterialUniformBlock_SetUniform));
        register_function("MaterialUniformBlock_OverrideAlbedoMap", reinterpret_cast<void*>(&MaterialUniformBlock_OverrideAlbedoMap));
        register_function("MaterialUniformBlock_OverrideMetallicMap", reinterpret_cast<void*>(&MaterialUniformBlock_OverrideMetallicMap));
        register_function("MaterialUniformBlock_OverrideNormalMap", reinterpret_cast<void*>(&MaterialUniformBlock_OverrideNormalMap));
        register_function("MaterialUniformBlock_OverrideAlbedoTint", reinterpret_cast<void*>(&MaterialUniformBlock_OverrideAlbedoTint));
        register_function("MaterialUniformBlock_OverrideMetallic", reinterpret_cast<void*>(&MaterialUniformBlock_OverrideMetallic));
        register_function("MaterialUniformBlock_OverrideRoughness", reinterpret_cast<void*>(&MaterialUniformBlock_OverrideRoughness));

#pragma endregion

#pragma region Texture2D

        register_function("Texture2D_Create", reinterpret_cast<void*>(&Texture2D_Create));
        register_function("Texture2D_CreateWithData", reinterpret_cast<void*>(&Texture2D_CreateWithData));
        register_function("Texture2D_SetPixel", reinterpret_cast<void*>(&Texture2D_SetPixel));
        register_function("Texture2D_SetPixelData", reinterpret_cast<void*>(&Texture2D_SetPixelData));
        register_function("Texture2D_SubmitTextureChanges", reinterpret_cast<void*>(&Texture2D_SubmitTextureChanges));
        register_function("Texture2D_Destroy", reinterpret_cast<void*>(&Texture2D_Destroy));

#pragma endregion

#pragma region Mesh

        register_function("Mesh_CreateNewMesh", reinterpret_cast<void*>(&Mesh_CreateNewMesh));
        register_function("Mesh_Destroy", reinterpret_cast<void*>(&Mesh_Destroy));

        register_function("Mesh_SubmitChanges", reinterpret_cast<void*>(&Mesh_SubmitChanges));
        register_function("Mesh_CopyBufferDataToCPU", reinterpret_cast<void*>(&Mesh_CopyBufferDataToCPU));
        register_function("Mesh_ClearBufferDataFromCPU", reinterpret_cast<void*>(&Mesh_ClearBufferDataFromCPU));

        register_function("Mesh_SetVertices", reinterpret_cast<void*>(&Mesh_SetVertices));
        register_function("Mesh_SetNormals", reinterpret_cast<void*>(&Mesh_SetNormals));
        register_function("Mesh_SetTextureCoords", reinterpret_cast<void*>(&Mesh_SetTextureCoords));
        register_function("Mesh_SetTangents", reinterpret_cast<void*>(&Mesh_SetTangents));
        register_function("Mesh_SetBitangents", reinterpret_cast<void*>(&Mesh_SetBitangents));
        register_function("Mesh_SetBoneIDs", reinterpret_cast<void*>(&Mesh_SetBoneIDs));
        register_function("Mesh_SetBoneWeights", reinterpret_cast<void*>(&Mesh_SetBoneWeights));

        register_function("Mesh_GetVertices", reinterpret_cast<void*>(&Mesh_GetVertices));
        register_function("Mesh_GetNormals", reinterpret_cast<void*>(&Mesh_GetNormals));
        register_function("Mesh_GetTextureCoords", reinterpret_cast<void*>(&Mesh_GetTextureCoords));
        register_function("Mesh_GetTangents", reinterpret_cast<void*>(&Mesh_GetTangents));
        register_function("Mesh_GetBitangents", reinterpret_cast<void*>(&Mesh_GetBitangents));
        register_function("Mesh_SetTriangles", reinterpret_cast<void*>(&Mesh_SetTriangles));
        register_function("Mesh_GetTriangles", reinterpret_cast<void*>(&Mesh_GetTriangles));
        register_function("Mesh_GetBoneIDs", reinterpret_cast<void*>(&Mesh_GetBoneIDs));
        register_function("Mesh_GetBoneWeights", reinterpret_cast<void*>(&Mesh_GetBoneWeights));
        register_function("Mesh_RecalculateNormals", reinterpret_cast<void*>(&Mesh_RecalculateNormals));

#pragma endregion

        return true;
    }

#pragma region Debug

    void ScriptRegister::Debug_NativeLogMessage(const char* message, uint8_t type)
    {
        const char* typeStr = nullptr;
        switch (type)
        {
            default:
            case 0:     L_SCRIPT_INFO(message);     break;
            case 1:     L_SCRIPT_WARN(message);     break;
            case 2:     L_SCRIPT_ERROR(message);    break;
            case 3:     L_SCRIPT_FATAL(message);    break;
        }
    }

#pragma endregion

#pragma region Time

    float ScriptRegister::Time_GetDeltaTime() { return Time::GetDeltaTime(); }
    float ScriptRegister::Time_GetFixedDeltaTime() { return Time::GetFixedDeltaTime(); }
    float ScriptRegister::Time_GetUnscaledDeltaTime() { return Time::GetUnscaledDeltaTime(); }
    float ScriptRegister::Time_GetUnscaledFixedDeltaTime() { return Time::GetUnscaledFixedDeltaTime(); }
    float ScriptRegister::Time_GetTimeScale() { return Time::GetTimeScale(); }
    void ScriptRegister::Time_SetTimeScale(float time_scale) { Time::SetTimeScale(time_scale); }
    int ScriptRegister::Time_GetFrameRate() { return Time::GetFrameRate(); }
    float ScriptRegister::Time_GetCurrTime() { return static_cast<float>(Time::Get().GetCurrTime()); }

    void ScriptRegister::Time_SetFixedDeltaTime(float fixed_time) { Time::SetFixedDeltaTime(fixed_time); }

#pragma endregion

#pragma region Input

    bool ScriptRegister::Input_GetKey(KeyCode key_code) { return Engine::Get().GetInput().GetKey(key_code); }
    bool ScriptRegister::Input_GetKeyDown(KeyCode key_code) { return Engine::Get().GetInput().GetKeyDown(key_code); }
    bool ScriptRegister::Input_GetKeyUp(KeyCode key_code) { return Engine::Get().GetInput().GetKeyUp(key_code); }

    bool ScriptRegister::Input_GetMouseButton(MouseButtonCode button_code) { return Engine::Get().GetInput().GetMouseButton(button_code); }
    bool ScriptRegister::Input_GetMouseButtonDown(MouseButtonCode button_code) { return Engine::Get().GetInput().GetMouseButtonDown(button_code); }
    bool ScriptRegister::Input_GetMouseButtonUp(MouseButtonCode button_code) { return Engine::Get().GetInput().GetMouseButtonUp(button_code); }
    glm::vec2 ScriptRegister::Input_GetMousePosition() { return Engine::Get().GetInput().GetMousePosition(); }

#pragma endregion

#pragma region Entity

    uint32_t ScriptRegister::Entity_Instantiate(uint32_t prefab_handle)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        auto prefab_asset = AssetManager::GetAsset<Prefab>(prefab_handle);
        if(!prefab_asset)
            return NULL_UUID;

        Entity prefab_clone = scene_ref->InstantiatePrefab(prefab_asset);
        if (prefab_clone) 
        {
            prefab_clone.GetComponent<TagComponent>().SetUniqueName(prefab_asset->GetPrefabName());
            return prefab_clone.GetUUID();
        }
        return NULL_UUID;
    }

    uint32_t ScriptRegister::Entity_Instantiate_Transform(uint32_t prefab_handle, const _Transform* transform)
    {
        if(!transform)
            return NULL_UUID;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        auto prefab_asset = AssetManager::GetAsset<Prefab>(prefab_handle);
        if (!prefab_asset)
            return NULL_UUID;

        TransformComponent temp_transform{};
        temp_transform.m_Position = transform->position;
        temp_transform.m_Rotation = transform->rotation;
        temp_transform.m_Scale = transform->scale;

        Entity prefab_clone = scene_ref->InstantiatePrefab(prefab_asset, temp_transform);
        if (prefab_clone)
        {
            prefab_clone.GetComponent<TagComponent>().SetUniqueName(prefab_asset->GetPrefabName());
            auto& transform = prefab_clone.GetTransform();
            return prefab_clone.GetUUID();
        }
        return NULL_UUID;
    }

    uint32_t ScriptRegister::Entity_Create(const char* name)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

		return scene_ref->CreateEntity(name).GetUUID();
    }

    void ScriptRegister::Entity_Destroy(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        Engine::Get().SubmitToMainThread([scene_ref, entity]()
            {
                scene_ref->DestroyEntity(entity); // Destroy at start of next frame
            });
    }

    uint32_t ScriptRegister::Entity_FindByName(const char* name)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

		Entity entity = scene_ref->FindEntityByName(name);
		if (!entity)
			return NULL_UUID;

		return entity.GetUUID();
    }

    uint32_t ScriptRegister::Entity_FindByUUID(uint32_t entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return NULL_UUID;

        return entity.GetUUID();
    }

    UUID ScriptRegister::Entity_GetParent(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return NULL_UUID;

        return entity.GetComponent<HierarchyComponent>().GetParentID();
    }

    void ScriptRegister::Entity_SetParent(UUID entity_uuid, UUID parent_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        Entity parent_entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !parent_entity)
            return;

        entity.GetComponent<HierarchyComponent>().AttachParent(parent_uuid);
    }

    bool ScriptRegister::Entity_CheckValidEntity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        return scene_ref->FindEntityByUUID(entity_uuid).operator bool();
    }

    void ScriptRegister::Entity_AddComponent(UUID entity_uuid, ScriptFieldType component_type) 
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                  entity.AddComponent<IDComponent>();                 break;
            case ScriptFieldType::TagComponent:                 entity.AddComponent<TagComponent>();                break;
            case ScriptFieldType::HierarchyComponent:           entity.AddComponent<HierarchyComponent>();          break;
            case ScriptFieldType::ScriptComponent:              entity.AddComponent<ScriptComponent>();             break;
            case ScriptFieldType::TransformComponent:           entity.AddComponent<TransformComponent>();          break;
            case ScriptFieldType::CameraComponent:              entity.AddComponent<CameraComponent>();             break;
            case ScriptFieldType::AudioListenerComponent:       entity.AddComponent<AudioListenerComponent>();      break;
            case ScriptFieldType::AudioEmitterComponent:        entity.AddComponent<AudioEmitterComponent>();       break;
            case ScriptFieldType::MeshFilterComponent:          entity.AddComponent<MeshFilterComponent>();         break;
            case ScriptFieldType::MeshRendererComponent:        entity.AddComponent<MeshRendererComponent>();       break;
            case ScriptFieldType::LODMeshComponent:             entity.AddComponent<LODMeshComponent>();            break;
            case ScriptFieldType::SkinnedMeshComponent:         entity.AddComponent<SkinnedMeshComponent>();        break;
            case ScriptFieldType::AnimatorComponent:            entity.AddComponent<AnimatorComponent>();           break;
            case ScriptFieldType::SkyboxComponent:              entity.AddComponent<SkyboxComponent>();             break;
            case ScriptFieldType::PointLightComponent:          entity.AddComponent<PointLightComponent>();         break;
            case ScriptFieldType::SpotLightComponent:           entity.AddComponent<SpotLightComponent>();          break;
            case ScriptFieldType::DirectionalLightComponent:    entity.AddComponent<DirectionalLightComponent>();   break;
            case ScriptFieldType::RigidbodyComponent:           entity.AddComponent<RigidbodyComponent>();          break;
            case ScriptFieldType::BoxColliderComponent:         entity.AddComponent<BoxColliderComponent>();        break;
            case ScriptFieldType::SphereColliderComponent:      entity.AddComponent<SphereColliderComponent>();     break;
        }
    }

    void ScriptRegister::Entity_RemoveComponent(UUID entity_uuid, ScriptFieldType component_type)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                  entity.RemoveComponent<IDComponent>();                 break;
            case ScriptFieldType::TagComponent:                 entity.RemoveComponent<TagComponent>();                break;
            case ScriptFieldType::HierarchyComponent:           entity.RemoveComponent<HierarchyComponent>();          break;
            case ScriptFieldType::ScriptComponent:              entity.RemoveComponent<ScriptComponent>();             break;
            case ScriptFieldType::TransformComponent:           entity.RemoveComponent<TransformComponent>();          break;
            case ScriptFieldType::CameraComponent:              entity.RemoveComponent<CameraComponent>();             break;
            case ScriptFieldType::AudioListenerComponent:       entity.RemoveComponent<AudioListenerComponent>();               break;
            case ScriptFieldType::AudioEmitterComponent:        entity.RemoveComponent<AudioEmitterComponent>();                break;
            case ScriptFieldType::MeshFilterComponent:          entity.RemoveComponent<MeshFilterComponent>();         break;
            case ScriptFieldType::MeshRendererComponent:        entity.RemoveComponent<MeshRendererComponent>();       break;
            case ScriptFieldType::LODMeshComponent:             entity.RemoveComponent<LODMeshComponent>();            break;
            case ScriptFieldType::SkinnedMeshComponent:         entity.RemoveComponent<SkinnedMeshComponent>();        break;
            case ScriptFieldType::AnimatorComponent:            entity.RemoveComponent<AnimatorComponent>();           break;
            case ScriptFieldType::SkyboxComponent:              entity.RemoveComponent<SkyboxComponent>();             break;
            case ScriptFieldType::PointLightComponent:          entity.RemoveComponent<PointLightComponent>();         break;
            case ScriptFieldType::SpotLightComponent:           entity.RemoveComponent<SpotLightComponent>();          break;
            case ScriptFieldType::DirectionalLightComponent:    entity.RemoveComponent<DirectionalLightComponent>();   break;
            case ScriptFieldType::RigidbodyComponent:           entity.RemoveComponent<RigidbodyComponent>();          break;
            case ScriptFieldType::BoxColliderComponent:         entity.RemoveComponent<BoxColliderComponent>();        break;
            case ScriptFieldType::SphereColliderComponent:      entity.RemoveComponent<SphereColliderComponent>();     break;
        }
    }

    bool ScriptRegister::Entity_HasComponent(UUID entity_uuid, ScriptFieldType component_type)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return false;
        
        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                  return entity.HasComponent<IDComponent>();                 break;
            case ScriptFieldType::TagComponent:                 return entity.HasComponent<TagComponent>();                break;
            case ScriptFieldType::HierarchyComponent:           return entity.HasComponent<HierarchyComponent>();          break;
            case ScriptFieldType::ScriptComponent:              return entity.HasComponent<ScriptComponent>();             break;
            case ScriptFieldType::TransformComponent:           return entity.HasComponent<TransformComponent>();          break;
            case ScriptFieldType::CameraComponent:              return entity.HasComponent<CameraComponent>();             break;
            case ScriptFieldType::AudioListenerComponent:       return entity.HasComponent<AudioListenerComponent>();               break;
            case ScriptFieldType::AudioEmitterComponent:        return entity.HasComponent<AudioEmitterComponent>();                break;
            case ScriptFieldType::MeshFilterComponent:          return entity.HasComponent<MeshFilterComponent>();         break;
            case ScriptFieldType::MeshRendererComponent:        return entity.HasComponent<MeshRendererComponent>();       break;
            case ScriptFieldType::LODMeshComponent:             return entity.HasComponent<LODMeshComponent>();            break;
            case ScriptFieldType::SkinnedMeshComponent:         return entity.HasComponent<SkinnedMeshComponent>();        break;
            case ScriptFieldType::AnimatorComponent:            return entity.HasComponent<AnimatorComponent>();           break;
            case ScriptFieldType::SkyboxComponent:              return entity.HasComponent<SkyboxComponent>();             break;
            case ScriptFieldType::PointLightComponent:          return entity.HasComponent<PointLightComponent>();         break;
            case ScriptFieldType::SpotLightComponent:           return entity.HasComponent<SpotLightComponent>();          break;
            case ScriptFieldType::DirectionalLightComponent:    return entity.HasComponent<DirectionalLightComponent>();   break;
            case ScriptFieldType::RigidbodyComponent:           return entity.HasComponent<RigidbodyComponent>();          break;
            case ScriptFieldType::BoxColliderComponent:         return entity.HasComponent<BoxColliderComponent>();        break;
            case ScriptFieldType::SphereColliderComponent:      return entity.HasComponent<SphereColliderComponent>();     break;
        }
        return false;
    }

    uint32_t ScriptRegister::Entity_GetComponentInParent(UUID entity_uuid, ScriptFieldType component_type)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return NULL_UUID;

        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                 { auto component_entity = entity.GetComponentInParent<IDComponent>().GetEntity();               return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::TagComponent:                { auto component_entity = entity.GetComponentInParent<TagComponent>().GetEntity();              return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::HierarchyComponent:          { auto component_entity = entity.GetComponentInParent<HierarchyComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::ScriptComponent:             { auto component_entity = entity.GetComponentInParent<ScriptComponent>().GetEntity();           return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::TransformComponent:          { auto component_entity = entity.GetComponentInParent<TransformComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::CameraComponent:             { auto component_entity = entity.GetComponentInParent<CameraComponent>().GetEntity();           return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::AudioListenerComponent:      { auto component_entity = entity.GetComponentInParent<AudioListenerComponent>().GetEntity();    return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::AudioEmitterComponent:       { auto component_entity = entity.GetComponentInParent<AudioEmitterComponent>().GetEntity();     return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::MeshFilterComponent:         { auto component_entity = entity.GetComponentInParent<MeshFilterComponent>().GetEntity();       return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::MeshRendererComponent:       { auto component_entity = entity.GetComponentInParent<MeshRendererComponent>().GetEntity();     return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::LODMeshComponent:            { auto component_entity = entity.GetComponentInParent<LODMeshComponent>().GetEntity();          return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SkinnedMeshComponent:        { auto component_entity = entity.GetComponentInParent<SkinnedMeshComponent>().GetEntity();      return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::AnimatorComponent:           { auto component_entity = entity.GetComponentInParent<AnimatorComponent>().GetEntity();         return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SkyboxComponent:             { auto component_entity = entity.GetComponentInParent<SkyboxComponent>().GetEntity();           return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::PointLightComponent:         { auto component_entity = entity.GetComponentInParent<PointLightComponent>().GetEntity();       return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SpotLightComponent:          { auto component_entity = entity.GetComponentInParent<SpotLightComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::DirectionalLightComponent:   { auto component_entity = entity.GetComponentInParent<DirectionalLightComponent>().GetEntity(); return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::RigidbodyComponent:          { auto component_entity = entity.GetComponentInParent<RigidbodyComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::BoxColliderComponent:        { auto component_entity = entity.GetComponentInParent<BoxColliderComponent>().GetEntity();      return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SphereColliderComponent:     { auto component_entity = entity.GetComponentInParent<SphereColliderComponent>().GetEntity();   return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
        }
        return NULL_UUID;
    }

    uint32_t ScriptRegister::Entity_GetComponentInChildren(UUID entity_uuid, ScriptFieldType component_type)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return NULL_UUID;

        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                 { auto component_entity = entity.GetComponentInChild<IDComponent>().GetEntity();               return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::TagComponent:                { 
                auto component_entity = entity.GetComponentInChild<TagComponent>().GetEntity();              
                return component_entity ? (uint32_t)(component_entity->GetUUID()) : NULL_UUID;
            }
            case ScriptFieldType::HierarchyComponent:          { auto component_entity = entity.GetComponentInChild<HierarchyComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::ScriptComponent:             { auto component_entity = entity.GetComponentInChild<ScriptComponent>().GetEntity();           return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::TransformComponent:          { auto component_entity = entity.GetComponentInChild<TransformComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::CameraComponent:             { auto component_entity = entity.GetComponentInChild<CameraComponent>().GetEntity();           return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::AudioListenerComponent:      { auto component_entity = entity.GetComponentInChild<AudioListenerComponent>().GetEntity();    return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::AudioEmitterComponent:       { auto component_entity = entity.GetComponentInChild<AudioEmitterComponent>().GetEntity();     return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::MeshFilterComponent:         { auto component_entity = entity.GetComponentInChild<MeshFilterComponent>().GetEntity();       return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::MeshRendererComponent:       { auto component_entity = entity.GetComponentInChild<MeshRendererComponent>().GetEntity();     return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::LODMeshComponent:            { auto component_entity = entity.GetComponentInChild<LODMeshComponent>().GetEntity();          return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SkinnedMeshComponent:        { auto component_entity = entity.GetComponentInChild<SkinnedMeshComponent>().GetEntity();      return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::AnimatorComponent:           { auto component_entity = entity.GetComponentInChild<AnimatorComponent>().GetEntity();         return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SkyboxComponent:             { auto component_entity = entity.GetComponentInChild<SkyboxComponent>().GetEntity();           return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::PointLightComponent:         { auto component_entity = entity.GetComponentInChild<PointLightComponent>().GetEntity();       return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SpotLightComponent:          { auto component_entity = entity.GetComponentInChild<SpotLightComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::DirectionalLightComponent:   { auto component_entity = entity.GetComponentInChild<DirectionalLightComponent>().GetEntity(); return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::RigidbodyComponent:          { auto component_entity = entity.GetComponentInChild<RigidbodyComponent>().GetEntity();        return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::BoxColliderComponent:        { auto component_entity = entity.GetComponentInChild<BoxColliderComponent>().GetEntity();      return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
            case ScriptFieldType::SphereColliderComponent:     { auto component_entity = entity.GetComponentInChild<SphereColliderComponent>().GetEntity();   return component_entity ? (uint32_t)component_entity->GetUUID() : NULL_UUID;  break;}
        }
        return NULL_UUID;
    }

    static uint64_t GenerateCacheKey(UUID entity_uuid, ScriptFieldType type)
    {
        uint64_t key = std::hash<uint64_t>{}(entity_uuid);
        key ^= static_cast<uint8_t>(type) + 0x9e3779b9 + (key << 6) + (key >> 2);

        uint64_t quantized_time = static_cast<uint64_t>(Time::Get().GetCurrTime() * 10.0);
        key ^= quantized_time + 0x9e3779b9 + (key << 6) + (key >> 2);

        // Thread safety, ensure thread ID is used to differentiate between threads that may be calling this
        uint64_t thread_id_hash = std::hash<std::thread::id>{}(std::this_thread::get_id());
        key ^= thread_id_hash + 0x9e3779b9 + (key << 6) + (key >> 2);

        return key;
    }

    uint64_t ScriptRegister::Entity_GetComponentsInParentCount(UUID entity_uuid, ScriptFieldType component_type, size_t* count)
    {
        if (!count)
            return -1;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
        {
            *count = 0;
            return -1;
        }

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
        {
            *count = 0;
            return -1;
        }

        uint64_t cache_key = GenerateCacheKey(entity_uuid, component_type);
        
        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                 return CacheAndReturn<IDComponent>(entity, cache_key, count, true);
            case ScriptFieldType::TagComponent:                return CacheAndReturn<TagComponent>(entity, cache_key, count, true);
            case ScriptFieldType::HierarchyComponent:          return CacheAndReturn<HierarchyComponent>(entity, cache_key, count, true);
            case ScriptFieldType::ScriptComponent:             return CacheAndReturn<ScriptComponent>(entity, cache_key, count, true);
            case ScriptFieldType::TransformComponent:          return CacheAndReturn<TransformComponent>(entity, cache_key, count, true);
            case ScriptFieldType::CameraComponent:             return CacheAndReturn<CameraComponent>(entity, cache_key, count, true);
            case ScriptFieldType::AudioListenerComponent:      return CacheAndReturn<AudioListenerComponent>(entity, cache_key, count, true);
            case ScriptFieldType::AudioEmitterComponent:       return CacheAndReturn<AudioEmitterComponent>(entity, cache_key, count, true);
            case ScriptFieldType::MeshFilterComponent:         return CacheAndReturn<MeshFilterComponent>(entity, cache_key, count, true);
            case ScriptFieldType::MeshRendererComponent:       return CacheAndReturn<MeshRendererComponent>(entity, cache_key, count, true);
            case ScriptFieldType::LODMeshComponent:            return CacheAndReturn<LODMeshComponent>(entity, cache_key, count, true);
            case ScriptFieldType::SkinnedMeshComponent:        return CacheAndReturn<SkinnedMeshComponent>(entity, cache_key, count, true);
            case ScriptFieldType::AnimatorComponent:           return CacheAndReturn<AnimatorComponent>(entity, cache_key, count, true);
            case ScriptFieldType::SkyboxComponent:             return CacheAndReturn<SkyboxComponent>(entity, cache_key, count, true);
            case ScriptFieldType::PointLightComponent:         return CacheAndReturn<PointLightComponent>(entity, cache_key, count, true);
            case ScriptFieldType::SpotLightComponent:          return CacheAndReturn<SpotLightComponent>(entity, cache_key, count, true);
            case ScriptFieldType::DirectionalLightComponent:   return CacheAndReturn<DirectionalLightComponent>(entity, cache_key, count, true);
            case ScriptFieldType::RigidbodyComponent:          return CacheAndReturn<RigidbodyComponent>(entity, cache_key, count, true);
            case ScriptFieldType::BoxColliderComponent:        return CacheAndReturn<BoxColliderComponent>(entity, cache_key, count, true);
            case ScriptFieldType::SphereColliderComponent:     return CacheAndReturn<SphereColliderComponent>(entity, cache_key, count, true);
        }

        return -1;
    }

    uint64_t ScriptRegister::Entity_GetComponentsInChildrenCount(UUID entity_uuid, ScriptFieldType component_type, size_t* count)
    {
        if (!count)
            return -1;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return *count = 0, -1;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return *count = 0, -1;

        uint64_t cache_key = GenerateCacheKey(entity_uuid, component_type);

        switch (component_type)
        {
            case ScriptFieldType::IDComponent:                 return CacheAndReturn<IDComponent>(entity, cache_key, count, false);
            case ScriptFieldType::TagComponent:                return CacheAndReturn<TagComponent>(entity, cache_key, count, false);
            case ScriptFieldType::HierarchyComponent:          return CacheAndReturn<HierarchyComponent>(entity, cache_key, count, false);
            case ScriptFieldType::ScriptComponent:             return CacheAndReturn<ScriptComponent>(entity, cache_key, count, false);
            case ScriptFieldType::TransformComponent:          return CacheAndReturn<TransformComponent>(entity, cache_key, count, false);
            case ScriptFieldType::CameraComponent:             return CacheAndReturn<CameraComponent>(entity, cache_key, count, false);
            case ScriptFieldType::AudioListenerComponent:      return CacheAndReturn<AudioListenerComponent>(entity, cache_key, count, false);
            case ScriptFieldType::AudioEmitterComponent:       return CacheAndReturn<AudioEmitterComponent>(entity, cache_key, count, false);
            case ScriptFieldType::MeshFilterComponent:         return CacheAndReturn<MeshFilterComponent>(entity, cache_key, count, false);
            case ScriptFieldType::MeshRendererComponent:       return CacheAndReturn<MeshRendererComponent>(entity, cache_key, count, false);
            case ScriptFieldType::LODMeshComponent:            return CacheAndReturn<LODMeshComponent>(entity, cache_key, count, false);
            case ScriptFieldType::SkinnedMeshComponent:        return CacheAndReturn<SkinnedMeshComponent>(entity, cache_key, count, false);
            case ScriptFieldType::AnimatorComponent:           return CacheAndReturn<AnimatorComponent>(entity, cache_key, count, false);
            case ScriptFieldType::SkyboxComponent:             return CacheAndReturn<SkyboxComponent>(entity, cache_key, count, false);
            case ScriptFieldType::PointLightComponent:         return CacheAndReturn<PointLightComponent>(entity, cache_key, count, false);
            case ScriptFieldType::SpotLightComponent:          return CacheAndReturn<SpotLightComponent>(entity, cache_key, count, false);
            case ScriptFieldType::DirectionalLightComponent:   return CacheAndReturn<DirectionalLightComponent>(entity, cache_key, count, false);
            case ScriptFieldType::RigidbodyComponent:          return CacheAndReturn<RigidbodyComponent>(entity, cache_key, count, false);
            case ScriptFieldType::BoxColliderComponent:        return CacheAndReturn<BoxColliderComponent>(entity, cache_key, count, false);
            case ScriptFieldType::SphereColliderComponent:     return CacheAndReturn<SphereColliderComponent>(entity, cache_key, count, false);
        }

        return -1;
    }

    void ScriptRegister::Entity_GetComponentsInHierarchyCopy(UUID* entity_array, size_t count, uint64_t cached_key)
    {
        if (!entity_array || count == 0 || cached_key == uint64_t(-1))
            return;

        auto it = s_CachedFindHierarchyComponents.find(cached_key);
        if (it == s_CachedFindHierarchyComponents.end())
            return;

        const auto& cached_entities = it->second;
        size_t copy_count = std::min(count, cached_entities.size());

        for (size_t i = 0; i < copy_count; ++i)
            entity_array[i] = cached_entities[i].GetUUID();

        s_CachedFindHierarchyComponents.erase(it);
    }

#pragma endregion

#pragma region Transform Component

    ScriptRegister::_Transform ScriptRegister::TransformComponent_GetTransform(UUID entity_uuid) 
    { 
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return {};

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return {};

        auto& transform = entity.GetTransform();
        return { transform.GetLocalPosition(), transform.GetLocalRotation(), transform.GetLocalScale() };
    }

    void ScriptRegister::TransformComponent_SetTransform(UUID entity_uuid, const _Transform* value) 
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        auto& transform = entity.GetTransform();
        transform.SetPosition(value->position);
        transform.SetRotation(value->rotation);
        transform.SetScale(value->scale);
    }

    glm::vec3 ScriptRegister::TransformComponent_GetPosition(UUID entity_uuid) 
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return {};

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return {};

        return entity.GetTransform().GetLocalPosition();
    }

    void ScriptRegister::TransformComponent_SetPosition(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        entity.GetTransform().SetPosition(*value);
    }

    glm::vec3 ScriptRegister::TransformComponent_GetRotation(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return {};

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return {};

        return entity.GetTransform().GetLocalRotation();
    }

    void ScriptRegister::TransformComponent_SetRotation(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        entity.GetTransform().SetRotation(*value);
    }

    glm::vec3 ScriptRegister::TransformComponent_GetScale(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return { 1.0f, 1.0f, 1.0f };

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return { 1.0f, 1.0f, 1.0f };

        return entity.GetTransform().GetLocalScale();
    }

    void ScriptRegister::TransformComponent_SetScale(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        entity.GetTransform().SetScale(*value);
    }

    glm::vec3 ScriptRegister::TransformComponent_GetFront(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return { 0.0f, 0.0f, -1.0f };

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return { 0.0f, 0.0f, -1.0f };

        return entity.GetTransform().GetForwardDirection();
    }

    void ScriptRegister::TransformComponent_SetFront(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        entity.GetTransform().SetForwardDirection(*value);
    }

    glm::vec3 ScriptRegister::TransformComponent_GetUp(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return { 0.0f, 1.0f, 0.0f };

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return { 0.0f, 1.0f, 0.0f };

        return entity.GetTransform().GetUpDirection();
    }

    glm::vec3 ScriptRegister::TransformComponent_GetRight(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return { 1.0f, 0.0f, 0.0f };

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return { 1.0f, 0.0f, 0.0f };

        return entity.GetTransform().GetRightDirection();
    }

#pragma endregion

#pragma region Camera Component

    uint8_t ScriptRegister::CameraComponent_GetClearFlag(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return 0;

        return static_cast<uint8_t>(entity.GetComponent<CameraComponent>().ClearFlags);
    }

    void ScriptRegister::CameraComponent_SetClearFlag(UUID entity_uuid, CameraClearFlags value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return;

        entity.GetComponent<CameraComponent>().ClearFlags = value;
    }

    glm::vec4 ScriptRegister::CameraComponent_GetClearColour(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec4(1.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return glm::vec4(1.0f);

        return entity.GetComponent<CameraComponent>().ClearColour;
    }

    void ScriptRegister::CameraComponent_SetClearColour(UUID entity_uuid, glm::vec4 value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return;

        entity.GetComponent<CameraComponent>().ClearColour = value;
    }

    uint8_t ScriptRegister::CameraComponent_GetCameraDepth(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return 0;

        return entity.GetComponent<CameraComponent>().CameraDepth;
    }

    void ScriptRegister::CameraComponent_SetCameraDepth(UUID entity_uuid, uint8_t value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return;

        entity.GetComponent<CameraComponent>().CameraDepth = value;
    }

    bool ScriptRegister::CameraComponent_GetDisplayingToViewport(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return false;

        return entity.GetComponent<CameraComponent>().DisplayToMainViewport;
    }

    void ScriptRegister::CameraComponent_SetDisplayingToViewport(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return;

        entity.GetComponent<CameraComponent>().DisplayToMainViewport = value;
    }

    glm::vec4 ScriptRegister::CameraComponent_GetViewport(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

        return entity.GetComponent<CameraComponent>().GetViewport();
    }

    void ScriptRegister::CameraComponent_SetViewport(UUID entity_uuid, const glm::vec4* value)
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<CameraComponent>())
            return;

        auto& config = Project::GetActiveScene()->GetSceneFrameBuffer()->GetConfig();
        entity.GetComponent<CameraComponent>().SetViewport(*value, { config.Width, config.Height});
    }

#pragma endregion

#pragma region Tag Component

    const char* ScriptRegister::TagComponent_GetTag(UUID entity_uuid) 
    { 
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return "";

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return "";

        return entity.GetName().c_str();
    }

    void ScriptRegister::TagComponent_SetTag(UUID entity_uuid, const char* value) 
    { 
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity)
            return;

        entity.GetComponent<TagComponent>().Tag = value;
    }

#pragma endregion

#pragma region Skybox Component

    uint32_t ScriptRegister::SkyboxComponent_GetSkyboxMaterialAssetHandle(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkyboxComponent>())
            return NULL_UUID;

        AssetHandle handle = entity.GetComponent<SkyboxComponent>().SkyboxMaterialAssetHandle;
        if (!AssetManager::IsAssetHandleValid(handle))
            return NULL_UUID;

        return handle;
    }

    void ScriptRegister::SkyboxComponent_SetSkyboxMaterialAssetHandle(UUID entity_uuid, AssetHandle value)
    {
        if (!AssetManager::IsAssetHandleValid(value))
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkyboxComponent>())
            return;

        entity.GetComponent<SkyboxComponent>().SkyboxMaterialAssetHandle = value;
    }

#pragma endregion

#pragma region Point Light

    bool ScriptRegister::PointLightComponent_GetIsActive(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return false;

        return entity.GetComponent<PointLightComponent>().Active;
    }

    void ScriptRegister::PointLightComponent_SetIsActive(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return;

        entity.GetComponent<PointLightComponent>().Active = value;
    }

    glm::vec4 ScriptRegister::PointLightComponent_GetColour(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec4(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return glm::vec4(0.0f);

        return entity.GetComponent<PointLightComponent>().Colour;
    }

    void ScriptRegister::PointLightComponent_SetColour(UUID entity_uuid, const glm::vec4* value)
    {
        if(!value)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return;

        entity.GetComponent<PointLightComponent>().Colour = *value;
    }

    float ScriptRegister::PointLightComponent_GetRadius(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return 0.0f;

        return entity.GetComponent<PointLightComponent>().Radius;
    }

    void ScriptRegister::PointLightComponent_SetRadius(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return;

        entity.GetComponent<PointLightComponent>().Radius = value;
    }

    float ScriptRegister::PointLightComponent_GetIntensity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return 0.0f;

        return entity.GetComponent<PointLightComponent>().Intensity;
    }

    void ScriptRegister::PointLightComponent_SetIntensity(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return;

        entity.GetComponent<PointLightComponent>().Intensity = value;
    }

    uint8_t ScriptRegister::PointLightComponent_GetShadowFlag(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return 0;

        return static_cast<uint8_t>(entity.GetComponent<PointLightComponent>().ShadowFlag);
    }

    void ScriptRegister::PointLightComponent_SetShadowFlag(UUID entity_uuid, ShadowTypeFlag value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<PointLightComponent>())
            return;

        entity.GetComponent<PointLightComponent>().ShadowFlag = value;
    }

#pragma endregion

#pragma region Spot Light

    bool ScriptRegister::SpotLightComponent_GetIsActive(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return false;

        return entity.GetComponent<SpotLightComponent>().Active;
    }

    void ScriptRegister::SpotLightComponent_SetIsActive(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return;

        entity.GetComponent<SpotLightComponent>().Active = value;
    }

    glm::vec4 ScriptRegister::SpotLightComponent_GetColour(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec4(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return glm::vec4(0.0f);

        return entity.GetComponent<SpotLightComponent>().Colour;
    }

    void ScriptRegister::SpotLightComponent_SetColour(UUID entity_uuid, const glm::vec4* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return;

        entity.GetComponent<SpotLightComponent>().Colour = *value;
    }

    float ScriptRegister::SpotLightComponent_GetRange(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return 0.0f;

        return entity.GetComponent<SpotLightComponent>().Range;
    }

    void ScriptRegister::SpotLightComponent_SetRange(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return;

        entity.GetComponent<SpotLightComponent>().Range = value;
    }

    float ScriptRegister::SpotLightComponent_GetAngle(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return 0.0f;

        return entity.GetComponent<SpotLightComponent>().Angle;
    }

    void ScriptRegister::SpotLightComponent_SetAngle(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return;

        entity.GetComponent<SpotLightComponent>().Angle = value;
    }

    float ScriptRegister::SpotLightComponent_GetIntensity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return 0.0f;

        return entity.GetComponent<SpotLightComponent>().Intensity;
    }

    void ScriptRegister::SpotLightComponent_SetIntensity(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return;

        entity.GetComponent<SpotLightComponent>().Intensity = value;
    }

    uint8_t ScriptRegister::SpotLightComponent_GetShadowFlag(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return 0;

        return static_cast<uint8_t>(entity.GetComponent<SpotLightComponent>().ShadowFlag);
    }

    void ScriptRegister::SpotLightComponent_SetShadowFlag(UUID entity_uuid, ShadowTypeFlag value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SpotLightComponent>())
            return;

        entity.GetComponent<SpotLightComponent>().ShadowFlag = value;
    }

#pragma endregion

#pragma region Directional Light

    bool ScriptRegister::DirectionalLightComponent_GetIsActive(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return false;

        return entity.GetComponent<DirectionalLightComponent>().Active;
    }

    void ScriptRegister::DirectionalLightComponent_SetIsActive(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return;

        entity.GetComponent<DirectionalLightComponent>().Active = value;
    }

    glm::vec4 ScriptRegister::DirectionalLightComponent_GetColour(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec4(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return glm::vec4(0.0f);

        return entity.GetComponent<DirectionalLightComponent>().Colour;
    }

    void ScriptRegister::DirectionalLightComponent_SetColour(UUID entity_uuid, const glm::vec4* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return;

        entity.GetComponent<DirectionalLightComponent>().Colour = *value;
    }

    float ScriptRegister::DirectionalLightComponent_GetIntensity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return 0.0f;

        return entity.GetComponent<DirectionalLightComponent>().Intensity;
    }

    void ScriptRegister::DirectionalLightComponent_SetIntensity(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return;

        entity.GetComponent<DirectionalLightComponent>().Intensity = value;
    }

    float ScriptRegister::DirectionalLightComponent_GetMaxShadowDistance(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return 0.0f;

        return entity.GetComponent<DirectionalLightComponent>().MaxShadowVisibleDistance;
    }

    void ScriptRegister::DirectionalLightComponent_SetMaxShadowDistance(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return;

        entity.GetComponent<DirectionalLightComponent>().MaxShadowVisibleDistance = value;
    }

    uint8_t ScriptRegister::DirectionalLightComponent_GetShadowFlag(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return 0;

        return static_cast<uint8_t>(entity.GetComponent<DirectionalLightComponent>().ShadowFlag);
    }

    void ScriptRegister::DirectionalLightComponent_SetShadowFlag(UUID entity_uuid, uint8_t value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<DirectionalLightComponent>())
            return;

        entity.GetComponent<DirectionalLightComponent>().ShadowFlag = static_cast<ShadowTypeFlag>(value);
    }

#pragma endregion

#pragma region Rigidbody

    float ScriptRegister::RigidbodyComponent_GetMass(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return 0.0f;

        return entity.GetComponent<RigidbodyComponent>().GetMass();
    }

    void ScriptRegister::RigidbodyComponent_SetMass(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetMass(value);
    }

    float ScriptRegister::RigidbodyComponent_GetDrag(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return 0.0f;

        return entity.GetComponent<RigidbodyComponent>().GetDrag();
    }

    void ScriptRegister::RigidbodyComponent_SetDrag(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetDrag(value);
    }

    float ScriptRegister::RigidbodyComponent_GetAngularDrag(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return 0.0f;

        return entity.GetComponent<RigidbodyComponent>().GetAngularDrag();
    }

    void ScriptRegister::RigidbodyComponent_SetAngularDrag(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetAngularDrag(value);
    }

    bool ScriptRegister::RigidbodyComponent_GetAutomaticCentreOfMass(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return false;

        return entity.GetComponent<RigidbodyComponent>().IsAutomaticCentreOfMassEnabled();
    }

    void ScriptRegister::RigidbodyComponent_SetAutomaticCentreOfMass(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetAutomaticCentreOfMass(value);
    }

    bool ScriptRegister::RigidbodyComponent_GetUseGravity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return true; // Assume Gravity Always Enabled if Null

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return true; // Assume Gravity Always Enabled if Null

        return entity.GetComponent<RigidbodyComponent>().IsGravityEnabled();
    }

    void ScriptRegister::RigidbodyComponent_SetUseGravity(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetGravity(value);
    }

    bool ScriptRegister::RigidbodyComponent_GetIsKinematic(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return false;

        return entity.GetComponent<RigidbodyComponent>().IsKinematicEnabled();
    }

    void ScriptRegister::RigidbodyComponent_SetIsKinematic(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetKinematic(value);
    }

    glm::bvec3 ScriptRegister::RigidbodyComponent_GetPositionConstraint(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::bvec3(false);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return glm::bvec3(false);

        return entity.GetComponent<RigidbodyComponent>().GetPositionConstraint();
    }

    void ScriptRegister::RigidbodyComponent_SetPositionConstraint(UUID entity_uuid, const glm::bvec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetPositionConstraint(*value);
    }

    glm::bvec3 ScriptRegister::RigidbodyComponent_GetRotationConstraint(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::bvec3(false);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return glm::bvec3(false);

        return entity.GetComponent<RigidbodyComponent>().GetRotationConstraint();
    }

    void ScriptRegister::RigidbodyComponent_SetRotationConstraint(UUID entity_uuid, const glm::bvec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().SetRotationConstraint(*value);
    }

    glm::vec3 ScriptRegister::RigidbodyComponent_GetLinearVelocity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec3(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return glm::vec3(0.0f);

        auto velocity = entity.GetComponent<RigidbodyComponent>().GetActor()->GetLinearVelocity();

        return { velocity.x, velocity.y, velocity.z };
    }

    void ScriptRegister::RigidbodyComponent_SetLinearVelocity(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().GetActor()->SetLinearVelocity(*value);
    }

    glm::vec3 ScriptRegister::RigidbodyComponent_GetAngularVelocity(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec3(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return glm::vec3(0.0f);

        auto velocity = entity.GetComponent<RigidbodyComponent>().GetActor()->GetAngularVelocity();

        return { velocity.x, velocity.y, velocity.z };
    }

    void ScriptRegister::RigidbodyComponent_SetAngularVelocity(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().GetActor()->SetAngularVelocity(*value);
    }


    void ScriptRegister::RigidbodyComponent_ApplyForce(UUID entity_uuid, const glm::vec3* force, uint8_t forceMode)
    {
        if(!force)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().ApplyForce({force->x, force->y, force->z}, static_cast<physx::PxForceMode::Enum>(forceMode));
    }

    void ScriptRegister::RigidbodyComponent_ApplyTorque(UUID entity_uuid, const glm::vec3* torque)
    {
        if(!torque)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<RigidbodyComponent>())
            return;

        entity.GetComponent<RigidbodyComponent>().ApplyTorque(*torque);
    }

#pragma endregion

#pragma region Box Collider

    bool ScriptRegister::BoxColliderComponent_GetIsTrigger(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return false;

        return entity.GetComponent<BoxColliderComponent>().IsTrigger();
    }

    void ScriptRegister::BoxColliderComponent_SetIsTrigger(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return;

        entity.GetComponent<BoxColliderComponent>().SetIsTrigger(value);
    }

    glm::vec3 ScriptRegister::BoxColliderComponent_GetCentre(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec3(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return glm::vec3(0.0f);

        return entity.GetComponent<BoxColliderComponent>().GetCentre();
    }

    void ScriptRegister::BoxColliderComponent_SetCentre(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return;

        entity.GetComponent<BoxColliderComponent>().SetCentre(*value);
    }

    glm::vec3 ScriptRegister::BoxColliderComponent_GetSize(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec3(0.5f); // Default assume size of 0.5f

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return glm::vec3(0.5f); // Default assume size of 0.5f

        return entity.GetComponent<BoxColliderComponent>().GetSize();
    }

    void ScriptRegister::BoxColliderComponent_SetSize(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return;

        entity.GetComponent<BoxColliderComponent>().SetSize(*value);
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::BoxColliderComponent_GetMaterial(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return { 0.6f, 0.6f, 0.0f }; // Default physics material values

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return { 0.6f, 0.6f, 0.0f }; // Default physics material values

        auto phys_material = entity.GetComponent<BoxColliderComponent>().GetMaterial();
        if(!phys_material)
            return { 0.6f, 0.6f, 0.0f }; // Default physics material values

        return { phys_material->GetDynamicFriction(), phys_material->GetStaticFriction(), phys_material->GetBounciness() };
    }

    void ScriptRegister::BoxColliderComponent_SetMaterial(UUID entity_uuid, const _PhysicsMaterial* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<BoxColliderComponent>())
            return;

        auto phys_material = std::make_shared<PhysicsMaterial>(value->m_DynamicFriction, value->m_StaticFriction, value->m_Bounciness);
        entity.GetComponent<BoxColliderComponent>().SetMaterial(phys_material);
    }

#pragma endregion

#pragma region Sphere Collider

    bool ScriptRegister::SphereColliderComponent_GetIsTrigger(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return false;

        return entity.GetComponent<SphereColliderComponent>().IsTrigger();
    }

    void ScriptRegister::SphereColliderComponent_SetIsTrigger(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return;

        entity.GetComponent<SphereColliderComponent>().SetIsTrigger(value);
    }

    glm::vec3 ScriptRegister::SphereColliderComponent_GetCentre(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return glm::vec3(0.0f);

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return glm::vec3(0.0f);

        return entity.GetComponent<SphereColliderComponent>().GetCentre();
    }

    void ScriptRegister::SphereColliderComponent_SetCentre(UUID entity_uuid, const glm::vec3* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return;

        entity.GetComponent<SphereColliderComponent>().SetCentre(*value);
    }

    float ScriptRegister::SphereColliderComponent_GetRadius(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0.5f; // Default assume size of 0.5f

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return 0.5f; // Default assume size of 0.5f

        return entity.GetComponent<SphereColliderComponent>().GetRadius();
    }

    void ScriptRegister::SphereColliderComponent_SetRadius(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return;

        entity.GetComponent<SphereColliderComponent>().SetRadius(value);
    }

    ScriptRegister::_PhysicsMaterial ScriptRegister::SphereColliderComponent_GetMaterial(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return { 0.6f, 0.6f, 0.0f }; // Default physics material values

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return { 0.6f, 0.6f, 0.0f }; // Default physics material values

        auto phys_material = entity.GetComponent<SphereColliderComponent>().GetMaterial();
        if (!phys_material)
            return { 0.6f, 0.6f, 0.0f }; // Default physics material values

        return { phys_material->GetDynamicFriction(), phys_material->GetStaticFriction(), phys_material->GetBounciness() };
    }

    void ScriptRegister::SphereColliderComponent_SetMaterial(UUID entity_uuid, const _PhysicsMaterial* value)
    {
        if(!value)
            return;
            
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SphereColliderComponent>())
            return;

        auto phys_material = std::make_shared<PhysicsMaterial>(value->m_DynamicFriction, value->m_StaticFriction, value->m_Bounciness);
        entity.GetComponent<SphereColliderComponent>().SetMaterial(phys_material);
    }
    
#pragma endregion

#pragma region Animator

    void ScriptRegister::AnimatorComponent_PlayAnimation_Index(UUID entity_uuid, int32_t clip_index, bool should_loop)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().Play(clip_index, should_loop);
    }

    void ScriptRegister::AnimatorComponent_PlayAnimation_Name(UUID entity_uuid, const char *clip_name, bool should_loop)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().Play(clip_name, should_loop);
    }

    void ScriptRegister::AnimatorComponent_PauseAnimation(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().Pause();
    }

    void ScriptRegister::AnimatorComponent_ResumeAnimation(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().Resume();
    }

    void ScriptRegister::AnimatorComponent_StopAnimation(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().Stop();
    }

    bool ScriptRegister::AnimatorComponent_IsPlaying(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return false;

        return entity.GetComponent<AnimatorComponent>().IsPlaying;
    }

    bool ScriptRegister::AnimatorComponent_IsLooping(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return false;

        return entity.GetComponent<AnimatorComponent>().IsLooping;
    }

    void ScriptRegister::AnimatorComponent_SetIsLooping(UUID entity_uuid, bool should_loop)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().IsLooping = should_loop;
    }

    float ScriptRegister::AnimatorComponent_GetPlaybackSpeed(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return false;

        return entity.GetComponent<AnimatorComponent>().PlaybackSpeed;
    }

    void ScriptRegister::AnimatorComponent_SetPlaybackSpeed(UUID entity_uuid, float playback_speed)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;

        entity.GetComponent<AnimatorComponent>().PlaybackSpeed = playback_speed;
    }

    float ScriptRegister::AnimatorComponent_GetCurrentTimestep(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return false;

        return entity.GetComponent<AnimatorComponent>().CurrentTime;
    }

    void ScriptRegister::AnimatorComponent_SetCurrentTimestep(UUID entity_uuid, float normalised_time_step)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return;
        
        auto& animator = entity.GetComponent<AnimatorComponent>();
        auto animation_clip = AssetManager::GetAsset<AnimationClip>(animator.AnimationClipHandles[animator.CurrentClipIndex]);
        if (animation_clip)
        {
            entity.GetComponent<AnimatorComponent>().CurrentTime = glm::clamp(normalised_time_step, 0.0f, 1.0f) * animation_clip->GetDuration();
        }
    }

    uint32_t ScriptRegister::AnimatorComponent_GetCurrentClipIndex(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return NULL_UUID;
        
        return entity.GetComponent<AnimatorComponent>().CurrentClipIndex;
    }

    const char *ScriptRegister::AnimatorComponent_GetCurrentClipName(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return "";

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<AnimatorComponent>())
            return "";
            
        auto& animator_component = entity.GetComponent<AnimatorComponent>();
		if (animator_component.CurrentClipIndex < 0 || 
            animator_component.CurrentClipIndex >= animator_component.AnimationClipHandles.size())
            return "";
        
        const auto& animation_clip_meta_data = Project::GetActiveProject()->GetEditorAssetManager()->GetMetadata(animator_component.AnimationClipHandles[animator_component.CurrentClipIndex]);        
        return animation_clip_meta_data.AssetName.c_str();
    }

#pragma endregion

#pragma region Compute Buffer & Shader

    ComputeBuffer* ScriptRegister::ComputeBuffer_Create(size_t element_count, size_t element_size)
    {
        return new ComputeBuffer(element_count, element_size);
    }

    void ScriptRegister::ComputeBuffer_SetData(ComputeBuffer* buffer, const void* data, size_t element_count, size_t size_of_elements)
    {
        if (!buffer)
            return;

        buffer->SetData(data, element_count, size_of_elements);
    }

    void ScriptRegister::ComputeBuffer_GetData(ComputeBuffer* buffer, void* output, size_t element_count, size_t size_of_elements)
    {
        if (!buffer || !output)
            return;

        buffer->GetData(output, element_count, size_of_elements);
    }

    void ScriptRegister::ComputeBuffer_Release(ComputeBuffer* buffer)
    {
        if (!buffer)
            return;

        buffer->Release();
        delete buffer;  // Delete the buffer object
    }

    void ScriptRegister::ComputeShader_SetBuffer(UUID asset_handle, ComputeBuffer* buffer, uint32_t binding_index)
    {
        if (!buffer)
            return;

        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
            shader->SetBuffer(buffer, binding_index);
    }

    void ScriptRegister::ComputeShader_Dispatch(UUID asset_handle, uint32_t x, uint32_t y, uint32_t z)
    {
        if (x <= 0) x = 1;
        if (y <= 0) y = 1;
        if (z <= 0) z = 1;

        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
            shader->Dispatch(x, y, z);
    }

    void ScriptRegister::ComputeShader_SetBool(UUID asset_handle, const char* name, bool value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetBool(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetInt(UUID asset_handle, const char* name, int32_t value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetInt(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetUInt(UUID asset_handle, const char* name, uint32_t value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetUInt(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetFloat(UUID asset_handle, const char* name, float value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetFloat(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetVector2(UUID asset_handle, const char* name, glm::vec2 value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetFloatVec2(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetVector3(UUID asset_handle, const char* name, glm::vec3 value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetFloatVec3(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetVector4(UUID asset_handle, const char* name, glm::vec4 value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            shader->GetShader()->Bind();
            shader->GetShader()->SetFloatVec4(name, value);
        }
    }

    void ScriptRegister::ComputeShader_SetMat3(UUID asset_handle, const char* name, const float* value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            glm::mat3 mat = glm::make_mat3(value);
            shader->GetShader()->Bind();
            shader->GetShader()->SetMat3(name, mat);
        }
    }

    void ScriptRegister::ComputeShader_SetMat4(UUID asset_handle, const char* name, const float* value)
    {
        if (auto shader = AssetManager::GetAsset<ComputeShaderAsset>(asset_handle); shader)
        {
            glm::mat4 mat = glm::make_mat4(value);
            shader->GetShader()->Bind();
            shader->GetShader()->SetMat4(name, mat);
        }
    }

#pragma endregion

#pragma region Material

    uint32_t ScriptRegister::Material_Create(const char* material_name)
    {
        std::shared_ptr<Material> material = std::make_shared<Material>();
        return AssetManager::AddRuntimeAsset<Material>(material, material_name);
    }

    void ScriptRegister::Material_SetShader(UUID asset_handle, UUID shader_handle)
    {
        AssetManager::GetAsset<Material>(asset_handle)->SetShader(shader_handle);
    }

    uint32_t ScriptRegister::Material_GetShader(UUID asset_handle)
    {
        return AssetManager::GetAsset<Material>(asset_handle)->GetShaderHandle();
    }

    void ScriptRegister::Material_Destroy(UUID asset_handle)
    {
        AssetManager::RemoveRuntimeAsset(asset_handle);
    }

#pragma endregion

#pragma region Mesh Renderer

    bool ScriptRegister::MeshRendererComponent_GetIsActive(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return false;

        return entity.GetComponent<MeshRendererComponent>().Active;
    }

    void ScriptRegister::MeshRendererComponent_SetIsActive(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        entity.GetComponent<MeshRendererComponent>().Active = value;
    }

    bool ScriptRegister::MeshRendererComponent_GetIsCastingShadows(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return 0;

        return entity.GetComponent<MeshRendererComponent>().CastShadows;

    }

    void ScriptRegister::MeshRendererComponent_SetCastingShadows(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        entity.GetComponent<MeshRendererComponent>().CastShadows = value;
    }

    void ScriptRegister::MeshRendererComponent_GetAllMaterialsCount(UUID entity_uuid, size_t* material_count)
    {
        if (!material_count)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
        {
            *material_count = 0;
            return;
        }

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
        {
            *material_count = 0;
            return;
        }

        *material_count = entity.GetComponent<MeshRendererComponent>().MaterialHandles.size();
    }

    void ScriptRegister::MeshRendererComponent_GetAllMaterialsCopy(UUID entity_uuid, uint32_t* material_array, size_t material_count)
    {
        if (!material_array || material_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto& material_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;
        for (size_t i = 0; i < material_count; ++i)
        {
            material_array[i] = material_vector[i].first;
        }
    }

    void ScriptRegister::MeshRendererComponent_SetAllMaterials(UUID entity_uuid, const uint32_t* material_array, size_t material_count)
    {
        if (!material_array || material_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto copied_material_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;
        entity.GetComponent<MeshRendererComponent>().MaterialHandles.clear();
        auto& new_material_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;
        for (size_t i = 0; i < material_count; ++i)
        {
            if (material_array[i] == copied_material_vector[i].first)
                new_material_vector.emplace_back(copied_material_vector[i].first, copied_material_vector[i].second);

            new_material_vector.emplace_back(material_array[i], nullptr);
        }
    }

    uint32_t ScriptRegister::MeshRendererComponent_GetMaterial(UUID entity_uuid, size_t material_index)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return NULL_UUID;

		auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        if(material_index == size_t(-1))
			material_index = material_handle_vector.size() - 1; // We pass -1 when we want to enable last uniform block in material handle element pairs

		if (material_index >= material_handle_vector.size() || material_index < 0)
			return NULL_UUID;

		return material_handle_vector[material_index].first;
    }

    void ScriptRegister::MeshRendererComponent_SetMaterial(UUID entity_uuid, size_t material_index, UUID material_handle)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        if (material_handle_vector.empty() && (material_index == size_t(-1) || material_index == size_t(0)))
        {
            material_handle_vector.push_back({ material_handle, nullptr });
            return;
        }
        else if (material_index == size_t(-1))
        {
            material_index = material_handle_vector.size() - 1;
        }

        if (material_index >= entity.GetComponent<MeshRendererComponent>().MaterialHandles.size())
            return;

        if (material_handle_vector[material_index].first == material_handle) // If Material Handle Already Set
            return;

        if (AssetManager::IsAssetHandleValid(material_handle)) // Check validity of asset
        {
            auto& material_pair = material_handle_vector[material_index];

            material_pair.first = material_handle;

            material_pair.second.reset();   // Clear Material Uniform Block
            material_pair.second = nullptr; // Clear Material Uniform Block
        }

        return;
    }

    void ScriptRegister::MeshRendererComponent_EnableUniformBlock(UUID entity_uuid, uint32_t material_index)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        if (material_index == -1) material_index = (uint32_t)material_handle_vector.size() - 1; // We pass -1 when we want to enable last uniform block in material handle element pairs

        if (material_index >= material_handle_vector.size() || material_index < 0)
            return;

        auto material_asset = AssetManager::GetAsset<Material>(material_handle_vector[material_index].first);

        if (!material_asset)
            return;

        if (!material_handle_vector[material_index].second)
        {
            material_handle_vector[material_index].second = std::make_shared<MaterialUniformBlock>(*material_asset->GetUniformBlock());
            material_handle_vector[material_index].second->GenerateNewBlockID();
        }

        return;
    }

    void ScriptRegister::MeshRendererComponent_DisableUniformBlock(UUID entity_uuid, uint32_t material_index)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        if (material_index == -1) material_index = (uint32_t)material_handle_vector.size() - 1; // We pass -1 when we want to enable last uniform block in material handle element pairs

        if (material_index >= material_handle_vector.size() || material_index < 0)
            return;

        material_handle_vector[material_index].second.reset();
        material_handle_vector[material_index].second = nullptr;
    }

    void ScriptRegister::MeshRendererComponent_EnableAllUniformBlocks(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        for (auto& [material_handle, uniform_block] : material_handle_vector)
        {
            auto material_asset = AssetManager::GetAsset<Material>(material_handle);

            if (!material_asset)
                continue;

            if (!uniform_block)
                uniform_block = std::make_shared<MaterialUniformBlock>(*material_asset->GetUniformBlock());
        }

        return;
    }

    void ScriptRegister::MeshRendererComponent_DisableAllUniformBlocks(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return;

        auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        for (auto& [material_handle, uniform_block] : material_handle_vector)
        {
            uniform_block.reset();
            uniform_block = nullptr;
        }

        return;
    }

    MaterialUniformBlock* ScriptRegister::MeshRendererComponent_GetUniformBlock(UUID entity_uuid, uint32_t material_index)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return nullptr;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshRendererComponent>())
            return nullptr;

        auto& material_handle_vector = entity.GetComponent<MeshRendererComponent>().MaterialHandles;

        if (material_index == -1) material_index = (uint32_t)material_handle_vector.size() - 1; // We pass -1 when we want to enable last uniform block in material handle element pairs

        if (material_index >= material_handle_vector.size() || material_index < 0)
            return nullptr;

        if(!material_handle_vector[material_index].second) 
            return nullptr;

        return material_handle_vector[material_index].second.get();
    }

#pragma endregion

#pragma region Skinned Mesh

    bool ScriptRegister::SkinnedMeshComponent_GetIsActive(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return false;

        return entity.GetComponent<SkinnedMeshComponent>().Active;
    }

    void ScriptRegister::SkinnedMeshComponent_SetIsActive(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        entity.GetComponent<SkinnedMeshComponent>().Active = value;
    }

    bool ScriptRegister::SkinnedMeshComponent_GetIsCastingShadows(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return false;

        return entity.GetComponent<SkinnedMeshComponent>().CastShadows;
    }

    void ScriptRegister::SkinnedMeshComponent_SetCastingShadows(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        entity.GetComponent<SkinnedMeshComponent>().CastShadows = value;
    }

    uint32_t ScriptRegister::SkinnedMeshComponent_GetMeshAssetHandle(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return NULL_UUID;

        AssetHandle handle = entity.GetComponent<SkinnedMeshComponent>().StaticMeshHandle;
        if (!AssetManager::IsAssetHandleValid(handle))
            return NULL_UUID;

        return handle;
    }

    void ScriptRegister::SkinnedMeshComponent_SetMeshAssetHandle(UUID entity_uuid, AssetHandle value)
    {
        if (!AssetManager::IsAssetHandleValid(value))
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        entity.GetComponent<SkinnedMeshComponent>().StaticMeshHandle = value;
    }

    uint32_t ScriptRegister::SkinnedMeshComponent_GetSkeletonAssetHandle(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return NULL_UUID;

        AssetHandle handle = entity.GetComponent<SkinnedMeshComponent>().SkeletonHandle;
        if (!AssetManager::IsAssetHandleValid(handle))
            return NULL_UUID;

        return handle;
    }

    void ScriptRegister::SkinnedMeshComponent_SetSkeletonAssetHandle(UUID entity_uuid, AssetHandle value)
    {
        if (!AssetManager::IsAssetHandleValid(value))
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        entity.GetComponent<SkinnedMeshComponent>().SkeletonHandle = value;
    }

    Bounds_AABB ScriptRegister::SkinnedMeshComponent_GetMeshBounds(uint32_t entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return {};

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return {};

        return entity.GetComponent<SkinnedMeshComponent>().TransformedAABB;
    }

    void ScriptRegister::SkinnedMeshComponent_SetMeshBounds(uint32_t entity_uuid, Bounds_AABB value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        entity.GetComponent<SkinnedMeshComponent>().TransformedAABB = value;
        entity.GetComponent<SkinnedMeshComponent>().AABBNeedsUpdate = true;
        entity.GetComponent<SkinnedMeshComponent>().OctreeNeedsUpdate = true;
    }

    void ScriptRegister::SkinnedMeshComponent_GetAllMaterialsCount(UUID entity_uuid, size_t* material_count)
    {
        if (!material_count)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
        {
            *material_count = 0;
            return;
        }

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
        {
            *material_count = 0;
            return;
        }

        *material_count = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles.size();
    }

    void ScriptRegister::SkinnedMeshComponent_GetAllMaterialsCopy(UUID entity_uuid, uint32_t* material_array, size_t material_count)
    {
        if (!material_array || material_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        auto& material_vector = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;
        for (size_t i = 0; i < material_count; ++i)
        {
            material_array[i] = material_vector[i].first;
        }
    }

    void ScriptRegister::SkinnedMeshComponent_SetAllMaterials(UUID entity_uuid, const uint32_t* material_array, size_t material_count)
    {
        if (!material_array || material_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        auto copied_material_vector = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;
        entity.GetComponent<SkinnedMeshComponent>().MaterialHandles.clear();
        auto& new_material_vector = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;
        for (size_t i = 0; i < material_count; ++i)
        {
            if (material_array[i] == copied_material_vector[i].first)
                new_material_vector.emplace_back(copied_material_vector[i].first, copied_material_vector[i].second);

            new_material_vector.emplace_back(material_array[i], nullptr);
        }
    }

    uint32_t ScriptRegister::SkinnedMeshComponent_GetMaterial(UUID entity_uuid, size_t material_index)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return NULL_UUID;

        auto& material_handle_vector = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;

        if (material_index == size_t(-1))
            material_index = material_handle_vector.size() - 1; // We pass -1 when we want to enable last uniform block in material handle element pairs

        if (material_index >= material_handle_vector.size() || material_index < 0)
            return NULL_UUID;

        return material_handle_vector[material_index].first;
    }

    void ScriptRegister::SkinnedMeshComponent_SetMaterial(UUID entity_uuid, size_t material_index, UUID material_handle)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        auto& material_handle_vector = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;

        if (material_handle_vector.empty() && (material_index == size_t(-1) || material_index == size_t(0)))
        {
            material_handle_vector.push_back({ material_handle, nullptr });
            return;
        }
        else if (material_index == size_t(-1))
        {
            material_index = material_handle_vector.size() - 1;
        }

        if (material_index >= entity.GetComponent<MeshRendererComponent>().MaterialHandles.size())
            return;

        if (material_handle_vector[material_index].first == material_handle) // If Material Handle Already Set
            return;

        if (AssetManager::IsAssetHandleValid(material_handle)) // Check validity of asset
        {
            auto& material_pair = material_handle_vector[material_index];

            material_pair.first = material_handle;

            material_pair.second.reset();   // Clear Material Uniform Block
            material_pair.second = nullptr; // Clear Material Uniform Block
        }

        return;
    }

    void ScriptRegister::SkinnedMeshComponent_GetBoneMapCount(UUID entity_uuid, size_t* bone_count)
    {
        if (!bone_count)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
        {
            *bone_count = 0;
            return;
        }

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
        {
            *bone_count = 0;
            return;
        }

        *bone_count = entity.GetComponent<SkinnedMeshComponent>().SkeletonBoneMapping.size();
    }

    void ScriptRegister::SkinnedMeshComponent_GetBoneMapEntities(UUID entity_uuid, uint32_t* key_array, uint32_t* entity_array, size_t bone_count)
    {
        if (!key_array || !entity_array || bone_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        size_t i = 0;
        auto& bone_map = entity.GetComponent<SkinnedMeshComponent>().SkeletonBoneMapping;
        for (const auto& [bone_id, bone_entity] : bone_map)
        {
            if (i >= bone_count)
                break;

            key_array[i] = bone_id;
            entity_array[i] = bone_entity;

            ++i;
        }
    }

    void ScriptRegister::SkinnedMeshComponent_SetBoneMapEntities(UUID entity_uuid, const uint32_t* key_array, const uint32_t* entity_array, size_t bone_count)
    {
        if (!key_array || !entity_array || bone_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
            return;

        auto& bone_map = entity.GetComponent<SkinnedMeshComponent>().SkeletonBoneMapping;
        bone_map.clear();
        for (size_t i = 0; i < bone_count; ++i)
        {
            bone_map[key_array[i]] = entity_array[i];
        }
    }

#pragma endregion

#pragma region LOD Component

    bool ScriptRegister::LODMeshComponent_GetUseMaxDistanceOverFarPlane(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return false;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return false;

        return entity.GetComponent<LODMeshComponent>().MaxDistanceOverFarPlane;
    }

    void ScriptRegister::LODMeshComponent_SetMaxDistanceOverFarPlane(UUID entity_uuid, bool value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return;

        entity.GetComponent<LODMeshComponent>().MaxDistanceOverFarPlane = value;
    }

    float ScriptRegister::LODMeshComponent_GetMaxDistance(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 500.0f;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return 500.0f;

        return entity.GetComponent<LODMeshComponent>().MaxDistance;
    }

    void ScriptRegister::LODMeshComponent_SetMaxDistance(UUID entity_uuid, float value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return;

        entity.GetComponent<LODMeshComponent>().MaxDistance = value;
    }

    size_t ScriptRegister::LODMeshComponent_GetElementCount(UUID entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return 0;

        return entity.GetComponent<LODMeshComponent>().LOD_Elements.size();
    }

    void ScriptRegister::LODMeshComponent_GetDistances(UUID entity_uuid, float* distance_array, size_t element_count)
    {
        if (!distance_array || element_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return;

        size_t i = 0;
        for (const auto& [distance, element] : entity.GetComponent<LODMeshComponent>().LOD_Elements)
        {
            if (i >= element_count)
                break;

            distance_array[i] = distance;
            ++i;
        }
    }

    size_t ScriptRegister::LODMeshComponent_GetEntityCount(UUID entity_uuid, size_t element_index)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return 0;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return 0;

        if (element_index < 0 || element_index >= entity.GetComponent<LODMeshComponent>().LOD_Elements.size())
            return 0;

        return entity.GetComponent<LODMeshComponent>().LOD_Elements[element_index].MeshRendererEntities.size();
    }

    void ScriptRegister::LODMeshComponent_GetEntityArray(UUID entity_uuid, uint32_t* entity_array, size_t entity_count, size_t element_index)
    {
        if (!entity_array || entity_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return;

        if (element_index < 0 || element_index >= entity.GetComponent<LODMeshComponent>().LOD_Elements.size())
            return;

        size_t i = 0;
        auto& entity_vector = entity.GetComponent<LODMeshComponent>().LOD_Elements[element_index].MeshRendererEntities;
        for (const auto& entity: entity_vector)
        {
            if (i >= entity_count)
                break;

            entity_array[i] = entity;
            ++i;
        }
    }

    void ScriptRegister::LODMeshComponent_SetLODElements(UUID entity_uuid, const float* distance_array, const size_t* entity_counts, const uint32_t* entity_array, size_t element_count)
    {
        if (!distance_array || !entity_counts || !entity_array || element_count <= 0)
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<LODMeshComponent>())
            return;

        auto& lod_element_vector = entity.GetComponent<LODMeshComponent>().LOD_Elements;
        lod_element_vector.clear();
        lod_element_vector.reserve(element_count);

        size_t entity_offset = 0;
        for (size_t i = 0; i < element_count; ++i)
        {
            LODMeshComponent::LODElement lod_element;
            lod_element.DistanceThresholdNormalised = distance_array[i];

            size_t count = entity_counts[i];
            lod_element.MeshRendererEntities.reserve(count);

            for (size_t j = 0; j < count; ++j)
            {
                // Convert uint32_t to UUID � assuming 1:1 mapping for simplicity
                UUID uuid = static_cast<UUID>(entity_array[entity_offset + j]);
                lod_element.MeshRendererEntities.emplace_back(uuid);
            }

            entity_offset += count;
            lod_element_vector.emplace_back(std::move(lod_element));
        }
    }

#pragma endregion

#pragma region Material Uniform Block

    void ScriptRegister::MaterialUniformBlock_SetUniform(MaterialUniformBlock* uniform_block, const char* uniform_name, GLSLType type, void* value)
    {
        if (!uniform_block || !value)
            return;

        switch (type)
        {
            // Bool
            case GLSLType::Bool:		uniform_block->SetUniform(uniform_name, GLSLType::Bool, *static_cast<bool*>(value));            return;
            case GLSLType::BVec2:		uniform_block->SetUniform(uniform_name, GLSLType::BVec2, *static_cast<glm::bvec2*>(value));     return;
            case GLSLType::BVec3:		uniform_block->SetUniform(uniform_name, GLSLType::BVec3, *static_cast<glm::bvec3*>(value));     return;
            case GLSLType::BVec4:		uniform_block->SetUniform(uniform_name, GLSLType::BVec4, *static_cast<glm::bvec4*>(value));     return;

            // Int
            case GLSLType::Int:		uniform_block->SetUniform(uniform_name, GLSLType::Int, *static_cast<int*>(value));              return;
            case GLSLType::IVec2:		uniform_block->SetUniform(uniform_name, GLSLType::IVec2, *static_cast<glm::ivec2*>(value));     return;
            case GLSLType::IVec3:		uniform_block->SetUniform(uniform_name, GLSLType::IVec3, *static_cast<glm::ivec3*>(value));     return;
            case GLSLType::IVec4:		uniform_block->SetUniform(uniform_name, GLSLType::IVec4, *static_cast<glm::ivec4*>(value));     return;

            // UInt
            case GLSLType::Uint:		uniform_block->SetUniform(uniform_name, GLSLType::Uint, *static_cast<unsigned int*>(value));    return;
            case GLSLType::UVec2:		uniform_block->SetUniform(uniform_name, GLSLType::UVec2, *static_cast<glm::uvec2*>(value));     return;
            case GLSLType::UVec3:		uniform_block->SetUniform(uniform_name, GLSLType::UVec3, *static_cast<glm::uvec3*>(value));     return;
            case GLSLType::UVec4:		uniform_block->SetUniform(uniform_name, GLSLType::UVec4, *static_cast<glm::uvec4*>(value));     return;

            // Float
            case GLSLType::Float:		uniform_block->SetUniform(uniform_name, GLSLType::Float, *static_cast<float*>(value));          return;
            case GLSLType::Vec2:		uniform_block->SetUniform(uniform_name, GLSLType::Vec2, *static_cast<glm::vec2*>(value));       return;
            case GLSLType::Vec3:		uniform_block->SetUniform(uniform_name, GLSLType::Vec3, *static_cast<glm::vec3*>(value));       return;
            case GLSLType::Vec4:		uniform_block->SetUniform(uniform_name, GLSLType::Vec4, *static_cast<glm::vec4*>(value));       return;

            // Double
            case GLSLType::Double:	uniform_block->SetUniform(uniform_name, GLSLType::Double, *static_cast<double*>(value));        return;
            case GLSLType::DVec2:		uniform_block->SetUniform(uniform_name, GLSLType::DVec2, *static_cast<glm::dvec2*>(value));     return;
            case GLSLType::DVec3:		uniform_block->SetUniform(uniform_name, GLSLType::DVec3, *static_cast<glm::dvec3*>(value));     return;
            case GLSLType::DVec4:		uniform_block->SetUniform(uniform_name, GLSLType::DVec4, *static_cast<glm::dvec4*>(value));     return;

            // Mat
            case GLSLType::Mat2:		uniform_block->SetUniform(uniform_name, GLSLType::Mat2, *static_cast<glm::mat2*>(value));       return;
            case GLSLType::Mat3:		uniform_block->SetUniform(uniform_name, GLSLType::Mat3, *static_cast<glm::mat3*>(value));       return;
            case GLSLType::Mat4:		uniform_block->SetUniform(uniform_name, GLSLType::Mat4, *static_cast<glm::mat4*>(value));       return;
            
            // Textures
            case GLSLType::Sampler1D:					uniform_block->SetUniform(uniform_name, GLSLType::Sampler1D, *static_cast<AssetHandle*>(value));                return;
            case GLSLType::Sampler1DArray:			uniform_block->SetUniform(uniform_name, GLSLType::Sampler1DArray, *static_cast<AssetHandle*>(value));           return;
            case GLSLType::Sampler1DShadow:			uniform_block->SetUniform(uniform_name, GLSLType::Sampler1DShadow, *static_cast<AssetHandle*>(value));          return;
            case GLSLType::Sampler1DArrayShadow:		uniform_block->SetUniform(uniform_name, GLSLType::Sampler1DArrayShadow, *static_cast<AssetHandle*>(value));     return;

            case GLSLType::Sampler2D:					uniform_block->SetUniform(uniform_name, GLSLType::Sampler2D, *static_cast<AssetHandle*>(value));                return;
            case GLSLType::Sampler2DArray:			uniform_block->SetUniform(uniform_name, GLSLType::Sampler2DArray, *static_cast<AssetHandle*>(value));           return;
            case GLSLType::Sampler2DShadow:			uniform_block->SetUniform(uniform_name, GLSLType::Sampler2DShadow, *static_cast<AssetHandle*>(value));          return;
            case GLSLType::Sampler2DArrayShadow:		uniform_block->SetUniform(uniform_name, GLSLType::Sampler2DArrayShadow, *static_cast<AssetHandle*>(value));     return;

            case GLSLType::Sampler3D:					uniform_block->SetUniform(uniform_name, GLSLType::Sampler3D, *static_cast<AssetHandle*>(value));                return;

            case GLSLType::SamplerCube:				uniform_block->SetUniform(uniform_name, GLSLType::SamplerCube, *static_cast<AssetHandle*>(value));              return;
            case GLSLType::SamplerCubeArray:			uniform_block->SetUniform(uniform_name, GLSLType::SamplerCubeArray, *static_cast<AssetHandle*>(value));         return;
            case GLSLType::SamplerCubeShadow:			uniform_block->SetUniform(uniform_name, GLSLType::SamplerCubeShadow, *static_cast<AssetHandle*>(value));        return;
            case GLSLType::SamplerCubeArrayShadow:	uniform_block->SetUniform(uniform_name, GLSLType::SamplerCubeArrayShadow, *static_cast<AssetHandle*>(value));   return;
        }
    }

    void ScriptRegister::MaterialUniformBlock_OverrideAlbedoMap(MaterialUniformBlock* uniform_block, UUID asset_handle)
    {
        if (!uniform_block)
            return;

        uniform_block->OverrideAlbedoMap(asset_handle);
    }

    void ScriptRegister::MaterialUniformBlock_OverrideMetallicMap(MaterialUniformBlock* uniform_block, UUID asset_handle)
    {
        if (!uniform_block)
            return;

        uniform_block->OverrideMetallicMap(asset_handle);
    }

    void ScriptRegister::MaterialUniformBlock_OverrideNormalMap(MaterialUniformBlock* uniform_block, UUID asset_handle)
    {
        if (!uniform_block)
            return;

        uniform_block->OverrideNormalMap(asset_handle);
    }

    void ScriptRegister::MaterialUniformBlock_OverrideAlbedoTint(MaterialUniformBlock* uniform_block, const glm::vec4* value)
    {
        if (!uniform_block || !value)
            return;

        uniform_block->OverrideAlbedoTint(*value);
    }

    void ScriptRegister::MaterialUniformBlock_OverrideMetallic(MaterialUniformBlock* uniform_block, float value)
    {
        if (!uniform_block)
            return;

        uniform_block->OverrideMetallic(value);
    }

    void ScriptRegister::MaterialUniformBlock_OverrideRoughness(MaterialUniformBlock* uniform_block, float value)
    {
        if (!uniform_block)
            return;

        uniform_block->OverrideRoughness(value);
    }

#pragma endregion

#pragma region Texture

    uint32_t ScriptRegister::Texture2D_Create(int width, int height, uint8_t internal_format)
    {
        std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(width, height, static_cast<Texture2D::TextureFormat>(internal_format), true);
        return AssetManager::AddRuntimeAsset(texture, "Runtime Texture");
    }

    uint32_t ScriptRegister::Texture2D_CreateWithData(unsigned char* data, int width, int height, uint8_t internal_format, uint8_t data_format)
    {
        if (!data) return NULL_UUID;

        std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(data, width, height, static_cast<Texture2D::TextureFormat>(internal_format), static_cast<Texture2D::TextureFormat>(data_format), true);
        return AssetManager::AddRuntimeAsset(texture, "Runtime Texture");
    }

    void ScriptRegister::Texture2D_SetPixel(uint32_t asset_handle, const glm::vec4* colour, const glm::ivec2* pixel_coord)
    {
        if(!colour || !pixel_coord)
            return;
            
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& texture_ref = AssetManager::GetAsset<Texture2D>(asset_handle);
        if (!texture_ref)
            return;

        texture_ref->SetPixel(*colour, *pixel_coord);
    }

    void ScriptRegister::Texture2D_SetPixelData(uint32_t asset_handle, unsigned char* pixel_data, size_t pixel_data_size, uint8_t pixel_data_format)
    {
        if (!pixel_data || pixel_data_size == 0)
            return;

        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& texture_ref = AssetManager::GetAsset<Texture2D>(asset_handle);
        if (!texture_ref)
            return;

        texture_ref->SetPixelData(pixel_data, pixel_data_size, static_cast<Texture2D::TextureFormat>(pixel_data_format));
    }

    void ScriptRegister::Texture2D_SubmitTextureChanges(uint32_t asset_handle)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& texture_ref = AssetManager::GetAsset<Texture2D>(asset_handle);
        if (!texture_ref)
            return;

        texture_ref->SubmitTextureChanges();
    }

    void ScriptRegister::Texture2D_Destroy(AssetHandle handle)
    {
        AssetManager::RemoveRuntimeAsset(handle);
    }

#pragma endregion

#pragma region Mesh

    uint32_t ScriptRegister::Mesh_CreateNewMesh()
    {
        std::shared_ptr<SubMesh> sub_mesh = std::make_shared<SubMesh>();

        sub_mesh->SetVAO(std::make_unique<VertexArray>());

        VertexBuffer* vbo_verts = new VertexBuffer(3);
        vbo_verts->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aPos"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_verts);

        VertexBuffer* vbo_norms = new VertexBuffer(3);
        vbo_norms->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aNormal"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_norms);

        VertexBuffer* vbo_texcoords = new VertexBuffer(2);
        vbo_texcoords->SetLayout(BufferLayout{ {ShaderDataType::Float2, "aTexCoord"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_texcoords);

        VertexBuffer* vbo_tangents = new VertexBuffer(3);
        vbo_tangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aTangent"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_tangents);

        VertexBuffer* vbo_bitangents = new VertexBuffer(3);
        vbo_bitangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aBitangent"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_bitangents);

        IndexBuffer* ebo = new IndexBuffer();
        sub_mesh->GetVAO()->SetIndexBuffer(ebo);

        std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>();

        mesh->SubMeshes.push_back(std::move(sub_mesh));

        return AssetManager::AddRuntimeAsset(mesh, "New Runtime Mesh");
    }

    void ScriptRegister::Mesh_Destroy(AssetHandle handle)
    {
        AssetManager::RemoveRuntimeAsset(handle);
    }

    void ScriptRegister::Mesh_SubmitChanges(UUID asset_handle, bool clearCPUData)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SubmitChangesToGPU(clearCPUData);
    }

    void ScriptRegister::Mesh_CopyBufferDataToCPU(UUID asset_handle)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->CopyGPUData();
    }

    void ScriptRegister::Mesh_ClearBufferDataFromCPU(UUID asset_handle)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->ClearCPUData();
    }

    void ScriptRegister::Mesh_SetVertices(UUID asset_handle, const float* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);
        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetVertices(data, data_size);

        for (uint32_t i = 0; i < data_size; i += 3) // Assuming (x, y, z) ordering!!
        {
            glm::vec3 v(data[i], data[i + 1], data[i + 2]);
            mesh->MeshBounds.BoundsMin = glm::min(mesh->MeshBounds.BoundsMin, v);
            mesh->MeshBounds.BoundsMax = glm::max(mesh->MeshBounds.BoundsMax, v);
        }

        mesh->ModifiedAABB = true;
    }

    void ScriptRegister::Mesh_SetNormals(UUID asset_handle, const float* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetNormals(data, data_size);
    }

    void ScriptRegister::Mesh_SetTextureCoords(UUID asset_handle, const float* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetTextureCoords(data, data_size);
    }

    void ScriptRegister::Mesh_SetTangents(UUID asset_handle, const float* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetTangents(data, data_size);
    }

    void ScriptRegister::Mesh_SetBitangents(UUID asset_handle, const float* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetBiTangents(data, data_size);
    }

    void ScriptRegister::Mesh_SetBoneIDs(UUID asset_handle, const glm::ivec4* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetBoneIDs(data, data_size);
    }

    void ScriptRegister::Mesh_SetBoneWeights(UUID asset_handle, const glm::vec4* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetBoneWeights(data, data_size);
    }

    void ScriptRegister::Mesh_GetVertices(UUID asset_handle, const float** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetVertices(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_GetNormals(UUID asset_handle, const float** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetNormals(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_GetTextureCoords(UUID asset_handle, const float** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetTextureCoords(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_GetTangents(UUID asset_handle, const float** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetTangents(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_GetBitangents(UUID asset_handle, const float** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetBiTangents(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_GetBoneIDs(UUID asset_handle, const glm::ivec4** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetBoneIDs(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_GetBoneWeights(UUID asset_handle, const glm::vec4** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetBoneWeights(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    uint32_t ScriptRegister::MeshFilterComponent_GetMeshAssetHandle(uint32_t entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshFilterComponent>())
            return NULL_UUID;

        AssetHandle handle = entity.GetComponent<MeshFilterComponent>().StaticMeshHandle;
        if (!AssetManager::IsAssetHandleValid(handle))
            return NULL_UUID;

        return handle;
    }

    void ScriptRegister::MeshFilterComponent_SetMeshAssetHandle(uint32_t entity_uuid, uint32_t asset_handle)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshFilterComponent>())
            return;

        entity.GetComponent<MeshFilterComponent>().StaticMeshHandle = asset_handle;
    }

    Bounds_AABB ScriptRegister::MeshFilterComponent_GetMeshBounds(uint32_t entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return {};

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshFilterComponent>())
            return {};

        return entity.GetComponent<MeshFilterComponent>().TransformedAABB;
    }

    void ScriptRegister::MeshFilterComponent_SetMeshBounds(uint32_t entity_uuid, Bounds_AABB value)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshFilterComponent>())
            return;

        entity.GetComponent<MeshFilterComponent>().TransformedAABB = value;
        entity.GetComponent<MeshFilterComponent>().AABBNeedsUpdate = true;
        entity.GetComponent<MeshFilterComponent>().OctreeNeedsUpdate = true;
    }

    uint32_t ScriptRegister::MeshFilterComponent_CopyMesh(uint32_t entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return NULL_UUID;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<MeshFilterComponent>())
            return NULL_UUID;

        auto asset_mesh = AssetManager::GetAsset<StaticMesh>(entity.GetComponent<MeshFilterComponent>().StaticMeshHandle);

        if (asset_mesh && asset_mesh->SubMeshes.size() >= 1 && asset_mesh->SubMeshes.front())
        {
            auto copy_asset_mesh = std::make_shared<StaticMesh>();
            copy_asset_mesh->SubMeshes.push_back(std::make_shared<SubMesh>(*asset_mesh->SubMeshes.front()));
            copy_asset_mesh->MeshBounds = asset_mesh->MeshBounds;

            return AssetManager::AddRuntimeAsset<StaticMesh>(copy_asset_mesh, "New Runtime Mesh");
        }

        std::shared_ptr<SubMesh> sub_mesh = std::make_shared<SubMesh>();

        sub_mesh->SetVAO(std::make_unique<VertexArray>());

        VertexBuffer* vbo_verts = new VertexBuffer(3);
        vbo_verts->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aPos"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_verts);

        VertexBuffer* vbo_norms = new VertexBuffer(3);
        vbo_norms->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aNormal"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_norms);

        VertexBuffer* vbo_texcoords = new VertexBuffer(2);
        vbo_texcoords->SetLayout(BufferLayout{ {ShaderDataType::Float2, "aTexCoord"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_texcoords);

        VertexBuffer* vbo_tangents = new VertexBuffer(3);
        vbo_tangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aTangent"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_tangents);

        VertexBuffer* vbo_bitangents = new VertexBuffer(3);
        vbo_bitangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aBitangent"} });
        sub_mesh->GetVAO()->AddVertexBuffer(vbo_bitangents);

        IndexBuffer* ebo = new IndexBuffer();
        sub_mesh->GetVAO()->SetIndexBuffer(ebo);

        asset_mesh = std::make_shared<StaticMesh>();

        asset_mesh->SubMeshes.push_back(std::move(sub_mesh));

        AssetHandle handle = AssetManager::AddRuntimeAsset(asset_mesh, "New Runtime Mesh");
        entity.GetComponent<MeshFilterComponent>().StaticMeshHandle = handle;

        return handle;
    }

    void ScriptRegister::Mesh_SetTriangles(UUID asset_handle, const uint32_t* data, size_t data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle) || !data || data_size <= 0)
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->SetTriangles(data, data_size);
    }

    void ScriptRegister::Mesh_GetTriangles(UUID asset_handle, const uint32_t** data, size_t* data_size)
    {
        if (!AssetManager::IsAssetHandleValid(asset_handle))
            return;

        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        size_t temp_count = 0;
        *data = mesh->SubMeshes.front()->GetTriangles(&temp_count);
        *data_size = static_cast<int>(temp_count);
    }

    void ScriptRegister::Mesh_RecalculateNormals(UUID asset_handle)
    {
        const auto& mesh = AssetManager::GetAsset<StaticMesh>(asset_handle);

        if (!mesh || mesh->SubMeshes.empty() || !mesh->SubMeshes.front()) return;

        mesh->SubMeshes.front()->RecalculateNormals();
    }

#pragma endregion



}
