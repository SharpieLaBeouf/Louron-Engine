#pragma once

// Louron Core Headers
#include "Scene.h"

#include "../Core/UUID.h"
#include "../Asset/Asset Manager API.h"
#include "../Scripting/Script Manager.h"
#include "../OpenGL/Mesh.h"

#include "Components/Audio Components.h"
#include "Components/Camera Component.h"
#include "Components/Core Components.h"
#include "Components/Light Components.h"
#include "Components/Mesh Components.h"
#include "Components/Script Component.h"
#include "Components/Skybox Component.h"
#include "Components/Physics/Collider Components.h"
#include "Components/Physics/Rigidbody Component.h"
#include "Components/Animator Component.h"
#include "Components/SkinnedMeshComponent.h"

#include "Scene Systems/Physics System.h"

// C++ Standard Library Headers
#include <iostream>
#include <typeindex>
#include <memory>
#include <utility>

// External Vendor Library Headers
#include <entt/entt.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Louron {

	//Base Entity Class
	class Entity {

	public:

		Entity() = default;
		Entity(entt::entity regHandle, Scene* scene);

		Entity(const Entity&) = default;
		Entity(Entity&&) = default;

		Entity& operator=(const Entity&) = default;
		Entity& operator=(Entity&&) = default;

#pragma region Setters

		// This adds a Component to the applicable Entity, and returns that Component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {

			if (HasComponent<T>()) {
				L_CORE_WARN("Entity Already Has: {0}", typeid(T).name());
				return GetComponent<T>();
			}

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			component.SetEntity(*this);

			if constexpr (std::is_same_v<T, ScriptComponent>) {
				if(m_Scene && m_Scene->IsRunning())
					InitAllEntityScripts();
			}

			if constexpr (std::is_same_v<T, RigidbodyComponent>) {
				if ((m_Scene->IsRunning() || m_Scene->IsSimulating()) && m_Scene->GetPhysScene())
					component.Init(&GetTransform(), m_Scene->GetPhysScene());

				if (HasComponent<BoxColliderComponent>())
				{
					GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
					GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_ShapePropsUpdated);
					GetComponent<BoxColliderComponent>().AddFlag(ColliderFlag_TransformUpdated);
				}

				if (HasComponent<SphereColliderComponent>())
				{
					GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_RigidbodyUpdated);
					GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_ShapePropsUpdated);
					GetComponent<SphereColliderComponent>().AddFlag(ColliderFlag_TransformUpdated);
				}
			}

			if constexpr (std::is_same_v<T, SphereColliderComponent>) {
				if ((m_Scene->IsRunning() || m_Scene->IsSimulating()) && m_Scene->GetPhysScene())
					component.Init();

				if (HasComponent<RigidbodyComponent>())
				{
					if (auto actor_ref = GetComponent<RigidbodyComponent>().GetActor(); actor_ref && *actor_ref)
					{
						actor_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
						actor_ref->AddFlag(RigidbodyFlag_TransformUpdated);
					}
				}

			}

			if constexpr (std::is_same_v<T, BoxColliderComponent>)
			{
				if ((m_Scene->IsRunning() || m_Scene->IsSimulating()) && m_Scene->GetPhysScene())
					component.Init();

				if (HasComponent<RigidbodyComponent>())
				{
					if (auto actor_ref = GetComponent<RigidbodyComponent>().GetActor(); actor_ref && *actor_ref)
					{
						actor_ref->AddFlag(RigidbodyFlag_ShapesUpdated);
						actor_ref->AddFlag(RigidbodyFlag_TransformUpdated);
					}
				}

				// Get the Optimal Size and Center for Box Collider Based on MeshFilter
				if (HasComponent<MeshFilterComponent>())
				{
					glm::mat4 global_transform = GetTransform().GetGlobalTransform();

					auto mesh_asset = AssetManager::GetAsset<StaticMesh>(GetComponent<MeshFilterComponent>().StaticMeshHandle);

					if (mesh_asset)
					{
						Bounds_AABB mesh_bounds = mesh_asset->MeshBounds;

						// Create OBB transformation matrix
						glm::mat4 obb_transform = glm::mat4(1.0f);
						obb_transform = glm::translate(obb_transform, mesh_bounds.Center());
						obb_transform = glm::scale(obb_transform, mesh_bounds.Size());

						// Apply OBB to Global Transform
						global_transform *= obb_transform;

						// Inverse Global so Center and Size are local
						glm::mat4 inv_global = glm::inverse(global_transform);

						// Extract Center
						glm::vec3 center = (inv_global * global_transform)[3];

						// Extract Local Half Extents
						glm::vec3 right = glm::vec3(inv_global * global_transform[0]);		// X basis vector
						glm::vec3 up = glm::vec3(inv_global * global_transform[1]);			// Y basis vector
						glm::vec3 forward = glm::vec3(inv_global * global_transform[2]);	// Z basis vector
						glm::vec3 size = glm::vec3(glm::length(right), glm::length(up), glm::length(forward)) * 0.5f;

						component.SetCentre(center);
						component.SetSize(size);

					}
				}
			}

			return component;
		}

		// This removes any Component within the applicable Entity
		template <typename T>
		void RemoveComponent() {

			if constexpr (std::is_same_v<T, RigidbodyComponent>) {
				if (HasComponent<RigidbodyComponent>())
					PhysicsSystem::RemoveRigidBody({ m_EntityHandle, m_Scene }, m_Scene);
			}
			if constexpr (std::is_same_v<T, SphereColliderComponent>) {
				if (HasComponent<SphereColliderComponent>())
					PhysicsSystem::RemoveCollider({ m_EntityHandle, m_Scene }, m_Scene, PxGeometryType::eSPHERE);
			}
			if constexpr (std::is_same_v<T, BoxColliderComponent>) {
				if (HasComponent<BoxColliderComponent>())
					PhysicsSystem::RemoveCollider({ m_EntityHandle, m_Scene }, m_Scene, PxGeometryType::eBOX);
			}

			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

#pragma endregion

#pragma region Getters

		template<typename T>
		T& GetComponent() const 
		{
						
			// Return Blank Component - I want to be able to just call GetComponent, and 
			// not have to worry about error handling when an entity does not have a
			// specified component, rather, it logs in the console the entity does not
			// have a component.
			if (m_EntityHandle == entt::null) {
				L_CORE_ERROR("Entity Cannot GetComponent as Entity Handle is Null");

				return GetBlankComponent<T>();
			}
			
			if (!HasComponent<T>()) {
				L_CORE_ERROR("Entity Does Not Have Component");

				return GetBlankComponent<T>();
			}

			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetComponentInParent() const
		{
			std::function<T&(Entity)> recursive_parent_search;
			recursive_parent_search = [&](Entity current_entity) -> T&
			{
				// If the current entity has the component, return it
				if (current_entity.HasComponent<T>())
				{
					return current_entity.GetComponent<T>();
				}

				L_CORE_ASSERT(current_entity.HasComponent<HierarchyComponent>(), "Entity Does Not Have Hierarchy Component - All Entities Must Have This Component.");
				HierarchyComponent& hierarchy = current_entity.GetComponent<HierarchyComponent>();
				const Entity& parent_entity = hierarchy.HasParent() ? hierarchy.GetParentEntity() : Entity{};
				if (parent_entity)  // If a parent exists, continue searching
				{
					return recursive_parent_search(parent_entity);
				}

				return GetBlankComponent<T>();
			};

			L_CORE_ASSERT(HasComponent<HierarchyComponent>(), "Entity Does Not Have Hierarchy Component - All Entities Must Have This Component.");
			HierarchyComponent& hierarchy = GetComponent<HierarchyComponent>();
			const Entity& parent_entity = hierarchy.HasParent() ? hierarchy.GetParentEntity() : Entity{};
			if (parent_entity)  // If a parent exists, continue searching
			{
				return recursive_parent_search(parent_entity);
			}

			return GetBlankComponent<T>();
		}

		template<typename T>
		T& GetComponentInChild() const
		{
			std::function<T& (Entity)> recursive_child_search;
			recursive_child_search = [&](Entity current_entity) -> T&
			{
				L_CORE_ASSERT(current_entity.HasComponent<HierarchyComponent>(), "Entity Does Not Have Hierarchy Component - All Entities Must Have This Component.");
				const HierarchyComponent& hierarchy = current_entity.GetComponent<HierarchyComponent>();

				if (hierarchy.HasChildren())
				{
					const auto& children = hierarchy.GetChildren();

					// 1. Check all direct children for the component
					for (const auto& child_uuid : children)
					{
						Entity child_entity = m_Scene->FindEntityByUUID(child_uuid);
						if (child_entity && child_entity.HasComponent<T>())
						{
							return child_entity.GetComponent<T>();
						}
					}

					// 2. If not found, recurse into children
					for (const auto& child_uuid : children)
					{
						Entity child_entity = m_Scene->FindEntityByUUID(child_uuid);
						if (!child_entity) continue;  // Ensure entity is valid before recursing

						T& found_component = recursive_child_search(child_entity);
						if (&found_component != &GetBlankComponent<T>())  // Component found in a child
						{
							return found_component;
						}
					}
				}

				return GetBlankComponent<T>();
			};

			return recursive_child_search(*this);
		}

		template<typename T>
		std::vector<Entity> GetComponentsInParents() const
		{
			std::vector<Entity> entities_with_component;

			std::function<void(Entity)> recursive_parent_search;
			recursive_parent_search = [&](Entity current_entity)
			{
				if (current_entity.HasComponent<T>())
				{
					entities_with_component.push_back(current_entity);
				}

				L_CORE_ASSERT(current_entity.HasComponent<HierarchyComponent>(), "Entity Does Not Have Hierarchy Component - All Entities Must Have This Component.");
				HierarchyComponent& hierarchy = current_entity.GetComponent<HierarchyComponent>();
				if (hierarchy.HasParent())
				{
					recursive_parent_search(hierarchy.GetParentEntity());
				}
			};

			L_CORE_ASSERT(HasComponent<HierarchyComponent>(), "Entity Does Not Have Hierarchy Component - All Entities Must Have This Component.");
			HierarchyComponent& hierarchy = GetComponent<HierarchyComponent>();
			if (hierarchy.HasParent())
			{
				recursive_parent_search(hierarchy.GetParentEntity());
			}

			return entities_with_component;
		}

		template<typename T>
		std::vector<Entity> GetComponentsInChildren() const
		{
			std::vector<Entity> entities_with_component;
			std::function<void(Entity)> recursive_child_search;

			recursive_child_search = [&](Entity current_entity)
			{
				L_CORE_ASSERT(current_entity.HasComponent<HierarchyComponent>(), "Entity does not have a HierarchyComponent.");
				const HierarchyComponent& hierarchy = current_entity.GetComponent<HierarchyComponent>();

				if (hierarchy.HasChildren())
				{
					const auto& children = hierarchy.GetChildren();

					// 1. Check all direct children for the component
					for (const auto& child_uuid : children)
					{
						Entity child_entity = m_Scene->FindEntityByUUID(child_uuid);
						if (child_entity && child_entity.HasComponent<T>())
						{
							entities_with_component.push_back(child_entity);
						}
					}

					// 2. Recurse into children
					for (const auto& child_uuid : children)
					{
						Entity child_entity = m_Scene->FindEntityByUUID(child_uuid);
						if (child_entity)
						{
							recursive_child_search(child_entity);
						}
					}
				}
			};

			recursive_child_search(*this);
			return entities_with_component;
		}

		template<typename T>
		std::vector<Entity> GetComponentsInSelfAndChildren() const
		{
			std::vector<Entity> entities_with_component;

			if (this->HasComponent<T>()) entities_with_component.push_back(*this);

			auto children = this->GetComponentsInChildren<T>();
			entities_with_component.insert(entities_with_component.end(), children.begin(), children.end());
			return entities_with_component;
		}

		TransformComponent& GetTransform() const
		{
			return GetComponent<TransformComponent>();
		}

		Scene* GetScene() const { return m_Scene; }
		const UUID& GetUUID() const {
			static UUID null_uuid_instance = UUID{ NULL_UUID };
			return (operator bool()) ? GetComponent<IDComponent>().ID : null_uuid_instance;
		}
		const std::string& GetName() const { return GetComponent<TagComponent>().Tag; }

		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

#pragma endregion

#pragma region Validation

		// This returns if the Entity has an applicable Component
		template <typename T>
		bool HasComponent() const { return m_Scene->m_Registry.all_of<T>(m_EntityHandle); }

		template<typename... Components>
		bool HasAnyComponent() const { return m_Scene->m_Registry.any_of<Components...>(m_EntityHandle); }

		operator bool() const { return m_Scene ? m_Scene->GetRegistry()->valid(m_EntityHandle) : m_EntityHandle != entt::null; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }

#pragma endregion

	private:

		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

		template<typename T>
		static T& GetBlankComponent()
		{
			static std::unordered_map<std::type_index, std::shared_ptr<void>> s_BlankComponents;

			auto it = s_BlankComponents.find(typeid(T));
			if (it == s_BlankComponents.end())
			{
				s_BlankComponents[typeid(T)] = std::make_shared<T>();
			}

			return *std::static_pointer_cast<T>(s_BlankComponents[typeid(T)]);
		}

		void InitAllEntityScripts();

		friend struct ComponentBase;
	};


}