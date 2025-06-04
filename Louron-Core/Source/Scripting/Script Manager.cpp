#include "Script Manager.h"

// Louron Core Headers
#include "Script Register.h"

#include "../Debug/Assert.h"
#include "../Scene/Entity.h"
#include "../Project/Project.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron
{
	ScriptManager* ScriptManager::s_Instance = nullptr;

    ScriptManager* ScriptManager::Get()
    {
        return s_Instance;
    }

    bool ScriptManager::Init(const std::filesystem::path& path)
    {
        if (!s_Instance) s_Instance = new ScriptManager();
        L_CORE_ASSERT(s_Instance, "Script Manager Instance Failed to Initialise.");

        return (s_Instance && s_Instance->ReloadAssembly(path));
    }

    void ScriptManager::Shutdown()
    {
        if (!s_Instance) return;

        s_Instance->FreeAssembly();
        delete s_Instance;
        s_Instance = nullptr;
    }

    void ScriptManager::RemoveAllScriptInstances()
    {
        for (auto it = m_ScriptInstances.begin(); it != m_ScriptInstances.end(); )
        {
            if (it->second && it->second->GetInstanceRawPointer())
            {
                it->second->GetInstanceRawPointer()->OnDestroy();
            }
            it = m_ScriptInstances.erase(it);
        }

        m_ScriptInstances.clear(); // Double Check We're Cleared
    }

    void ScriptManager::RemoveAllScriptFields()
    {
        m_ScriptFieldInstances.clear();
    }

    bool ScriptManager::ReloadAssembly(const std::filesystem::path& path)
    {
        FreeAssembly();
        return LoadAssembly(path);
    }

    std::shared_ptr<ScriptClass> ScriptManager::GetScriptClass(const std::string& name)
    {
        if (!m_Assembly)
        {
            L_CORE_ERROR("Script Assembly Not Loaded.");
            return nullptr;
        }

        auto it = m_ScriptClasses.find(name);
        if (it != m_ScriptClasses.end())
            return it->second;
        return nullptr;
    }

    ScriptClassInstance* ScriptManager::GetScriptClassInstance(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        auto it = m_ScriptInstances.find(key);
        if (it != m_ScriptInstances.end())
            return it->second.get();
        return nullptr;
    }

    ScriptFieldInstance* ScriptManager::GetScriptFieldInstance(uint32_t entity_uuid, const std::string& script_name, const std::string& field_name)
    {
        // 1. Validate Script Class
        auto klass = GetScriptClass(script_name);
        if (!klass)
        {
            L_CORE_ERROR("Script Class Does Not Exist");
            return nullptr;
        }

        // 2. Validate Field Exists
        int field_index = -1;
        for (int i = 0; i < klass->field_count; ++i)
        {
            if (klass->fields[i].name == field_name)
            {
                field_index = i;
                break;
            }
        }

        if (field_index == -1)
        {
            L_CORE_ERROR("Script Field Does Not Exist in Class");
            return nullptr;
        }

        // 3. Validate Script Field Map
        std::string key = std::to_string(entity_uuid) + script_name;
        if (m_ScriptFieldInstances.find(key) == m_ScriptFieldInstances.end())
        {
            m_ScriptFieldInstances[key] = {}; // Create Script Field Map
            m_ScriptFieldInstances[key][field_name] = ScriptFieldInstance(klass->fields[field_index]); // Insert Field into Field Map
        }

        // 4. Return Pointer to ScriptFieldIsntance
        return &m_ScriptFieldInstances[key][field_name];
    }

    ScriptManager::ScriptFieldMap* ScriptManager::GetScriptFieldMap(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        if (m_ScriptFieldInstances.find(key) == m_ScriptFieldInstances.end())
        {
            m_ScriptFieldInstances[key] = {};
        }
        return &m_ScriptFieldInstances[key];
    }

    void ScriptManager::AddDummyScriptClass(const std::string& script_name)
    {
        m_ScriptClasses[script_name] = nullptr; // Just add a dummy script to the class list
    }

    bool ScriptManager::ScriptClassExists(const std::string& script_name)
    {
        return m_ScriptClasses.find(script_name) != m_ScriptClasses.end();
    }

    void ScriptManager::OnCreateScript(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        if (m_ScriptInstances.find(key) != m_ScriptInstances.end())
            return;

        if (!CreateInstance(entity_uuid, script_name))
        {
            L_CORE_ERROR("Failed to Create Script Instance for Entity: {}, {}", script_name, std::to_string(entity_uuid));
            return;
        }

        ScriptFieldMap& map = m_ScriptFieldInstances[key];
        for (auto& [field_name, field_instance] : map)
        {
            switch (field_instance.field.type)
            {
                default:
                {
                    const void* raw_buffer = field_instance.GetRawBuffer();
                    m_ScriptInstances[key]->SetFieldRaw(field_name, raw_buffer, ScriptUtils::GetFieldSize(field_instance.field.type));

                    break;
                }

                case ScriptFieldType::CString:
                {
                    const char* string = field_instance.GetCStringValue();
                    m_ScriptInstances[key]->SetFieldValue<const char*>(field_name, string);

                    break;
                }

                // ECS Types
                case ScriptFieldType::Entity:
                {
                    uint32_t field_entity_uuid = *reinterpret_cast<const uint32_t*>(field_instance.GetRawBuffer());

                    if (auto ref = m_ScriptInstances[key]->m_Class.lock())
                    {
                        for (int i = 0; i < ref->field_count; ++i)
                        {
                            if (ref->fields[i].name == field_name)
                            {
                                IScript_Entity* entity = reinterpret_cast<IScript_Entity*>(
                                    (uint8_t*)m_ScriptInstances[key]->m_InstancePtr + ref->fields[i].offset
                                );

                                entity->m_EntityID = field_entity_uuid;
                            }
                        }
                    }

                    break;
                }

                case ScriptFieldType::IDComponent:
                case ScriptFieldType::TagComponent:
                case ScriptFieldType::HierarchyComponent:
                case ScriptFieldType::ScriptComponent:
                case ScriptFieldType::TransformComponent:

                case ScriptFieldType::CameraComponent:

                case ScriptFieldType::AudioListenerComponent:
                case ScriptFieldType::AudioEmitterComponent:

                case ScriptFieldType::MeshFilterComponent:
                case ScriptFieldType::MeshRendererComponent:
                case ScriptFieldType::LODMeshComponent:

                case ScriptFieldType::SkinnedMeshComponent:
                case ScriptFieldType::BasicAnimationComponent:
                case ScriptFieldType::AnimatorComponent:

                case ScriptFieldType::SkyboxComponent:

                case ScriptFieldType::PointLightComponent:
                case ScriptFieldType::SpotLightComponent:
                case ScriptFieldType::DirectionalLightComponent:

                case ScriptFieldType::RigidbodyComponent:
                case ScriptFieldType::BoxColliderComponent:
                case ScriptFieldType::SphereColliderComponent:

                case ScriptFieldType::Component:
                {
                    uint32_t component_entity_uuid = *reinterpret_cast<const uint32_t*>(field_instance.GetRawBuffer());

                    if (auto ref = m_ScriptInstances[key]->m_Class.lock())
                    {
                        for (int i = 0; i < ref->field_count; ++i)
                        {
                            if (ref->fields[i].name == field_name)
                            {
                                IComponent* component = reinterpret_cast<IComponent*>(
                                    (uint8_t*)m_ScriptInstances[key]->m_InstancePtr + ref->fields[i].offset
                                );

                                component->m_EntityID = component_entity_uuid;
                            }
                        }
                    }

                    break;
                }

                // Asset Types

                case ScriptFieldType::Prefab:
                case ScriptFieldType::Shader:
                case ScriptFieldType::ComputeShader:
                case ScriptFieldType::Material:
                case ScriptFieldType::Texture2D:
                case ScriptFieldType::TextureCubeMap:
                case ScriptFieldType::StaticMesh:
                case ScriptFieldType::AudioClip:
                case ScriptFieldType::Skeleton:
                case ScriptFieldType::AnimationClip:
                case ScriptFieldType::StateMachine:
                case ScriptFieldType::Humanoid:
                case ScriptFieldType::HumanoidMask:
                {
                    uint32_t asset_handle = *reinterpret_cast<const uint32_t*>(field_instance.GetRawBuffer());

                    if (auto ref = m_ScriptInstances[key]->m_Class.lock())
                    {
                        for (int i = 0; i < ref->field_count; ++i)
                        {
                            if (ref->fields[i].name == field_name)
                            {
                                IAsset* asset = reinterpret_cast<IAsset*>(
                                    (uint8_t*)m_ScriptInstances[key]->m_InstancePtr + ref->fields[i].offset
                                );

                                asset->SetAssetHandle(asset_handle);
                            }
                        }
                    }
                    break;
                }
            }
        }

        if (m_ScriptInstances[key] && m_ScriptInstances[key]->GetInstanceRawPointer())
        {
            m_ScriptInstances[key]->GetInstanceRawPointer()->m_EntityID = entity_uuid;

            try
            {
                m_ScriptInstances[key]->GetInstanceRawPointer()->OnCreate();
            }
            catch (const std::exception& e)
            {
                L_SCRIPT_ERROR("Script OnCreate Error: {}, {}, {}", script_name, std::to_string(entity_uuid), e.what());
            }
        }
    }

    void ScriptManager::OnDestroyScript(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        auto it = m_ScriptInstances.find(key);
        if (it == m_ScriptInstances.end() || !it->second || !it->second->GetInstanceRawPointer())
            return;

        try
        {
            it->second->GetInstanceRawPointer()->OnDestroy();
        }
        catch (const std::exception& e)
        {
            L_SCRIPT_ERROR("Script OnDestroy Error: {}, {}, {}", script_name, std::to_string(entity_uuid), e.what());
        }
        m_ScriptInstances.erase(it);
    }

    void ScriptManager::OnDestroyAllScripts(uint32_t entity_uuid)
    {
        auto scene_ref = Project::GetActiveScene();
        if (!scene_ref)
            return;

        Entity entity = scene_ref->FindEntityByUUID(entity_uuid);
        if (!entity || !entity.HasComponent<ScriptComponent>())
            return;

        auto& script_component = entity.GetComponent<ScriptComponent>();
        for (const auto& [script_name, script_active] : script_component.Scripts)
        {
            OnDestroyScript(entity_uuid, script_name);
        }
    }

    void ScriptManager::OnUpdateScript(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        auto it = m_ScriptInstances.find(key);
        if (it == m_ScriptInstances.end() || !it->second || !it->second->GetInstanceRawPointer())
            return;

        try
        {
            it->second->GetInstanceRawPointer()->OnUpdate();
        }
        catch (const std::exception& e)
        {
            L_SCRIPT_ERROR("Script OnUpdate Error: {}, {}, {}", script_name, std::to_string(entity_uuid), e.what());
        }
    }

    void ScriptManager::OnLateUpdateScript(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        auto it = m_ScriptInstances.find(key);
        if (it == m_ScriptInstances.end() || !it->second || !it->second->GetInstanceRawPointer())
            return;

        try
        {
            it->second->GetInstanceRawPointer()->OnLateUpdate();
        }
        catch (const std::exception& e)
        {
            L_SCRIPT_ERROR("Script OnLateUpdate Error: {}, {}, {}", script_name, std::to_string(entity_uuid), e.what());
        }
    }

    void ScriptManager::OnFixedUpdateScript(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        auto it = m_ScriptInstances.find(key);
        if (it == m_ScriptInstances.end() || !it->second || !it->second->GetInstanceRawPointer())
            return;

        try
        {
            it->second->GetInstanceRawPointer()->OnFixedUpdate();
        }
        catch (const std::exception& e)
        {
            L_SCRIPT_ERROR("Script OnFixedUpdate Error: {}, {}, {}", script_name, std::to_string(entity_uuid), e.what());
        }
    }

    void ScriptManager::OnCollideScript(uint32_t entity_uuid, uint32_t other_uuid, const std::string& script_name, Script_Collision_Type collision_type)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        auto it = m_ScriptInstances.find(key);
        if (it == m_ScriptInstances.end() || !it->second || !it->second->GetInstanceRawPointer())
            return;

        try
        {
            Script_Collider other_collider{};
            other_collider.other_collider_uuid = IScript_Entity{};
            other_collider.other_collider_uuid.m_EntityID = other_uuid;

            if (Project::GetActiveScene()->FindEntityByUUID(other_uuid).HasComponent<BoxColliderComponent>())
                other_collider.type = Script_Collider_Type::Box_Collider;

            if (Project::GetActiveScene()->FindEntityByUUID(other_uuid).HasComponent<SphereColliderComponent>())
                other_collider.type = Script_Collider_Type::Sphere_Collider;

            // TODO
            //if (other_entity.HasComponent<CapsuleCollider>())
            //	other_collider.type = _Collider_Type::Capsule_Collider;
            //if (other_entity.HasComponent<MeshCollider>())
            //	other_collider.type = _Collider_Type::Mesh_Collider;

            switch (collision_type) {

                // COLLIDE
            case Script_Collision_Type::CollideEnter: {
                it->second->GetInstanceRawPointer()->OnCollideEnter(other_collider);
                break;
            }

            case Script_Collision_Type::CollideStay: {
                it->second->GetInstanceRawPointer()->OnCollideStay(other_collider);
                break;
            }

            case Script_Collision_Type::CollideLeave: {
                it->second->GetInstanceRawPointer()->OnCollideLeave(other_collider);
                break;
            }

                                              // TRIGGER
            case Script_Collision_Type::TriggerEnter: {
                it->second->GetInstanceRawPointer()->OnTriggerEnter(other_collider);
                break;
            }

            case Script_Collision_Type::TriggerStay: {
                it->second->GetInstanceRawPointer()->OnTriggerStay(other_collider);
                break;
            }

            case Script_Collision_Type::TriggerLeave: {
                it->second->GetInstanceRawPointer()->OnTriggerLeave(other_collider);
                break;
            }

            case Script_Collision_Type::Unknown:
            default: L_CORE_WARN("Invalid Collision Type!");
            }
        }
        catch (const std::exception& e)
        {
            L_SCRIPT_ERROR("Script OnFixedUpdate Error: {}, {}, {}", script_name, std::to_string(entity_uuid), e.what());
        }
    }

    bool ScriptManager::LoadAssembly(const std::filesystem::path& path)
    {
    
    #if defined(L_PLATFORM_WINDOWS)

        m_Assembly = LoadLibraryA(path.string().c_str());

    #elif defined(L_PLATFORM_LINUX)

        m_Assembly = dlopen(path.string().c_str(), RTLD_NOW);

    #endif

        if (!m_Assembly)
        {
            L_CORE_ERROR("Failed to Load Assembly: {}", path.string());
            return false;
        }

    #if defined(L_PLATFORM_WINDOWS)

        m_LoadScripts = (void(*)())GetProcAddress(m_Assembly, "LoadScripts");
        m_GetScriptTypes = (const ScriptClass * (*)(size_t*))GetProcAddress(m_Assembly, "GetScriptTypes");

    #elif defined(L_PLATFORM_LINUX)

        m_LoadScripts = reinterpret_cast<void(*)()>(dlsym(m_Assembly, "LoadScripts"));
        m_GetScriptTypes = reinterpret_cast<const ScriptClass* (*)(size_t*)>(dlsym(m_Assembly, "GetScriptTypes"));
    
    #endif

        if (!ScriptRegister::RegisterAll(m_Assembly))
            return false;

        if (!m_LoadScripts || !m_GetScriptTypes)
        {
            L_CORE_ERROR("Missing Expected Exports in Assembly");
            return false;
        }

        // Load Script Reflection Data
        m_LoadScripts();

        // Retrieve Scripts and Field Information
        size_t scripts_count = 0;
        const ScriptClass* script_classes = m_GetScriptTypes(&scripts_count);
        for (size_t i = 0; i < scripts_count; ++i)
        {
            auto script_class = std::make_shared<ScriptClass>(script_classes[i]);
            m_ScriptClasses[script_classes[i].name] = script_class;

            // Initialise w/ Default Values - Create Default Instance, and Retrieve Default Values
            auto temp_instance = std::make_unique<ScriptClassInstance>(script_class, -1);

            uint8_t buffer[64]; // Max Field Size 64 BYTES -> Mat4
            memset(buffer, 0, sizeof(buffer)); // Zero out scratch buf
            auto& script_field_map = m_ScriptFieldInstances[std::to_string(NULL_UUID) + std::string(script_class->name)];
            for (int j = 0; j < script_class->field_count; ++j)
            {
                auto script_field = script_class->fields[j];
                auto& script_field_instance = script_field_map[script_field.name];
                script_field_instance.field = script_field;

                if (script_field_instance.field.type == ScriptFieldType::CString)
                {
                    const char* string = "";
                    temp_instance->GetFieldValue(script_field.name, string);
                    script_field_instance.SetCStringValue(string);
                }
                else
                {
                    memset(buffer, 0, sizeof(buffer)); // Zero out scratch buf
                    temp_instance->GetFieldValue(script_field.name, buffer);
                    script_field_instance.SetValue(buffer, ScriptUtils::GetFieldSize(script_field.type));
                }
            }

            // Free Script Instance
            temp_instance.reset();
        }

        return true;
    }

    void ScriptManager::FreeAssembly()
    {
        if (!m_Assembly) return;

        m_ScriptClasses.clear();
        m_ScriptInstances.clear();

        m_LoadScripts = nullptr;
        m_GetScriptTypes = nullptr;

    #if defined(L_PLATFORM_WINDOWS)

        FreeLibrary(m_Assembly);

    #elif defined(L_PLATFORM_LINUX)
    
        dlclose(m_Assembly);

    #endif

        m_Assembly = nullptr;
    }

    bool ScriptManager::CreateInstance(uint32_t entity_uuid, const std::string& script_name)
    {
        std::string key = std::to_string(entity_uuid) + script_name;
        if (m_ScriptInstances.find(key) != m_ScriptInstances.end())
        {
            std::cerr << "Script Instance already exists for entity: " << entity_uuid << " and script: " << script_name << "\n";
            return false;
        }

        auto klass = GetScriptClass(script_name);
        if (!klass)
        {
            std::cerr << "ScriptClass not found: " << script_name << "\n";
            return false;
        }

        m_ScriptInstances[key] = std::make_unique<ScriptClassInstance>(klass, entity_uuid);

        if(m_ScriptFieldInstances.find(key) == m_ScriptFieldInstances.end())
            m_ScriptFieldInstances[key] = {};

        L_CORE_ASSERT(m_ScriptInstances[key] && m_ScriptInstances[key]->GetInstanceRawPointer(), "Could Not Create Instance of Script Class");
        return true;
    }

}
