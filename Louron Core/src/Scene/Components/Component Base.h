#pragma once

// Louron Core Headers
#include "../../Core/UUID.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>

// External Vendor Library Headers

namespace YAML 
{
    class Emitter;
    class Node;
}

namespace Louron 
{

    class Entity;

    struct ComponentBase 
    {

    public:

        virtual ~ComponentBase() = default;

        std::shared_ptr<Entity> GetEntity() const;
        void SetEntity(const Entity& entity);

        template<typename T>
        T& GetComponent() const;

        template<typename T>
        T& GetComponentInParent() const;

        template<typename T>
        T& GetComponentInChild() const;

        template<typename T>
        std::vector<Entity> GetComponentsInParents() const;

        template<typename T>
        std::vector<Entity> GetComponentsInChildren() const;

    private:

        // Smart Shared Pointer to Auto-Destruct
        // Can't Use Unique_Ptr as we need the full definition of Entity for that
        std::shared_ptr<Entity> m_Entity = nullptr; 

    };

    #pragma region Component Group

    template<typename... Component>
    struct ComponentGroup {};

    struct IDComponent;
    struct TagComponent;
    struct HierarchyComponent;
    struct ScriptComponent;
    struct TransformComponent;
    
    struct CameraComponent;
     
    struct AudioListener;
    struct AudioEmitter;
     
    struct MeshFilterComponent;
    struct MeshRendererComponent;

    struct SkinnedMeshComponent;
    struct AnimatorComponent;
     
    struct LODMeshComponent;
     
    struct SkyboxComponent;
    struct PointLightComponent;
    struct SpotLightComponent;
    struct DirectionalLightComponent;
     
    struct RigidbodyComponent;
    struct BoxColliderComponent;
    struct SphereColliderComponent;

    using AllComponents = ComponentGroup <

        ComponentBase,

        IDComponent,
        TagComponent,
        HierarchyComponent,
        ScriptComponent,
        TransformComponent,

        CameraComponent,

        AudioListener,
        AudioEmitter,

        MeshFilterComponent,
        MeshRendererComponent,

        SkinnedMeshComponent,
        AnimatorComponent,

        LODMeshComponent,

        SkyboxComponent,
        PointLightComponent,
        SpotLightComponent,
        DirectionalLightComponent,

        RigidbodyComponent,
        BoxColliderComponent,
        SphereColliderComponent

    > ;

    #pragma endregion

}