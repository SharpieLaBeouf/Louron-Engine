#include "Scene.h"

// Louron Core Headers
#include "Entity.h"
#include "Prefab.h"
#include "Scene Serializer.h"
#include "Spatial Partitioning/OctreeBounds.h"

#include "../Animation/Animations.h"

#include "../Core/UUID.h"
#include "../Core/Time.h"
#include "../Core/Input.h"
#include "../Debug/Profiler.h"
#include "../Physics/PhysicsWrappers.h"
#include "../Physics/CollisionCallback.h"

#include "../Jobs/Jobs.h"

#include "Components/Core Components.h"
#include "Components/Light Components.h"
#include "Components/Mesh Components.h"
#include "Components/Skybox Component.h"
#include "Components/Physics/Collider Components.h"
#include "Components/Physics/Rigidbody Component.h"
#include "Components/Animator Component.h"
#include "Components/SkinnedMeshComponent.h"

#include "Scene Systems/Physics System.h"

#include "../Renderer/Camera.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RendererPipeline.h"

#include "../OpenGL/Framebuffer.h"

#include "../Scripting/Script Manager.h"
#include "../Scripting/Script Register.h"
#include "../Scripting/Script Defines.h"

#include "../Project/Project.h"

// C++ Standard Library Headers
#include <iomanip>
#include <future>

// External Vendor Library Headers
#include <glm/gtc/quaternion.hpp>

#include <imgui.h>

namespace Louron {

#pragma region Initialisation and ECS

	static PxFilterFlags CustomFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST;
		return PxFilterFlag::eDEFAULT;
	}

	Scene::Scene()
	{
		m_SceneConfig.SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneConfig.SceneFilePath.filename().replace_extension().string();

		m_SceneConfig.ScenePipelineType = L_RENDER_PIPELINE::FORWARD_PLUS;
	}

	Scene::Scene(L_RENDER_PIPELINE pipeline)
	{
		m_SceneConfig.SceneFilePath = "Scenes/Untitled Scene.lscene";

		m_SceneConfig.Name = m_SceneConfig.SceneFilePath.filename().replace_extension().string();
		m_SceneConfig.ScenePipelineType = pipeline;
	}

	/// <summary>
	/// Once the Scene has been initialised, call this to load the scene from file.
	/// </summary>
	/// <returns>Returns true if the SceneFile was loaded successfully, returns false if not.</returns>
	bool Scene::LoadSceneFile(const std::filesystem::path& sceneFilePath)
	{
		std::filesystem::path outFilePath = sceneFilePath;

		// Check if Scene File Path is Empty.
		if (outFilePath.empty()) {
			L_CORE_ERROR("Scene File Path Empty - Cannot Load Scene Data from File!");
			return false;
		}

		// Check if Scene File Extension is Incompatible.
		if (outFilePath.extension() != ".lscene") {
			L_CORE_ERROR("Scene File Path Extension Incompatible - Cannot Load Scene Data from File!");
			return false;
		}

		// Load Existing Scene File or Create New Scene.
		if (std::filesystem::exists(outFilePath)) {

			SceneSerializer serializer(std::static_pointer_cast<Scene>(shared_from_this()));
			if (serializer.Deserialize(outFilePath)) {
				m_SceneConfig.SceneFilePath = std::filesystem::relative(outFilePath, Project::GetActiveProject()->GetProjectDirectory());
				return true;
			}

			L_CORE_ERROR("Scene File Path Does Not Exist - Cannot Load Scene Data from File!");
		}
		return false;
	}

	/// <summary>
	/// Creates Entity in Scene and Generates New UUID
	/// </summary>
	Entity Scene::CreateEntity(const std::string& name) {
		return CreateEntity(UUID(), name);
	}

	/// <summary>
	/// Create Entity in Scene with UUID
	/// </summary>
	Entity Scene::CreateEntity(UUID uuid, const std::string& name) {

		std::unique_lock<std::mutex> lock;

		if (m_Octree)
			lock = std::unique_lock<std::mutex>(m_Octree->GetOctreeMutex());

		while (m_EntityMap->find(uuid) != m_EntityMap->end()) 
		{
			uuid = UUID();
		}

		Entity entity = { m_Registry.create(), this };

		// 1. Add UUID Component
		auto& temp = entity.AddComponent<IDComponent>(uuid);

		// 2. Add Transform Component
		entity.AddComponent<TransformComponent>();

		// 3. Add Tag Component
		std::string uniqueName = name.empty() ? "Untitled Entity" : name;
		int suffix = 1;
		std::string baseName = uniqueName;

		// Ensure the name is unique by appending a numeric suffix		
		auto check_tags = [&](const char* name) -> bool {

			auto view = m_Registry.view<TagComponent>();
			for (auto entity : view) {
				const TagComponent& tag = view.get<TagComponent>(entity);
				if (tag.Tag == name)
					return true;
			}

			return false;
			};

		while (check_tags(uniqueName.c_str())) {
			uniqueName = baseName + " (" + std::to_string(suffix++) + ")";
		}
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = uniqueName;

		// 4. Add Hierarchy Component
		entity.AddComponent<HierarchyComponent>();

		m_EntityMap->emplace(uuid, entity);

		return entity;
	}

#pragma region Component Copying

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, Scene* scene_ref)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto source_entity_handle : view)
				{
					auto& source_component = src.get<Component>(source_entity_handle);

					auto source_entity = source_component.GetEntity();
					if (!source_entity)
						continue;

					entt::entity dest_entity_handle = enttMap.at(src.get<IDComponent>(source_entity_handle).ID);
					Entity dest_entity = { dest_entity_handle, scene_ref };

					// Temporarily Set Source Entity to Destination Entity for Component Copying
					source_component.SetEntity(dest_entity);

					auto& dest_component = dst.emplace_or_replace<Component>(dest_entity_handle, source_component);

					// Revert Source Component Entity Reference to Correct Reference
					source_component.SetEntity(*source_entity.get());

					// Ensure Destination Component Has Correct Entity
					dest_component.SetEntity(dest_entity);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap, Scene* scene_ref)
	{
		CopyComponent<Component...>(dst, src, enttMap, scene_ref);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	/// <summary>
	/// Copy Constructor and Operator Deleted in ENTT for Registry.
	/// Have to Manually Copy Over Data.
	/// </summary>
	bool Scene::CopyRegistry(std::shared_ptr<Scene> otherScene)
	{
		auto& srcSceneRegistry = otherScene->m_Registry;
		auto& dstSceneRegistry = m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap, this);
		return true;
	}

	std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> source_scene)
	{
		L_RENDER_PIPELINE pipeline = source_scene->GetConfig().ScenePipelineType;

		std::shared_ptr<Scene> dest_scene = std::make_shared<Scene>(pipeline);

		dest_scene->m_DisplayOctree = source_scene->m_DisplayOctree;
		dest_scene->m_IsPaused = source_scene->m_IsPaused;
		dest_scene->m_IsRunning = source_scene->m_IsRunning;
		dest_scene->m_IsPhysicsCalculating = source_scene->m_IsPhysicsCalculating;

		dest_scene->m_SceneConfig.Name = source_scene->m_SceneConfig.Name;
		dest_scene->m_SceneConfig.SceneFilePath = source_scene->m_SceneConfig.SceneFilePath;
		dest_scene->m_SceneConfig.ScenePipelineType = source_scene->m_SceneConfig.ScenePipelineType;

		dest_scene->CopyRegistry(source_scene);

		// Generate Octree for New Scene
		{
			// Calculate Overall Scene Octree
			OctreeBoundsConfig octree_config{};

			std::vector<OctreeBounds<Entity>::OctreeData> data_sources;

			auto static_mesh_view = dest_scene->GetAllEntitiesWith<MeshFilterComponent, MeshRendererComponent>();
			for (const auto& entity_handle : static_mesh_view) {
				auto& mesh_filter = static_mesh_view.get<MeshFilterComponent>(entity_handle);

				// Ensure the AABB is up to date
				mesh_filter.UpdateTransformedAABB();

				const auto& aabb = mesh_filter.TransformedAABB;

				if (!mesh_filter.GetEntity()) {
					L_CORE_ERROR("Cannot Insert Entity to Octree - Current Entity Is Invalid!");
					continue;
				}

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(*mesh_filter.GetEntity(), aabb));
			}

			auto skinned_mesh_view = dest_scene->GetAllEntitiesWith<SkinnedMeshComponent>();
			for (const auto& entity_handle : skinned_mesh_view) {
				auto& skinned_mesh = skinned_mesh_view.get<SkinnedMeshComponent>(entity_handle);

				// Ensure the AABB is up to date
				skinned_mesh.UpdateTransformedAABB();

				const auto& aabb = skinned_mesh.TransformedAABB;

				if (!skinned_mesh.GetEntity()) {
					L_CORE_ERROR("Cannot Insert Entity to Octree - Current Entity Is Invalid!");
					continue;
				}

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(*skinned_mesh.GetEntity(), aabb));
			}

			octree_config.Looseness = 1.25f;
			octree_config.PreferredDataSourceLimit = 8;

			// Create the octree and insert data sources
			dest_scene->m_Octree = std::make_shared<OctreeBounds<Entity>>(octree_config, data_sources);
		}

		return dest_scene;
	}

#pragma endregion

	void Scene::SetPhysScene(PxScene* physScene) {

		// Release PhysX Scene
		if (m_PhysxScene) {
			m_PhysxScene->release();
			m_PhysxScene = physScene;
			return;
		}

		if (physScene) {
			m_PhysxScene = physScene;
			return;
		}

		L_CORE_WARN("Physics Scene Not Valid.");
		m_PhysxScene = nullptr;
	}

	// FBO Stuff
	void Scene::CreateSceneFrameBuffer(const FrameBufferConfig& framebuffer_config)
	{
		m_SceneFrameBuffer = std::make_unique<FrameBuffer>(framebuffer_config);
		OnViewportResize({ framebuffer_config.Width, framebuffer_config.Height });
	}
	const std::unique_ptr<FrameBuffer>& Scene::GetSceneFrameBuffer() const { return m_SceneFrameBuffer; }

	// TODO: Duplicates Entity in Scene
	Entity Scene::DuplicateEntity(Entity sourceEntity, Entity parent, std::shared_ptr<std::unordered_map<UUID, UUID>> reference_map)
	{
		// Step 1: Create the new entity & entity reference map
		std::string name = sourceEntity.HasComponent<TagComponent>() ? sourceEntity.GetComponent<TagComponent>().Tag : "Unnamed Entity";
		Entity new_entity = CreateEntity(name + " (Copy)");

		// First iteration will create the reference map which will be passed to all recursive calls
		// Key = Source Entity UUID
		// Value = New Entity UUID
		if (!reference_map) reference_map = std::make_shared<std::unordered_map<UUID, UUID>>();
		reference_map->emplace(sourceEntity.GetUUID(), new_entity.GetUUID());

		// Step 2: Copy all components, excluding IDComponent and HierarchyComponent (which we handle manually)
		CopyComponentIfExists(AllDuplicatableComponents{}, new_entity, sourceEntity);

		auto& this_transform = new_entity.GetComponent<TransformComponent>();
		auto& other_transform = sourceEntity.GetComponent<TransformComponent>();

		this_transform.SetPosition(other_transform.GetLocalPosition());
		this_transform.SetRotation(other_transform.GetLocalRotation());
		this_transform.SetScale(other_transform.GetLocalScale());

		// Step 3: Handle hierarchy manually
		if (parent) 
		{
			new_entity.AddComponent<HierarchyComponent>();
			new_entity.GetComponent<HierarchyComponent>().AttachParent(parent.GetUUID());
		}
		else if (sourceEntity.HasComponent<HierarchyComponent>()) 
		{
			new_entity.AddComponent<HierarchyComponent>();
		}

		// Step 4: Recursively duplicate children
		if (sourceEntity.HasComponent<HierarchyComponent>()) 
		{
			const auto& children = sourceEntity.GetComponent<HierarchyComponent>().GetChildren();

			for (const auto& child_uuid : children) {
				Entity child_entity = FindEntityByUUID(child_uuid);
				if (!child_entity)
					continue;

				Entity duplicatedChild = DuplicateEntity(child_entity, new_entity, reference_map);
			}
		}

		if (!parent)
		{
			// Step 5. Refresh transform hierarchy
			new_entity.GetComponent<TransformComponent>().GetGlobalTransform();

			// Step 6. Validate All Skinned Mesh Component Bone References
			auto child_skinned_entities = new_entity.GetComponentsInSelfAndChildren<SkinnedMeshComponent>();
			for (auto& entity : child_skinned_entities)
			{
				if (entity.HasComponent<SkinnedMeshComponent>())
				{
					auto& skinned_mesh = entity.GetComponent<SkinnedMeshComponent>();
					for (auto& [bone_id, entity_uuid] : skinned_mesh.SkeletonBoneMapping)
					{
						if (reference_map->find(entity_uuid) != reference_map->end())
						{
							entity_uuid = reference_map->at(entity_uuid);
						}
						else
						{
							entity_uuid = NULL_UUID;
							L_CORE_WARN("Failed to Duplicate Skinned Mesh Bone Reference: {0}", std::to_string(bone_id));
						}
					}
				}
			}
		}

		return new_entity;
	}

	void Scene::DestroyEntity(const UUID& entity_uuid)
	{
		Entity entity = this->FindEntityByUUID(entity_uuid);

		if (!entity)
			return;

		this->DestroyEntity(entity);
	}

	// Destroys Entity in Scene
	void Scene::DestroyEntity(Entity entity, std::unique_lock<std::mutex>* parent_lock) {

		// Need to lock the octree because it may be trying to 
		// get things from scene as it's being deleted!
		std::unique_lock<std::mutex> octree_lock;
		if (!parent_lock && m_Octree)
			octree_lock = std::unique_lock<std::mutex>(m_Octree->GetOctreeMutex());

		// 1. Check if entity is valid
		if (!entity) {
			L_CORE_WARN("Attempted to Destroy Null Entity.");
			return;
		}

		// 2. Check If Entity is Part of the Scene
		if (!HasEntity(entity.GetUUID())) {
			L_CORE_WARN("Attempted to Destroy an Entity Not In The Scene.");
			return;
		}

		if (m_IsRunning && entity.HasComponent<ScriptComponent>())
			ScriptManager::Get()->OnDestroyAllScripts(entity.GetUUID());

		// 3. Call Physics System Remove Methods
		if (entity.HasAnyComponent<RigidbodyComponent, SphereColliderComponent, BoxColliderComponent>()) {

			if (entity.HasComponent<RigidbodyComponent>())
			{
				entity.GetComponent<RigidbodyComponent>().Shutdown();
				PhysicsSystem::RemoveRigidBody(entity, this);
			}

			if (entity.HasComponent<SphereColliderComponent>())
			{
				entity.GetComponent<SphereColliderComponent>().Shutdown();
				PhysicsSystem::RemoveCollider(entity, this, PxGeometryType::eSPHERE);
			}
			if (entity.HasComponent<BoxColliderComponent>())
			{
				entity.GetComponent<BoxColliderComponent>().Shutdown();
				PhysicsSystem::RemoveCollider(entity, this, PxGeometryType::eBOX);
			}
		}

		// 4. Remove From Parent and Destroy All Children
		if (entity.HasComponent<HierarchyComponent>()) {

			auto& component = entity.GetComponent<HierarchyComponent>();

			if (component.HasParent())
				component.DetachParent();

			// Make a copy of the child list so we don't invalidate 
			// the iterator whilst destroying children
			std::vector<UUID> children_vec = component.GetChildren();
			for (const auto& children_uuid : children_vec)
				DestroyEntity(FindEntityByUUID(children_uuid), &octree_lock);
		}

		// 5. Remove the Entity from the Scene Entity Map
		m_EntityMap->erase(entity.GetUUID());

		// 6. Destroy the Entity and Components from the ENTT Registry
		m_Registry.destroy(entity);
	}

	// Returns Entity within Scene on Tag Name
	Entity Scene::FindEntityByName(std::string_view name) {

		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view) {
			const TagComponent& tag = view.get<TagComponent>(entity);
			if (tag.Tag == name)
				return Entity{ entity, this };
		}

		L_CORE_WARN("Scene Does Not Have an Entity Named: {0}", name);
		return Entity{ entt::null, nullptr };
	}

	bool Scene::HasEntityByUUID(UUID uuid)
	{
		return m_EntityMap->find(uuid) != m_EntityMap->end();
	}

	Entity Scene::FindEntityByUUID(UUID uuid)
	{
		if (m_EntityMap->find(uuid) != m_EntityMap->end()) {
			return Entity{ m_EntityMap->at(uuid), this };
		}

		L_CORE_WARN("Entity UUID not found in scene: {0}", std::to_string(uuid));
		return Entity{ entt::null, nullptr };
	}

	// Returns Primary Camera Entity
	Entity Scene::GetPrimaryCameraEntity() {

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			const CameraComponent& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}

		L_CORE_WARN("Scene Does Not Have an Entity with a Camera Component.");
		return Entity{ entt::null, nullptr };
	}

	bool Scene::HasEntity(const Entity& entity)
	{
		if (ValidEntity(entity))
			return m_Registry.all_of(entity);
		return false;
	}
	bool Scene::HasEntity(const std::string& name)
	{
		return (FindEntityByName(name)) ? true : false;
	}
	bool Scene::HasEntity(const UUID& uuid)
	{
		return (FindEntityByUUID(uuid)) ? true : false;
	}

	bool Scene::ValidEntity(const Entity& entity)
	{
		return m_Registry.valid(entity);
	}

	Entity Scene::InstantiatePrefab(std::shared_ptr<Prefab> prefab, std::optional<TransformComponent> transform, const UUID& parent_uuid)
	{
		if (!prefab)
			return {};

		entt::registry* prefab_registry = &prefab->m_PrefabRegistry;
		entt::entity prefab_root_entity = prefab->m_RootEntity;
		std::string prefab_name = prefab->m_PrefabName;

		std::vector<Entity> LODMeshEntities;		// So we can easily resolve the prefab handles to entity UUIDs
		std::vector<Entity> SkinnedMeshEntities;	// So we can easily resolve the prefab handles to entity UUIDs
		std::unordered_map<Louron::UUID, Louron::UUID> PrefabUUID_To_EntityUUID{}; // Key == Prefab entt::entity, Value == instantiated_entity.GetUUID()

		std::function<Entity(entt::entity, const UUID&)> copy_prefab_entity = [&](entt::entity start_prefab_entity, const UUID& parent_uuid) -> Entity {

			Entity instantiated_entity = this->CreateEntity("");

			PrefabUUID_To_EntityUUID[(uint32_t)start_prefab_entity] = instantiated_entity.GetUUID();

			// 1. Copy Data in All Components
			{
				// 1.a. Tag Component
				if (prefab_registry->all_of<TagComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<TagComponent>(start_prefab_entity);
					instantiated_entity.GetComponent<TagComponent>().Tag = component.Tag;
				}

				// 1.b. Hierarchy Component
				if (prefab_registry->all_of<HierarchyComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<HierarchyComponent>(start_prefab_entity);
					instantiated_entity.GetComponent<HierarchyComponent>().AttachParent(parent_uuid);
				}

				// 1.c. Camera Component
				if (prefab_registry->all_of<CameraComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<CameraComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<CameraComponent>(component);
				}

				// 1.d. Audio Listener
				if (prefab_registry->all_of<AudioListenerComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<AudioListenerComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<AudioListenerComponent>(component);
				}

				// 1.e. Audio Emitter
				if (prefab_registry->all_of<AudioEmitterComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<AudioEmitterComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<AudioEmitterComponent>(component);
				}

				// 1.f. Transform Component
				if (prefab_registry->all_of<TransformComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<TransformComponent>(start_prefab_entity);
					auto& transform_component = instantiated_entity.GetTransform();

					if (parent_uuid == NULL_UUID) {
						// If we are the root entity, we check if the transform 
						// passed has a value, if not, standard copy from prefab
						if (transform.has_value())
						{
							transform_component.SetPosition(transform.value().GetLocalPosition());
							transform_component.SetRotation(transform.value().GetLocalRotation());
							transform_component.SetScale(transform.value().GetLocalScale());
						}
						else
						{
							transform_component.SetPosition(component.GetLocalPosition());
							transform_component.SetRotation(component.GetLocalRotation());
							transform_component.SetScale(component.GetLocalScale());
						}
					}
					else {
						transform_component.SetPosition(component.GetLocalPosition());
						transform_component.SetRotation(component.GetLocalRotation());
						transform_component.SetScale(component.GetLocalScale());
					}
				}

				// 1.g. MeshFilter
				if (prefab_registry->all_of<MeshFilterComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<MeshFilterComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<MeshFilterComponent>(component);

					// Ensure Mesh Loaded
					if (!Project::GetStaticAssetManager()->IsAssetLoaded(component.StaticMeshHandle))
					{
						Project::GetStaticAssetManager()->GetAsset(component.StaticMeshHandle);
					}
				}

				// 1.h. MeshRenderer
				if (prefab_registry->all_of<MeshRendererComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<MeshRendererComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<MeshRendererComponent>(component);

					// Ensure Materials Loaded
					for (const auto& material_pair : component.MaterialHandles)
					{
						if (!Project::GetStaticAssetManager()->IsAssetLoaded(material_pair.first))
						{
							Project::GetStaticAssetManager()->GetAsset(material_pair.first);
						}
					}
				}

				// 1.i. PointLight Component
				if (prefab_registry->all_of<PointLightComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<PointLightComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<PointLightComponent>(component);
				}

				// 1.j. SpotLight Component
				if (prefab_registry->all_of<SpotLightComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<SpotLightComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<SpotLightComponent>(component);
				}

				// 1.k. DirectionalLight Component
				if (prefab_registry->all_of<DirectionalLightComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<DirectionalLightComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<DirectionalLightComponent>(component);
				}

				// 1.l. Skybox Component
				if (prefab_registry->all_of<SkyboxComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<SkyboxComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<SkyboxComponent>(component);
				}

				// 1.m. Rigidbody Component
				if (prefab_registry->all_of<RigidbodyComponent>(start_prefab_entity)) {
					RigidbodyComponent component = prefab_registry->get<RigidbodyComponent>(start_prefab_entity);
					auto& ent_rb_component = instantiated_entity.AddComponent<RigidbodyComponent>();
					ent_rb_component = component;

					if (IsRunning() || IsSimulating())
						instantiated_entity.GetComponent<RigidbodyComponent>().Init(&instantiated_entity.GetTransform(), m_PhysxScene);
				}

				// 1.n. Sphere Collider
				if (prefab_registry->all_of<SphereColliderComponent>(start_prefab_entity)) {
					SphereColliderComponent component = prefab_registry->get<SphereColliderComponent>(start_prefab_entity);
					auto& ent_sc_component = instantiated_entity.AddComponent<SphereColliderComponent>();
					ent_sc_component = component;


					if (IsRunning() || IsSimulating())
						instantiated_entity.GetComponent<SphereColliderComponent>().Init();
				}

				// 1.o. Box Collider
				if (prefab_registry->all_of<BoxColliderComponent>(start_prefab_entity)) {
					BoxColliderComponent component = prefab_registry->get<BoxColliderComponent>(start_prefab_entity);
					auto& ent_bc_component = instantiated_entity.AddComponent<BoxColliderComponent>();
					ent_bc_component = std::move(component);

					if (IsRunning() || IsSimulating())
						instantiated_entity.GetComponent<BoxColliderComponent>().Init();
				}

				// 1.p. Script Component
				if (prefab_registry->all_of<ScriptComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<ScriptComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<ScriptComponent>(component);

					// TODO: Add functionality to move script and script fields from prefabs into new components
					// basically just need to copy the ScriptFieldInstances from the prefabs entries in the scriptmanager
					// to this new entity! easy peasy...

				}

				// 1.q. LOD Component
				if (prefab_registry->all_of<LODMeshComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<LODMeshComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<LODMeshComponent>(component);
					LODMeshEntities.push_back(instantiated_entity);
				}

				// 1.r. Skinned Mesh Component
				if (prefab_registry->all_of<SkinnedMeshComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<SkinnedMeshComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<SkinnedMeshComponent>(component);
					SkinnedMeshEntities.push_back(instantiated_entity);

					// Ensure Mesh Loaded
					if (!Project::GetStaticAssetManager()->IsAssetLoaded(component.StaticMeshHandle))
					{
						Project::GetStaticAssetManager()->GetAsset(component.StaticMeshHandle);
					}

					// Ensure Skeleton Loaded
					if (!Project::GetStaticAssetManager()->IsAssetLoaded(component.SkeletonHandle))
					{
						Project::GetStaticAssetManager()->GetAsset(component.SkeletonHandle);
					}

					// Ensure Materials Loaded
					for (const auto& material_pair : component.MaterialHandles)
					{
						if (!Project::GetStaticAssetManager()->IsAssetLoaded(material_pair.first))
						{
							Project::GetStaticAssetManager()->GetAsset(material_pair.first);
						}
					}
				}

				// 1.s. Animator Component
				if (prefab_registry->all_of<AnimatorComponent>(start_prefab_entity)) {
					auto& component = prefab_registry->get<AnimatorComponent>(start_prefab_entity);
					instantiated_entity.AddComponent<AnimatorComponent>(component);
				}
			}

			// 2. Recurse Children
			if (prefab_registry->all_of<HierarchyComponent>(start_prefab_entity))
			{
				for (const auto& child_uuid : prefab_registry->get<HierarchyComponent>(start_prefab_entity).GetChildren())
				{
					Entity child_entity = copy_prefab_entity(prefab->FindEntityByUUID(child_uuid), PrefabUUID_To_EntityUUID[(uint32_t)start_prefab_entity]);
				}
			}

			return instantiated_entity;

			};

		Entity instantiated_entity = copy_prefab_entity(prefab_root_entity, parent_uuid);

		// Resolve prefab handles to scene uuids
		for (auto& entity : LODMeshEntities)
		{
			if (!entity || !entity.HasComponent<LODMeshComponent>())
				continue;

			auto& component = entity.GetComponent<LODMeshComponent>();
			for (auto& element : component.LOD_Elements)
			{
				for (auto& entity_handle : element.MeshRendererEntities)
				{
					entity_handle = PrefabUUID_To_EntityUUID[entity_handle];
				}
			}
		}

		// Resolve prefab handles to scene uuids
		for (auto& entity : SkinnedMeshEntities)
		{
			if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
				continue;

			auto& component = entity.GetComponent<SkinnedMeshComponent>();

			std::shared_ptr<Skeleton> asset_skeleton = AssetManager::GetAsset<Skeleton>(component.SkeletonHandle);

			std::function<void(BoneLayout&)> recurse_bone_tree;

			recurse_bone_tree = [&](BoneLayout& current_bone)
				{
					component.SkeletonBoneMapping[current_bone.BoneID] = (uint32_t)PrefabUUID_To_EntityUUID[component.SkeletonBoneMapping[current_bone.BoneID]];

					for (auto& child_bone : current_bone.BoneChildren)
					{
						recurse_bone_tree(child_bone);
					}
				};

			recurse_bone_tree(asset_skeleton->SkeletonLayout);

			component.ComputeFinalBoneTransformations();
		}

		return instantiated_entity;
	}

#pragma endregion

#pragma region Scene Logic

	// SCENE HARD START & STOP
	// All scenes are started when they are created, not when we 
	// are playing. This is to setup required things such as 
	// collision callbacks and rendering pipeline
	void Scene::OnStart()
	{
		switch (m_SceneConfig.ScenePipelineType)
		{
		case L_RENDER_PIPELINE::FORWARD:		ForwardPipeline::OnStartPipeline(this);			break;
		case L_RENDER_PIPELINE::FORWARD_PLUS:	ForwardPlusPipeline::OnStartPipeline(this);		break;
		case L_RENDER_PIPELINE::DEFERRED:		DeferredRenderPipeline::OnStartPipeline(this);	break;
		}
	}

	void Scene::OnStop() {

		if (m_IsRunning)
			OnRuntimeStop();

		if (m_IsSimulating)
			OnSimulationStop();

		m_IsRunning = false;
		m_IsSimulating = false;

		switch (m_SceneConfig.ScenePipelineType)
		{
			case L_RENDER_PIPELINE::FORWARD:		ForwardPipeline::OnStopPipeline(this);	break;
			case L_RENDER_PIPELINE::FORWARD_PLUS:	ForwardPlusPipeline::OnStopPipeline(this);		break;
			case L_RENDER_PIPELINE::DEFERRED:		DeferredRenderPipeline::OnStopPipeline(this);	break;
		}
	}

	// RUNTIME
	void Scene::OnRuntimeStart() {

		m_IsRunning = true;

		OnPhysicsStart();

		// Scripting
		{
			// Instantiate all script entities
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& script_component = view.get<ScriptComponent>(e);
				for (const auto& [script_name, script_active] : script_component.Scripts)
				{
					if (!script_active) 
						continue;

					ScriptManager::Get()->OnCreateScript(entity.GetUUID(), script_name);
				}
			}
		}
	}

	void Scene::OnRuntimeStop() {

		m_IsRunning = false;
		m_IsSimulating = false;

		ScriptManager::Get()->RemoveAllScriptInstances();
		AssetManager::ClearRuntimeAssets();
		OnPhysicsStop();
	}

	// PHYSICS SIMULATION
	void Scene::OnSimulationStart() {

		m_IsSimulating = true;

		OnPhysicsStart();
	}

	void Scene::OnSimulationStop() {

		m_IsRunning = false;
		m_IsSimulating = false;

		OnPhysicsStop();
	}

	void Scene::OnPhysicsStart() {

		// 1. Create new PhysX scene
		PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.filterShader = CustomFilterShader;
		m_PhysxScene = PxGetPhysics().createScene(sceneDesc);

#ifdef _DEBUG
		PxPvdSceneClient* pvdClient = m_PhysxScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
#endif

		if (!m_CollisionCallback) {
			m_CollisionCallback = std::make_unique<CollisionCallback>(std::static_pointer_cast<Scene>(shared_from_this()));

			m_PhysxScene->setSimulationEventCallback(m_CollisionCallback.get());
		}

		// 2. Get All Entities with physics components
		std::unordered_map<UUID, Entity> entities;
		{
			auto rb_view = GetAllEntitiesWith<RigidbodyComponent>();
			for (auto& entt_id : rb_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto bc_view = GetAllEntitiesWith<BoxColliderComponent>();
			for (auto& entt_id : bc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto sc_view = GetAllEntitiesWith<SphereColliderComponent>();
			for (auto& entt_id : sc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}
		}

		// 3. Initialise All Physics Components
		for (auto& [uuid, entity] : entities) {

			if (entity.HasComponent<RigidbodyComponent>())
				entity.GetComponent<RigidbodyComponent>().Init(&entity.GetTransform(), m_PhysxScene);

			if (entity.HasComponent<BoxColliderComponent>())
				entity.GetComponent<BoxColliderComponent>().Init();

			if (entity.HasComponent<SphereColliderComponent>())
				entity.GetComponent<SphereColliderComponent>().Init();

		}
	}

	void Scene::OnPhysicsStop() {

		// 1. Get All Entities with physics components

		std::unordered_map<UUID, Entity> entities;
		{
			auto rb_view = GetAllEntitiesWith<RigidbodyComponent>();
			for (auto& entt_id : rb_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto bc_view = GetAllEntitiesWith<BoxColliderComponent>();
			for (auto& entt_id : bc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}

			auto sc_view = GetAllEntitiesWith<SphereColliderComponent>();
			for (auto& entt_id : sc_view)
			{
				Entity entity = { entt_id, this };
				entities[entity.GetUUID()] = entity;
			}
		}

		// 2. Shutdown All Physics Components
		for (auto& [uuid, entity] : entities) {

			if (entity.HasComponent<RigidbodyComponent>())
				entity.GetComponent<RigidbodyComponent>().Shutdown();

			if (entity.HasComponent<BoxColliderComponent>())
				entity.GetComponent<BoxColliderComponent>().Shutdown();

			if (entity.HasComponent<SphereColliderComponent>())
				entity.GetComponent<SphereColliderComponent>().Shutdown();
		}

		m_PhysxScene->release();
		m_PhysxScene = nullptr;
	}

	// UPDATE
	void Scene::OnUpdate() {

		L_PROFILE_SCOPE("Scene - OnUpdate");
		// Physics
		if (!m_IsPaused && (m_IsRunning || m_IsSimulating))
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Scene - Physics");

			PhysicsSystem::UpdatePhysicsObjects(std::static_pointer_cast<Scene>(shared_from_this()));
		}

		// Scripts - only if running
		if (!m_IsPaused && m_IsRunning)
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Scene - Scripts");

			// Validate and Update
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity_handle : view)
			{
				Entity entity = { entity_handle, this };
				auto& script_component = view.get<ScriptComponent>(entity_handle);
				for (const auto& [script_name, script_active] : script_component.Scripts)
				{
					// Validate Script Instance
					if (!script_active)
					{
						continue;
					}
					else if (!ScriptManager::Get()->GetScriptClassInstance(entity.GetUUID(), script_name))
					{
						// Script Active in Component, but Script Instance Not Created Yet
						ScriptManager::Get()->OnCreateScript(entity.GetUUID(), script_name);
					}

					// Call Update
					ScriptManager::Get()->OnUpdateScript(entity.GetUUID(), script_name);
				}
			}
		}

		// Animation Update
		if (!m_IsPaused && (m_IsRunning || m_IsSimulating))
		{
			L_PROFILE_SCOPE("Scene - Animation");

			if (m_LastFrameAnimationUpdateCounter) {
				L_PROFILE_SCOPE("Scene - Animation Update Job Wait");
				m_LastFrameAnimationUpdateCounter->Wait();
				m_LastFrameAnimationUpdateCounter.reset();
			}

			std::shared_ptr<std::unordered_map<UUID, glm::mat4>> updated_bone_transformations; // Copy data

			{
				std::unique_lock<std::mutex> lock(m_BoneUpdateMutex);
				updated_bone_transformations = std::make_shared<std::unordered_map<UUID, glm::mat4>>(m_BoneUpdates);
			}

			// === PHASE 1: Submit final bone transformation jobs (High Priority) ===
			std::vector<std::pair<std::string, JobFunction>> animation_jobs;
			auto animator_view = GetAllEntitiesWith<SkinnedMeshComponent, AnimatorComponent>();

			const uint32_t total_entities = static_cast<uint32_t>(animator_view.size_hint());
			const uint32_t hardware_threads = std::max(1u, std::thread::hardware_concurrency() / 2);

			// Clamp to avoid oversaturation on extreme core counts
			const uint32_t max_allowed_batches = 32;
			const uint32_t target_batches = std::min<uint32_t>(std::min(total_entities, hardware_threads), max_allowed_batches);

			// Dynamic batch size (round up)
			size_t batch_size = target_batches > 0 ? (total_entities + target_batches - 1) / target_batches : total_entities;
			batch_size = std::max<size_t>(1, batch_size); // Always at least 1

			std::vector<entt::entity> current_batch;
			for (const auto& entity_handle : animator_view)
			{
				Entity entity = { entity_handle, this };
				if (!entity) continue;

				auto& animator_component = entity.GetComponent<AnimatorComponent>();

				switch (animator_component.CullingMode)
				{
					case AnimatorComponent::AnimationCullingMode::NoAnimateOffScreenContinueTimer:
					case AnimatorComponent::AnimationCullingMode::NoAnimateOffScreenStopTimer:
					{
						switch (m_SceneConfig.ScenePipelineType)
						{
							case L_RENDER_PIPELINE::FORWARD_PLUS:
							{
								if (auto context = ForwardPlusPipeline::GetSceneContext(this); context && context->Entities_OverallVisible.count(entity.GetUUID()) == 0)
								{
									continue;
								}
								break;
							}
						}
						break;
					}
				}

				current_batch.push_back(entity_handle);

				if (current_batch.size() == batch_size)
				{
					animation_jobs.emplace_back("Animation Transform Updates", [scene_ref = this, batch = std::move(current_batch), updated_bone_transformations]() mutable
						{
							for (const auto& entity_handle : batch)
							{
								Entity entity = { entity_handle, scene_ref };
								if (!entity) continue;

								entity.GetComponent<SkinnedMeshComponent>().ComputeFinalBoneTransformations(*updated_bone_transformations);
							}
						});
					current_batch.clear();
				}
			}

			if (!current_batch.empty())
			{
				animation_jobs.emplace_back("Animation Transform Updates", [scene_ref = this, batch = std::move(current_batch), updated_bone_transformations]() mutable
					{
						for (const auto& entity_handle : batch)
						{
							Entity entity = { entity_handle, scene_ref };
							if (!entity) continue;

							entity.GetComponent<SkinnedMeshComponent>().ComputeFinalBoneTransformations(*updated_bone_transformations);
						}
					});
			}

			JobCounter job_counter = {};
			JobSystem::Get()->SubmitJobs(animation_jobs, &job_counter, JobPriority::High);

			// === PHASE 2: Submit deferred animation step + bone update swap ===
			m_LastFrameAnimationUpdateCounter = std::make_shared<JobCounter>();
			JobSystem::Get()->SubmitJob("Animator Component Update - Job Kick", [&]()
				{
					auto bone_map_mutex = std::make_shared<std::mutex>();
					auto next_updated_bone_transformations = std::make_shared<std::unordered_map<UUID, glm::mat4>>();
					next_updated_bone_transformations->reserve(m_BoneUpdates.size());

					std::vector<std::pair<std::string, JobFunction>> animation_update_jobs;

					auto animator_update_view = GetAllEntitiesWith<SkinnedMeshComponent, AnimatorComponent>();

					const uint32_t total_entities = static_cast<uint32_t>(animator_update_view.size_hint());
					const uint32_t hardware_threads = std::max(1u, std::thread::hardware_concurrency() / 2);

					// Clamp to avoid oversaturation on extreme core counts
					const uint32_t max_allowed_batches = 32;
					const uint32_t target_batches = std::min<uint32_t>(std::min(total_entities, hardware_threads), max_allowed_batches);

					// Dynamic batch size (round up)
					size_t batch_size = target_batches > 0 ? (total_entities + target_batches - 1) / target_batches : total_entities;
					batch_size = std::max<size_t>(1, batch_size); // Always at least 1

					std::vector<entt::entity> current_batch;

					// Shared work lambda
					auto animation_worker = [scene_ref = this, bone_map_mutex, next_updated_bone_transformations](const std::vector<entt::entity>& batch)
						{
							for (const auto& entity_handle : batch)
							{
								Entity entity = { entity_handle, scene_ref };
								if (!entity) continue;

								auto& animator = entity.GetComponent<AnimatorComponent>();

								if (!animator.IsPlaying || animator.CurrentClipIndex == -1)
									continue;

								// Visibility culling
								switch (animator.CullingMode)
								{
									case AnimatorComponent::AnimationCullingMode::NoAnimateOffScreenContinueTimer:
									{
										if (scene_ref->m_SceneConfig.ScenePipelineType == L_RENDER_PIPELINE::FORWARD_PLUS)
										{
											if (auto context = ForwardPlusPipeline::GetSceneContext(scene_ref);
												context && context->Entities_OverallVisible.count(entity.GetUUID()) == 0)
											{
												if (AssetManager::IsAssetLoaded(animator.AnimationClipHandles[animator.CurrentClipIndex]))
												{
													auto clip = AssetManager::GetAsset<AnimationClip>(animator.AnimationClipHandles[animator.CurrentClipIndex]);
													animator.StepAnimationTimer(clip);
												}
												continue;
											}
										}
										break;
									}
									case AnimatorComponent::AnimationCullingMode::NoAnimateOffScreenStopTimer:
									{
										if (scene_ref->m_SceneConfig.ScenePipelineType == L_RENDER_PIPELINE::FORWARD_PLUS)
										{
											if (auto context = ForwardPlusPipeline::GetSceneContext(scene_ref);
												context && context->Entities_OverallVisible.count(entity.GetUUID()) == 0)
												continue;
										}
										break;
									}
									default: break;
								}

								auto result = animator.UpdateDeferred();

								std::lock_guard<std::mutex> lock(*bone_map_mutex);
								next_updated_bone_transformations->insert(result.begin(), result.end());
							}
						};

					// Batch entity processing jobs
					for (const auto& entity_handle : animator_update_view)
					{
						current_batch.push_back(entity_handle);
						if (current_batch.size() == batch_size)
						{
							animation_update_jobs.emplace_back("Animator Component Update - Job Work", [batch = std::move(current_batch), animation_worker]() mutable
								{
									animation_worker(batch);
								});
							current_batch.clear();
						}
					}
					if (!current_batch.empty())
					{
						animation_update_jobs.emplace_back("Animator Component Update - Job Work", [batch = std::move(current_batch), animation_worker]() mutable
							{
								animation_worker(batch);
							});
					}

					JobCounter job_counter = {};
					JobSystem::Get()->SubmitJobs(animation_update_jobs, &job_counter, JobPriority::High, true);
					job_counter.Wait();

					std::unique_lock<std::mutex> lock(m_BoneUpdateMutex);
					m_BoneUpdates.swap(*next_updated_bone_transformations);

				}, m_LastFrameAnimationUpdateCounter.get(), JobPriority::Medium, true);

			job_counter.Wait(); // Wait on Transformation Updates Before Proceeding
		}
	}

	void Scene::OnRuntimeRender()
	{
		L_PROFILE_SCOPE("Scene::OnRuntimeRender - Render Scene Cameras");

		std::vector<std::tuple<FrameBuffer*, glm::vec4, uint8_t>> camera_frame_buffers;

		switch (m_SceneConfig.ScenePipelineType)
		{
		case L_RENDER_PIPELINE::FORWARD:        ForwardPipeline::OnRenderScene(this);			break;
		case L_RENDER_PIPELINE::FORWARD_PLUS:   ForwardPlusPipeline::OnRenderScene(this);     break;
		case L_RENDER_PIPELINE::DEFERRED:       DeferredRenderPipeline::OnRenderScene(this);  break;
		}

		// Get active cameras (sorted from GetActiveCameras)
		std::vector<Entity> active_cameras;
		switch (m_SceneConfig.ScenePipelineType)
		{
		case L_RENDER_PIPELINE::FORWARD:       active_cameras = ForwardPipeline::GetActiveCameras(this);			break;
		case L_RENDER_PIPELINE::FORWARD_PLUS:  active_cameras = ForwardPlusPipeline::GetActiveCameras(this);         break;
		case L_RENDER_PIPELINE::DEFERRED:      active_cameras = DeferredRenderPipeline::GetActiveCameras(this);      break;
		}

		for (const auto& camera_entity : active_cameras)
		{
			if (!camera_entity || !camera_entity.GetComponent<CameraComponent>().DisplayToMainViewport)
				continue;

			auto& camera_component = camera_entity.GetComponent<CameraComponent>();
			camera_frame_buffers.emplace_back(camera_component.CameraFramebuffer.get(), camera_component.GetViewport(), camera_component.CameraDepth);
		}

		std::sort(camera_frame_buffers.begin(), camera_frame_buffers.end(),
			[](const auto& a, const auto& b) {
				uint8_t depthA = std::get<2>(a);
				uint8_t depthB = std::get<2>(b);
				if (depthA != depthB)
					return depthA < depthB; // Lower depth renders first

				// If depths are equal, compare viewport size (larger first)
				const glm::vec4& viewportA = std::get<1>(a);
				const glm::vec4& viewportB = std::get<1>(b);
				float sizeA = viewportA.z * viewportA.w; // Width * Height
				float sizeB = viewportB.z * viewportB.w;
				return sizeA > sizeB; // Larger viewport renders first
			});

		CombineCameraTargets(camera_frame_buffers);
	}

	void Scene::OnEditorRender(EditorCamera* editor_camera, const std::vector<Entity>& scene_cameras)
	{
		L_PROFILE_SCOPE("Scene::OnEditorRender - Render Editor + Scene Cameras");

		std::vector<std::tuple<FrameBuffer*, glm::vec4, uint8_t>> camera_frame_buffers;

		if (editor_camera)
		{
			switch (m_SceneConfig.ScenePipelineType)
			{
			case L_RENDER_PIPELINE::FORWARD:       ForwardPipeline::OnRenderEditorScene(this, scene_cameras, editor_camera);   break;
			case L_RENDER_PIPELINE::FORWARD_PLUS:  ForwardPlusPipeline::OnRenderEditorScene(this, scene_cameras, editor_camera);    break;
			case L_RENDER_PIPELINE::DEFERRED:      DeferredRenderPipeline::OnRenderEditorScene(this, scene_cameras, editor_camera); break;
			}

			// Full screen editor camera - other scene_camera targets can be composited ontop later e.g., if we wanted a preview of a selected camera
			camera_frame_buffers.emplace_back(const_cast<FrameBuffer*>(&editor_camera->GetFrameBuffer()), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0);
		}

		CombineCameraTargets(camera_frame_buffers);
	}

	void Scene::CombineCameraTargets(const std::vector<std::tuple<FrameBuffer*, glm::vec4, uint8_t>>& camera_targets)
	{
		if (!m_SceneFrameBuffer)
			return;

		m_SceneFrameBuffer->Bind();

		Renderer::ClearColour({ 0.0f, 0.0f, 0.0f, 1.0f });
		Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto shader = AssetManager::GetInbuiltShader("Scene_FrameBuffer_Composite");
		if (shader)
		{
			shader->Bind();

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);

			// Preserve order when rendering
			for (const auto& [framebuffer, viewport_dimensions, depth] : camera_targets)
			{
				if (!framebuffer || (viewport_dimensions.z == 0.0f && viewport_dimensions.w == 0.0f))
					continue;

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, framebuffer->GetTexture(FrameBufferTexture::ColourTexture));
				shader->SetInt("u_CameraTexture", 0);

				glViewport(
					static_cast<GLint>(std::ceil(viewport_dimensions.x * m_SceneFrameBuffer->GetConfig().Width)),
					static_cast<GLint>(std::ceil(viewport_dimensions.y * m_SceneFrameBuffer->GetConfig().Height)),
					static_cast<GLint>(std::ceil(viewport_dimensions.z * m_SceneFrameBuffer->GetConfig().Width)),
					static_cast<GLint>(std::ceil(viewport_dimensions.w * m_SceneFrameBuffer->GetConfig().Height))
				);

				DrawCameraTextureToSceneFramebuffer();
			}

			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			shader->UnBind();
		}

		m_SceneFrameBuffer->Unbind();
		m_SceneFrameBuffer->ResolveMultisampledFBO();
	}

	void Scene::OnUpdateGUI() {



	}

	void Scene::OnFixedUpdate() {

		// Scripts - only if running
		if (!m_IsPaused && m_IsRunning)
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Scene - Scripts");

			// Validate and Fixed Update
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity_handle : view)
			{
				Entity entity = { entity_handle, this };
				auto& script_component = view.get<ScriptComponent>(entity_handle);
				for (const auto& [script_name, script_active] : script_component.Scripts)
				{
					// Validate Script Instance
					if (!script_active)
					{
						continue;
					}
					else if (!ScriptManager::Get()->GetScriptClassInstance(entity.GetUUID(), script_name))
					{
						// Script Active in Component, but Script Instance Not Created Yet
						ScriptManager::Get()->OnCreateScript(entity.GetUUID(), script_name);
					}

					// Call Fixed Update
					ScriptManager::Get()->OnFixedUpdateScript(entity.GetUUID(), script_name);
				}
			}
		}

		// Physics
		if (!m_IsPaused && (m_IsRunning || m_IsSimulating))
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Scene - Physics");

			m_IsPhysicsCalculating = true;

			PhysicsSystem::Update(std::static_pointer_cast<Scene>(shared_from_this()));

			// Handle persistent collision triggers as trigger callback is not called when contact is persistent
			for (const auto& pair : CollisionCallback::s_ActiveTriggers) 
			{
				Entity trigger_entity = FindEntityByUUID(pair.first);
				Entity other_entity = FindEntityByUUID(pair.second);

				if (trigger_entity && other_entity && trigger_entity.HasComponent<ScriptComponent>()) 
				{
					auto& script_component = trigger_entity.GetComponent<ScriptComponent>();
					for (const auto& [script_name, script_active] : script_component.Scripts)
					{					
						// Validate Script Instance
						if (!script_active)
						{
							continue;
						}
						else if (!ScriptManager::Get()->GetScriptClassInstance(trigger_entity.GetUUID(), script_name))
						{
							// Script Active in Component, but Script Instance Not Created Yet
							ScriptManager::Get()->OnCreateScript(trigger_entity.GetUUID(), script_name);
						}

						ScriptManager::Get()->OnCollideScript(trigger_entity.GetUUID(), other_entity.GetUUID(), script_name, Script_Collision_Type::TriggerStay);
					}
				}
			}

			m_IsPhysicsCalculating = false;
		}

	}

	void Scene::OnLateUpdate()
	{
		// Scripts - only if running
		if (!m_IsPaused && m_IsRunning)
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Scene - Scripts");

			// Validate and Fixed Update
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity_handle : view)
			{
				Entity entity = { entity_handle, this };
				auto& script_component = view.get<ScriptComponent>(entity_handle);
				for (const auto& [script_name, script_active] : script_component.Scripts)
				{
					// Validate Script Instance
					if (!script_active)
					{
						continue;
					}
					else if (!ScriptManager::Get()->GetScriptClassInstance(entity.GetUUID(), script_name))
					{
						// Script Active in Component, but Script Instance Not Created Yet
						ScriptManager::Get()->OnCreateScript(entity.GetUUID(), script_name);
					}

					// Call Fixed Update
					ScriptManager::Get()->OnLateUpdateScript(entity.GetUUID(), script_name);
				}
			}
		}

	}

	void Scene::OnViewportResize(const glm::ivec2& new_size) {

		glm::ivec2 final_size = new_size;

		if (final_size.x <= 0 || final_size.y <= 0)
		{
			final_size = { final_size.x > 1 ? final_size.x : 1, final_size.y > 1 ? final_size.y : 1 };

			L_CORE_ERROR("Attempted to Resize FrameBuffer to Invalid Size: X({0}), Y({1}).", new_size.x, new_size.y);
			L_CORE_ERROR("Setting New Viewport Size to: X({0}), Y({1}).", final_size.x, final_size.y);
		}

		if (m_SceneFrameBuffer) {

			if (m_SceneFrameBuffer->GetConfig().Width == final_size.x && m_SceneFrameBuffer->GetConfig().Height == final_size.y)
				return;

			// Update Render Pipeline Data
			m_SceneFrameBuffer->Resize(final_size);

			switch (m_SceneConfig.ScenePipelineType)
			{
			case L_RENDER_PIPELINE::FORWARD:		ForwardPipeline::OnViewportResize(this, final_size);	break;
			case L_RENDER_PIPELINE::FORWARD_PLUS:	ForwardPlusPipeline::OnViewportResize(this, final_size);		break;
			case L_RENDER_PIPELINE::DEFERRED:		DeferredRenderPipeline::OnViewportResize(this, final_size);	break;
			}
		}
	}

	void Scene::DrawCameraTextureToSceneFramebuffer()
	{
		static GLuint VAO = -1, VBO = -1, EBO = -1;
		if (VAO == -1)
		{
			float quadVertices[] = {
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  0.0f,  1.0f, 0.0f,
				 1.0f,  1.0f,  0.0f,  1.0f, 1.0f
			};
			GLuint quadIndices[] = { 0, 1, 2, 0, 2, 3 };

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);

			// VBO
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

			// EBO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

			// Position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			// TexCoord attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);

		}

		//L_CORE_ASSERT(m_ScreenQuadVAO, "Scene - Could Not Create Screen Quad Vertex Array."); 
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

	}


#pragma endregion

}