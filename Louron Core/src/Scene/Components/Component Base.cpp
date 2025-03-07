#include "Component Base.h"

// Louron Core Headers
#include "../Entity.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron
{

	std::shared_ptr<Entity> ComponentBase::GetEntity() const { return m_Entity; }
	void ComponentBase::SetEntity(const Entity& entity) { m_Entity = std::make_shared<Entity>(entity); }

	template<typename T>
	T& ComponentBase::GetComponent() const { return (m_Entity) ? m_Entity->GetComponent<T>() : Entity::GetBlankComponent<T>(); }

	template<typename T>
	T& ComponentBase::GetComponentInParent() const { return (m_Entity) ? m_Entity->GetComponentInParent<T>() : Entity::GetBlankComponent<T>(); }

	template<typename T>
	T& ComponentBase::GetComponentInChild() const { return (m_Entity) ? m_Entity->GetComponentInChild<T>() : Entity::GetBlankComponent<T>(); }
	
	template<typename T>
	std::vector<Entity> ComponentBase::GetComponentsInParents() const { return (m_Entity) ? m_Entity->GetComponentsInParents<T>() : std::vector<Entity>{}; }

	template<typename T>
	std::vector<Entity> ComponentBase::GetComponentsInChildren() const { return (m_Entity) ? m_Entity->GetComponentsInChildren<T>() : std::vector<Entity>{}; }

	// Specialisations for GetComponent
	template ComponentBase&					ComponentBase::GetComponent<ComponentBase>() const;
	template IDComponent&					ComponentBase::GetComponent<IDComponent>() const;
	template TagComponent&					ComponentBase::GetComponent<TagComponent>() const;
	template HierarchyComponent&			ComponentBase::GetComponent<HierarchyComponent>() const;
	template ScriptComponent&				ComponentBase::GetComponent<ScriptComponent>() const;
	template TransformComponent&			ComponentBase::GetComponent<TransformComponent>() const;
	template CameraComponent&				ComponentBase::GetComponent<CameraComponent>() const;
	template AudioListener&					ComponentBase::GetComponent<AudioListener>() const;
	template AudioEmitter&					ComponentBase::GetComponent<AudioEmitter>() const;
	template MeshFilterComponent&			ComponentBase::GetComponent<MeshFilterComponent>() const;
	template MeshRendererComponent&			ComponentBase::GetComponent<MeshRendererComponent>() const;
	template SkinnedMeshComponent&			ComponentBase::GetComponent<SkinnedMeshComponent>() const;
	template AnimatorComponent&				ComponentBase::GetComponent<AnimatorComponent>() const;
	template LODMeshComponent&				ComponentBase::GetComponent<LODMeshComponent>() const;
	template SkyboxComponent&				ComponentBase::GetComponent<SkyboxComponent>() const;
	template PointLightComponent&			ComponentBase::GetComponent<PointLightComponent>() const;
	template SpotLightComponent&			ComponentBase::GetComponent<SpotLightComponent>() const;
	template DirectionalLightComponent&		ComponentBase::GetComponent<DirectionalLightComponent>() const;
	template RigidbodyComponent&			ComponentBase::GetComponent<RigidbodyComponent>() const;
	template BoxColliderComponent&			ComponentBase::GetComponent<BoxColliderComponent>() const;
	template SphereColliderComponent&		ComponentBase::GetComponent<SphereColliderComponent>() const;
	
	// Specialisations for GetComponentInParent
	template ComponentBase&					ComponentBase::GetComponentInParent<ComponentBase>() const;
	template IDComponent&					ComponentBase::GetComponentInParent<IDComponent>() const;
	template TagComponent&					ComponentBase::GetComponentInParent<TagComponent>() const;
	template HierarchyComponent&			ComponentBase::GetComponentInParent<HierarchyComponent>() const;
	template ScriptComponent&				ComponentBase::GetComponentInParent<ScriptComponent>() const;
	template TransformComponent&			ComponentBase::GetComponentInParent<TransformComponent>() const;
	template CameraComponent&				ComponentBase::GetComponentInParent<CameraComponent>() const;
	template AudioListener&					ComponentBase::GetComponentInParent<AudioListener>() const;
	template AudioEmitter&					ComponentBase::GetComponentInParent<AudioEmitter>() const;
	template MeshFilterComponent&			ComponentBase::GetComponentInParent<MeshFilterComponent>() const;
	template MeshRendererComponent&			ComponentBase::GetComponentInParent<MeshRendererComponent>() const;
	template SkinnedMeshComponent&			ComponentBase::GetComponentInParent<SkinnedMeshComponent>() const;
	template AnimatorComponent&				ComponentBase::GetComponentInParent<AnimatorComponent>() const;
	template LODMeshComponent&				ComponentBase::GetComponentInParent<LODMeshComponent>() const;
	template SkyboxComponent&				ComponentBase::GetComponentInParent<SkyboxComponent>() const;
	template PointLightComponent&			ComponentBase::GetComponentInParent<PointLightComponent>() const;
	template SpotLightComponent&			ComponentBase::GetComponentInParent<SpotLightComponent>() const;
	template DirectionalLightComponent&		ComponentBase::GetComponentInParent<DirectionalLightComponent>() const;
	template RigidbodyComponent&			ComponentBase::GetComponentInParent<RigidbodyComponent>() const;
	template BoxColliderComponent&			ComponentBase::GetComponentInParent<BoxColliderComponent>() const;
	template SphereColliderComponent&		ComponentBase::GetComponentInParent<SphereColliderComponent>() const;
	
	// Specialisations for GetComponentInChild
	template ComponentBase&					ComponentBase::GetComponentInChild<ComponentBase>() const;
	template IDComponent&					ComponentBase::GetComponentInChild<IDComponent>() const;
	template TagComponent&					ComponentBase::GetComponentInChild<TagComponent>() const;
	template HierarchyComponent&			ComponentBase::GetComponentInChild<HierarchyComponent>() const;
	template ScriptComponent&				ComponentBase::GetComponentInChild<ScriptComponent>() const;
	template TransformComponent&			ComponentBase::GetComponentInChild<TransformComponent>() const;
	template CameraComponent&				ComponentBase::GetComponentInChild<CameraComponent>() const;
	template AudioListener&					ComponentBase::GetComponentInChild<AudioListener>() const;
	template AudioEmitter&					ComponentBase::GetComponentInChild<AudioEmitter>() const;
	template MeshFilterComponent&			ComponentBase::GetComponentInChild<MeshFilterComponent>() const;
	template MeshRendererComponent&			ComponentBase::GetComponentInChild<MeshRendererComponent>() const;
	template SkinnedMeshComponent&			ComponentBase::GetComponentInChild<SkinnedMeshComponent>() const;
	template AnimatorComponent&				ComponentBase::GetComponentInChild<AnimatorComponent>() const;
	template LODMeshComponent&				ComponentBase::GetComponentInChild<LODMeshComponent>() const;
	template SkyboxComponent&				ComponentBase::GetComponentInChild<SkyboxComponent>() const;
	template PointLightComponent&			ComponentBase::GetComponentInChild<PointLightComponent>() const;
	template SpotLightComponent&			ComponentBase::GetComponentInChild<SpotLightComponent>() const;
	template DirectionalLightComponent&		ComponentBase::GetComponentInChild<DirectionalLightComponent>() const;
	template RigidbodyComponent&			ComponentBase::GetComponentInChild<RigidbodyComponent>() const;
	template BoxColliderComponent&			ComponentBase::GetComponentInChild<BoxColliderComponent>() const;
	template SphereColliderComponent&		ComponentBase::GetComponentInChild<SphereColliderComponent>() const;
	
	// Specialisations for GetComponentsInParents
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<ComponentBase>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<IDComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<TagComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<HierarchyComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<ScriptComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<TransformComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<CameraComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<AudioListener>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<AudioEmitter>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<MeshFilterComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<MeshRendererComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SkinnedMeshComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<AnimatorComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<LODMeshComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SkyboxComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<PointLightComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SpotLightComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<DirectionalLightComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<RigidbodyComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<BoxColliderComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SphereColliderComponent>() const;

	// Specialisations for GetComponentsInChildren
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<ComponentBase>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<IDComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<TagComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<HierarchyComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<ScriptComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<TransformComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<CameraComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<AudioListener>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<AudioEmitter>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<MeshFilterComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<MeshRendererComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SkinnedMeshComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<AnimatorComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<LODMeshComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SkyboxComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<PointLightComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SpotLightComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<DirectionalLightComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<RigidbodyComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<BoxColliderComponent>() const;
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SphereColliderComponent>() const;
}
