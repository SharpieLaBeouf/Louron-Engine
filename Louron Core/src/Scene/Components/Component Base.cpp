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
	T& ComponentBase::GetComponent() { return (m_Entity) ? m_Entity->GetComponent<T>() : Entity::GetBlankComponent<T>(); }

	template<typename T>
	T& ComponentBase::GetComponentInParent() { return (m_Entity) ? m_Entity->GetComponentInParent<T>() : Entity::GetBlankComponent<T>(); }

	template<typename T>
	T& ComponentBase::GetComponentInChild() { return (m_Entity) ? m_Entity->GetComponentInChild<T>() : Entity::GetBlankComponent<T>(); }
	
	template<typename T>
	std::vector<Entity> ComponentBase::GetComponentsInParents() { return (m_Entity) ? m_Entity->GetComponentsInParents<T>() : std::vector<Entity>{}; }

	template<typename T>
	std::vector<Entity> ComponentBase::GetComponentsInChildren() { return (m_Entity) ? m_Entity->GetComponentsInChildren<T>() : std::vector<Entity>{}; }

	// Specialisations for GetComponent
	template ComponentBase&					ComponentBase::GetComponent<ComponentBase>();
	template IDComponent&					ComponentBase::GetComponent<IDComponent>();
	template TagComponent&					ComponentBase::GetComponent<TagComponent>();
	template HierarchyComponent&			ComponentBase::GetComponent<HierarchyComponent>();
	template ScriptComponent&				ComponentBase::GetComponent<ScriptComponent>();
	template TransformComponent&			ComponentBase::GetComponent<TransformComponent>();
	template CameraComponent&				ComponentBase::GetComponent<CameraComponent>();
	template AudioListener&					ComponentBase::GetComponent<AudioListener>();
	template AudioEmitter&					ComponentBase::GetComponent<AudioEmitter>();
	template MeshFilterComponent&			ComponentBase::GetComponent<MeshFilterComponent>();
	template MeshRendererComponent&			ComponentBase::GetComponent<MeshRendererComponent>();
	template LODMeshComponent&				ComponentBase::GetComponent<LODMeshComponent>();
	template SkyboxComponent&				ComponentBase::GetComponent<SkyboxComponent>();
	template PointLightComponent&			ComponentBase::GetComponent<PointLightComponent>();
	template SpotLightComponent&			ComponentBase::GetComponent<SpotLightComponent>();
	template DirectionalLightComponent&		ComponentBase::GetComponent<DirectionalLightComponent>();
	template RigidbodyComponent&			ComponentBase::GetComponent<RigidbodyComponent>();
	template BoxColliderComponent&			ComponentBase::GetComponent<BoxColliderComponent>();
	template SphereColliderComponent&		ComponentBase::GetComponent<SphereColliderComponent>();
	
	// Specialisations for GetComponentInParent
	template ComponentBase&					ComponentBase::GetComponentInParent<ComponentBase>();
	template IDComponent&					ComponentBase::GetComponentInParent<IDComponent>();
	template TagComponent&					ComponentBase::GetComponentInParent<TagComponent>();
	template HierarchyComponent&			ComponentBase::GetComponentInParent<HierarchyComponent>();
	template ScriptComponent&				ComponentBase::GetComponentInParent<ScriptComponent>();
	template TransformComponent&			ComponentBase::GetComponentInParent<TransformComponent>();
	template CameraComponent&				ComponentBase::GetComponentInParent<CameraComponent>();
	template AudioListener&					ComponentBase::GetComponentInParent<AudioListener>();
	template AudioEmitter&					ComponentBase::GetComponentInParent<AudioEmitter>();
	template MeshFilterComponent&			ComponentBase::GetComponentInParent<MeshFilterComponent>();
	template MeshRendererComponent&			ComponentBase::GetComponentInParent<MeshRendererComponent>();
	template LODMeshComponent&				ComponentBase::GetComponentInParent<LODMeshComponent>();
	template SkyboxComponent&				ComponentBase::GetComponentInParent<SkyboxComponent>();
	template PointLightComponent&			ComponentBase::GetComponentInParent<PointLightComponent>();
	template SpotLightComponent&			ComponentBase::GetComponentInParent<SpotLightComponent>();
	template DirectionalLightComponent&		ComponentBase::GetComponentInParent<DirectionalLightComponent>();
	template RigidbodyComponent&			ComponentBase::GetComponentInParent<RigidbodyComponent>();
	template BoxColliderComponent&			ComponentBase::GetComponentInParent<BoxColliderComponent>();
	template SphereColliderComponent&		ComponentBase::GetComponentInParent<SphereColliderComponent>();
	
	// Specialisations for GetComponentInChild
	template ComponentBase&					ComponentBase::GetComponentInChild<ComponentBase>();
	template IDComponent&					ComponentBase::GetComponentInChild<IDComponent>();
	template TagComponent&					ComponentBase::GetComponentInChild<TagComponent>();
	template HierarchyComponent&			ComponentBase::GetComponentInChild<HierarchyComponent>();
	template ScriptComponent&				ComponentBase::GetComponentInChild<ScriptComponent>();
	template TransformComponent&			ComponentBase::GetComponentInChild<TransformComponent>();
	template CameraComponent&				ComponentBase::GetComponentInChild<CameraComponent>();
	template AudioListener&					ComponentBase::GetComponentInChild<AudioListener>();
	template AudioEmitter&					ComponentBase::GetComponentInChild<AudioEmitter>();
	template MeshFilterComponent&			ComponentBase::GetComponentInChild<MeshFilterComponent>();
	template MeshRendererComponent&			ComponentBase::GetComponentInChild<MeshRendererComponent>();
	template LODMeshComponent&				ComponentBase::GetComponentInChild<LODMeshComponent>();
	template SkyboxComponent&				ComponentBase::GetComponentInChild<SkyboxComponent>();
	template PointLightComponent&			ComponentBase::GetComponentInChild<PointLightComponent>();
	template SpotLightComponent&			ComponentBase::GetComponentInChild<SpotLightComponent>();
	template DirectionalLightComponent&		ComponentBase::GetComponentInChild<DirectionalLightComponent>();
	template RigidbodyComponent&			ComponentBase::GetComponentInChild<RigidbodyComponent>();
	template BoxColliderComponent&			ComponentBase::GetComponentInChild<BoxColliderComponent>();
	template SphereColliderComponent&		ComponentBase::GetComponentInChild<SphereColliderComponent>();
	
	// Specialisations for GetComponentsInParents
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<ComponentBase>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<IDComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<TagComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<HierarchyComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<ScriptComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<TransformComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<CameraComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<AudioListener>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<AudioEmitter>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<MeshFilterComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<MeshRendererComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<LODMeshComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SkyboxComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<PointLightComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SpotLightComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<DirectionalLightComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<RigidbodyComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<BoxColliderComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInParents<SphereColliderComponent>();

	// Specialisations for GetComponentsInChildren
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<ComponentBase>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<IDComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<TagComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<HierarchyComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<ScriptComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<TransformComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<CameraComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<AudioListener>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<AudioEmitter>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<MeshFilterComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<MeshRendererComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<LODMeshComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SkyboxComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<PointLightComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SpotLightComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<DirectionalLightComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<RigidbodyComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<BoxColliderComponent>();
	template std::vector<Entity>			ComponentBase::GetComponentsInChildren<SphereColliderComponent>();
}
