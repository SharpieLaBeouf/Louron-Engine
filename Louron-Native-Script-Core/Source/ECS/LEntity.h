#pragma once

#include "LComponent.h"

#include "../Defines/LTypes.h"
#include "../Engine Callbacks.h"

namespace Louron
{

	namespace Components
	{
		class Component;
		class LODMeshComponent;
		class SkinnedMeshComponent;
	}

	/**
	* @brief Entity class representing an object in the scene.
	* 
	* The Entity class is what all scripts inherit from. It provides a way to interact with the engine's ECS (Entity Component System).
	* 
	* An Entity is merely a unique identifier (UUID) that can be used to access and manipulate components associated with it.
	* 
	* You may get Entities from instantiating Prefabs, or by using the static find method to retrieve them by name or UUID.
	* 
	* Where there is a NULL entity, any callbacks to the engine will simply be ignored. 
	*/
	class Entity
	{

	protected:

		/**
		* @brief The UUID this entity belongs to.
		*/
		uint32_t m_EntityID = NULL_UUID;

		/**
		* @brief Constructor for creating an entity with a specific UUID.
		* Protected to prevent direct instantiation, use either Create(), Instantiate(), FindByName(), or FindByUUID().
		*/
		Entity(uint32_t entity_id) : m_EntityID(entity_id) {}

		friend class Components::Component;
		friend class Components::LODMeshComponent;
		friend class Components::SkinnedMeshComponent;

	public:

		/**
		* @brief Default constructor for a null entity.
		*
		* In order to create an Entity within the scene, you must use either Create(), Instantiate(), FindByName(), or FindByUUID().
		*/
		Entity() = default;
		virtual ~Entity() = default;

		/**
		* @brief Get the UUID of the entity as a uint32_t.
		* @return uint32_t The UUID of the entity. 
		*/
		operator const uint32_t& () const { return m_EntityID; }

		/**
		* @brief Check if the entity is valid.
		* 
		* This will check in the Scene if the entity is still valid.
		* @return bool True if the entity is valid, false otherwise. 
		*/
		operator bool() const { return (m_EntityID == NULL_UUID) ? false : ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t), Entity_CheckValidEntity, m_EntityID); }

#pragma region Exposed Entity Functionality

		/**
		* @brief Get the name of the entity.
		* @return std::string The name of the entity.
		*/
		std::string GetTag() const { return std::string(ENGINE_SAFE_CALL_RET_PTR(const char*, const char* (*)(uint32_t), TagComponent_GetTag, m_EntityID)); }

		/**
		* @brief Set the name of the entity.
		* @param tag The new name of the entity.
		*/
		void SetTag(const std::string& tag)	const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*), TagComponent_SetTag, m_EntityID, tag.c_str()); }

		/**
		* @brief Get the transform of the entity.
		* @return Transform The transform of the entity.
		*/
		Transform GetTransform() const { return ENGINE_SAFE_CALL_RET(Transform, Transform(*)(uint32_t), TransformComponent_GetTransform, m_EntityID); }

		/**
		* @brief Set the transform of the entity.
		* @param transform The new transform of the entity.
		*/
		void SetTransform(const Transform& transform) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Transform*), TransformComponent_SetTransform, m_EntityID, &transform); }

		/**
		* @brief Get the position of the entity.
		* @return Vectors::Vector3 The position of the entity.
		*/
		Vectors::Vector3 GetPosition() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetPosition, m_EntityID); }

		/**
		* @brief Set the position of the entity.
		* @param position The new position of the entity.
		*/
		void SetPosition(const Vectors::Vector3& position) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), TransformComponent_SetPosition, m_EntityID, &position); }

		/**
		* @brief Get the rotation of the entity.
		* @return Vectors::Vector3 The rotation of the entity.
		*/
		Quaternion GetRotation() const { return ENGINE_SAFE_CALL_RET(Quaternion, Quaternion(*)(uint32_t), TransformComponent_GetRotation, m_EntityID); }

		/**
		* @brief Set the rotation of the entity.
		* @param rotation The new rotation of the entity.
		*/
		void SetRotation(const Quaternion& rotation) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Quaternion*), TransformComponent_SetRotation, m_EntityID, &rotation); }

		/**
		* @brief Get the scale of the entity.
		* @return Vectors::Vector3 The scale of the entity.
		*/
		Vectors::Vector3 GetScale() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetScale, m_EntityID); }

		/**
		* @brief Set the scale of the entity.
		* @param scale The new scale of the entity.
		*/
		void SetScale(const Vectors::Vector3& scale) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), TransformComponent_SetScale, m_EntityID, &scale); }

		/**
		* @brief Get the global world space transform of the entity.
		* @return Transform The transform of the entity.
		*/
		Transform GetGlobalTransform() const { return ENGINE_SAFE_CALL_RET(Transform, Transform(*)(uint32_t), TransformComponent_GetGlobalTransform, m_EntityID); }

		/**
		* @brief Set the global world space transform of the entity.
		* @param transform The new transform of the entity.
		*/
		void SetGlobalTransform(const Transform& transform) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Transform*), TransformComponent_SetGlobalTransform, m_EntityID, &transform); }

		/**
		* @brief Get the global world space position of the entity.
		* @return Vectors::Vector3 The position of the entity.
		*/
		Vectors::Vector3 GetGlobalPosition() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetGlobalPosition, m_EntityID); }

		/**
		* @brief Set the global world space position of the entity.
		* @param position The new position of the entity.
		*/
		void SetGlobalPosition(const Vectors::Vector3& position) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), TransformComponent_SetGlobalPosition, m_EntityID, &position); }

		/**
		* @brief Get the global world space rotation of the entity.
		* @return Vectors::Vector3 The rotation of the entity.
		*/
		Quaternion GetGlobalRotation() const { return ENGINE_SAFE_CALL_RET(Quaternion, Quaternion(*)(uint32_t), TransformComponent_GetGlobalRotation, m_EntityID); }

		/**
		* @brief Set the global world space rotation of the entity.
		* @param rotation The new rotation of the entity.
		*/
		void SetGlobalRotation(const Quaternion& rotation) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Quaternion*), TransformComponent_SetGlobalRotation, m_EntityID, &rotation); }

		/**
		* @brief Get the global world space scale of the entity.
		* @return Vectors::Vector3 The scale of the entity.
		*/
		Vectors::Vector3 GetGlobalScale() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetGlobalScale, m_EntityID); }

		/**
		* @brief Set the global world space scale of the entity.
		* @param scale The new scale of the entity.
		*/
		void SetGlobalScale(const Vectors::Vector3& scale) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), TransformComponent_SetGlobalScale, m_EntityID, &scale); }

		/**
		* @brief Get the front vector of the entity.
		* @return Vectors::Vector3 The front vector of the entity.
		*/
		Vectors::Vector3 GetFront() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetFront, m_EntityID); }

		/**
		* @brief Set the front vector of the entity.
		* @param front The new front vector of the entity.
		*/
		void SetFront(const Vectors::Vector3& front) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector3*), TransformComponent_SetFront, m_EntityID, &front); }

		/**
		* @brief Get the up vector of the entity.
		* @return Vectors::Vector3 The up vector of the entity.
		*/
		Vectors::Vector3 GetUp() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetUp, m_EntityID); }

		/**
		* @brief Get the right vector of the entity.
		* @return Vectors::Vector3 The right vector of the entity.
		*/
		Vectors::Vector3 GetRight() const { return ENGINE_SAFE_CALL_RET(Vectors::Vector3, Vectors::Vector3(*)(uint32_t), TransformComponent_GetRight, m_EntityID); }

		/**
		* @brief Get the parent entity.
		* @return Entity The parent entity.
		*/
		Entity GetParent() const { return Entity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), Entity_GetParent, m_EntityID)); }

		/**
		* @brief Set the parent entity.
		* @param parent_entity The new parent entity.
		*/
		void SetParent(Entity parent_entity) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t), Entity_SetParent, m_EntityID, parent_entity.m_EntityID); }

		std::vector<Entity> GetChildren() const
		{
			std::vector<Entity> children{};

			size_t child_count = 0;
			uint64_t cache_key = ENGINE_SAFE_CALL_RET(uint64_t, uint64_t(*)(uint32_t, uint8_t, size_t*), Entity_GetComponentsInChildrenCount, m_EntityID, static_cast<uint8_t>(::BackEndAPI::FieldType::IDComponent), &child_count);

			children.reserve(child_count);
			if (child_count > 0 && cache_key != uint64_t(-1))
			{
				std::unique_ptr<uint32_t[]> entity_array(new uint32_t[child_count]);
				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t*, size_t, uint64_t), Entity_GetComponentsInHierarchyCopy, entity_array.get(), child_count, cache_key);

				for (size_t i = 0; i < child_count; ++i)
				{
					children.push_back(std::move(Entity(entity_array[i])));
				}
			}

			return children;
		}

#pragma endregion

#pragma region Static Methods

		/**
		* @brief Instantiate a prefab within the Scene.
		* 
		* Create a new entity from a prefab. 
		* 
		* The prefab must be valid.
		* 
		* The new entity will be placed at the origin of the world with no rotation, and a default scale of 1.0f.
		* 
		* @param prefab The prefab to instantiate.
		* @return Entity The new entity created from the prefab.
		*/
		static Entity Instantiate(Assets::Prefab prefab) { return (prefab != NULL_UUID) ? Entity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), Entity_Instantiate, prefab.operator uint32_t())) : Entity(NULL_UUID); }
		
		/**
		* @brief Instantiate a prefab within the Scene.
		*
		* Create a new entity from a prefab with a transform in world space.
		*
		* The prefab must be valid.
		*
		* The new entity will be placed at the transform in world space (NOT LOCAL SPACE).
		*
		* @param prefab The prefab to instantiate.
		* @param transform The transform to place the new entity at in World Space.
		* @return Entity The new entity created from the prefab.
		*/
		static Entity Instantiate(Assets::Prefab prefab, const Transform& transform) { return (prefab != NULL_UUID) ? Entity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t, const Transform*), Entity_Instantiate_Transform, prefab.operator uint32_t(), &transform)) : Entity(NULL_UUID); }
		
		/**
		* @brief Instantiate a prefab within the Scene.
		* 
		* Create a new entity from a prefab with a Parent.
		*
		* The prefab must be valid.
		*
		* The new entity will be placed at the origin relative to the parent.
		* 
		* @param prefab The prefab to instantiate.
		* @param parent The parent entity to attach the new entity to.
		* @return Entity The new entity created from the prefab.
		*/
		static Entity Instantiate(Assets::Prefab prefab, Entity parent)
		{
			if (prefab != NULL_UUID)
			{
				Entity entity{ ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), Entity_Instantiate, prefab) };
				if (entity.m_EntityID != NULL_UUID)
				{
					entity.SetParent(parent.m_EntityID);
					entity.SetTransform(Transform{ Vectors::Vector3(0.0f), Quaternion(1.0f, 0.0f, 0.0f, 0.0f), Vectors::Vector3(1.0f) });
				}
				return entity;
			}
			return Entity(NULL_UUID);
		}

		/**
		* @brief Instantiate a prefab within the Scene.
		*
		* Create a new entity from a prefab with a parent and a local transform.
		*
		* The prefab must be valid.
		*
		* The new entity will be placed at the transform in local space relative to the parent (NOT WORLD SPACE).
		*
		* @param prefab The prefab to instantiate.
		* @param parent The parent entity to attach the new entity to.
		* @param transform The transform to place the new entity at in local space relative to parent.
		* @return Entity The new entity created from the prefab.
		*/
		static Entity Instantiate(Assets::Prefab prefab, Entity parent, const Transform& transform)
		{
			if (prefab != NULL_UUID)
			{
				Entity entity{ ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), Entity_Instantiate, prefab) };
				if (entity.m_EntityID != NULL_UUID)
				{
					entity.SetParent(parent.m_EntityID);
					entity.SetTransform(transform);
				}
				return entity;
			}
			return Entity(NULL_UUID);
		}

		/**
		* @brief Create a new entity within the Scene.
		* 
		* Create an empty entity with no components.
		* 
		* @param name The name of the new entity.
		* @return Entity The new entity created.
		*/
		static Entity Create(const std::string& name = "Untitled Entity") { return Entity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const char*), Entity_Create, name.c_str())); }
		
		/**
		* @brief Destroy an entity within the Scene.
		* @param entity_id The entity to destroy.
		*/
		static void Destroy(Entity entity) { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Entity_Destroy, entity.m_EntityID); }

		/**
		* @brief Find an entity by name.
		* @param name The name of the entity to find.
		* @return Entity The entity found by name. Null entity returned otherwise.
		*/
		static Entity FindByName(const std::string& name) { return Entity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const char*), Entity_FindByName, name.c_str())); }
		
		/**
		* @brief Find an entity by UUID.
		* @param entity_id The UUID of the entity to find.
		* @return Entity The entity found by UUID. Null entity returned otherwise.
		*/
		static Entity FindByUUID(uint32_t entity_id) { return Entity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), Entity_FindByUUID, entity_id)); }

#pragma endregion

#pragma region Components

		/**
		* @brief Check if the entity has a component of type T.
		* 
		* You can only use this function on components that inherit from Louron::Components::Component.
		* 
		* @param T The type of component to check for.
		* @return bool True if the entity has the component, false otherwise. 
		* 
		* @code
		* if (other_entity.HasComponent<RigidbodyComponent>())
		* {
		*     RigidbodyComponent rigidbody = other_entity.GetComponent<RigidbodyComponent>();
		*     rigidbody.SetMass(10.0f);
		* }
		* @endcode
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		bool HasComponent() const
		{
			uint8_t type = static_cast<uint8_t>(T::GetType());
			return ENGINE_SAFE_CALL_RET(bool, bool(*)(uint32_t, uint8_t), Entity_HasComponent, m_EntityID, type);
		}

		/**
		* @brief Add a component of type T to the entity.
		* 
		* You can only use this function on components that inherit from Louron::Components::Component.
		* 
		* @param T The type of component to add.
		* @return T The component added to the entity. If the component already exists, it will return the existing component.
		* 
		* @code
		* Entity other_entity = Entity::FindByName("Other Entity");
		* RigidbodyComponent rigidbody = other_entity.AddComponent<RigidbodyComponent>();
		* rigidbody.SetMass(10.0f);
		* @endcode 
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		T AddComponent() const
		{
			if (!HasComponent<T>())
			{
				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), Entity_AddComponent, m_EntityID, static_cast<uint8_t>(T::GetType()));
			}

			T component{};
			component.SetEntity(m_EntityID);
			return component;
		}

		/**
		* @brief Remove a component of type T from the entity.
		* 
		* You can only use this function on components that inherit from Louron::Components::Component.
		* 
		* Attempts to remove a component from an entity.
		* 
		* @param T The type of component to remove.
		* 
		* @code
		* if (player_health <= 0.0f)
		* {
		*     RemoveComponent<RigidbodyComponent>();
		* }
		* @endcode
		* 
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		void RemoveComponent() const
		{
			ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint8_t), Entity_RemoveComponent, m_EntityID, static_cast<uint8_t>(T::GetType()));
		}

		/**
		* @brief Get a component of type T from the entity.
		* 
		* You can only use this function on components that inherit from Louron::Components::Component.
		* 
		* @param T The type of component to get.
		* @return T The component found in the entity. If no component is found, a null component is returned.
		* 
		* @code
		* RigidbodyComponent rigidbody = GetComponent<RigidbodyComponent>();
		* rigidbody.SetKinematic(true);
		* @endcode
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		T GetComponent() const
		{
			if (HasComponent<T>())
			{
				T component{};
				component.SetEntity(m_EntityID);
				return component;
			}

			static T null_component = {};
			null_component.SetEntity(NULL_UUID);
			return null_component;
		}

		/**
		* @brief Try and get a component of type T from a parent entity.
		* 
		* This will recurisvely search upwards to the root of the scene for a parent entity which holds this component.
		* 
		* @param T The type of component to get.
		* @return T The component found in the parent entity. If no component is found, a null component is returned.
		* 
		* @code
		* RigidbodyComponent rigidbody = GetComponentInParent<RigidbodyComponent>();
		* rigidbody.SetGravity(false); // Space mode...
		* @endcode
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		T GetComponentInParent() const
		{
			T component{};
			component.SetEntity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t, uint8_t), Entity_GetComponentInParent, m_EntityID, static_cast<uint8_t>(T::GetType())));
			return component;
		}

		/**
		* @brief Get all component's of type T from all parent entities.
		*
		* This will recurisvely search upwards to the root of the scene, and gather all components contained by parent entities that hold this component.
		*
		* @param T The type of component to get.
		* @return std::vector<T> A vector of components found in the parent entities. If no component's are found, an empty vector is returned.
		*
		* @code
		* auto parent_components = GetComponentsInParent<RigidbodyComponent>();
		* for(auto& component : parent_components)
		* {
		*     component.SetGravity(false); // Space mode...
		* }
		* @endcode
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		std::vector<T> GetComponentsInParent() const 
		{
			std::vector<T> components{};

			size_t component_count = 0;
			uint64_t cache_key = ENGINE_SAFE_CALL_RET(uint64_t, uint64_t(*)(uint32_t, uint8_t, size_t*), Entity_GetComponentsInParentCount, m_EntityID, static_cast<uint8_t>(T::GetType()), &component_count);

			if (component_count > 0 && cache_key != uint64_t(-1))
			{
				std::unique_ptr<uint32_t[]> entity_array(new uint32_t[component_count]);
				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t*, size_t, uint64_t), Entity_GetComponentsInHierarchyCopy, entity_array.get(), component_count, cache_key);

				for (size_t i = 0; i < component_count; ++i)
				{
					T component{};
					component.SetEntity(entity_array[i]);
					components.emplace_back(component);
				}
			}

			return components;
		}

		/**
		* @brief Try and get a component of type T from a child entity.
		*
		* This will recurisvely search downwards through the children of the current entity for a child entity which holds this component.
		*
		* @param T The type of component to get.
		* @return T The component found in the child entity. If no component is found, a null component is returned.
		*
		* @code
		* RigidbodyComponent rigidbody = GetComponentInChildren<RigidbodyComponent>();
		* rigidbody.SetGravity(false); // Space mode...
		* @endcode
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		T GetComponentInChildren() const
		{
			T component{};
			component.SetEntity(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t, uint8_t), Entity_GetComponentInChildren, m_EntityID, static_cast<uint8_t>(T::GetType())));
			return component;
		}

		/**
		* @brief Get all component's of type T from all children entities.
		*
		* This will recurisvely search downwards through the children of the current entity, and gather all components contained by child entities that hold this component.
		*
		* @param T The type of component to get.
		* @return std::vector<T> A vector of components found in child entities. If no component's are found, an empty vector is returned.
		*
		* @code
		* auto child_components = GetComponentsInChildren<RigidbodyComponent>();
		* for(auto& component : child_components)
		* {
		*     component.SetGravity(false); // Space mode...
		* }
		* @endcode
		*/
		template<typename T, typename = std::enable_if_t<std::is_base_of<Components::Component, T>::value>>
		std::vector<T> GetComponentsInChildren() const
		{
			std::vector<T> components{};

			size_t component_count = 0;
			uint64_t cache_key = ENGINE_SAFE_CALL_RET(uint64_t, uint64_t(*)(uint32_t, uint8_t, size_t*), Entity_GetComponentsInChildrenCount, m_EntityID, static_cast<uint8_t>(T::GetType()), &component_count);

			if (component_count > 0 && cache_key != uint64_t(-1))
			{
				std::unique_ptr<uint32_t[]> entity_array(new uint32_t[component_count]);
				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t*, size_t, uint64_t), Entity_GetComponentsInHierarchyCopy, entity_array.get(), component_count, cache_key);

				for (size_t i = 0; i < component_count; ++i)
				{
					T component{};
					component.SetEntity(entity_array[i]);
					components.emplace_back(component);
				}
			}

			return components;
		}

#pragma endregion

	};
}