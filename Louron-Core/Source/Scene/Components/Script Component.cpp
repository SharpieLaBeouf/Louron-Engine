#include "Script Component.h"

// Louron Core Headers
#include "../Entity.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <yaml-cpp/yaml.h>

namespace Louron
{

    void ScriptComponent::Serialize(YAML::Emitter& out) const
    {
        if (!GetEntity()) {
            L_CORE_ERROR("Cannot Serialize Script Component - Current Entity Is Invalid!");
            return;
        }
        Entity entity = *GetEntity();

        if (!entity) {
            return;
        }

        out << YAML::Key << "ScriptComponent" << YAML::Value;
        out << YAML::BeginMap;

        int i = 1;
        for (auto& [script_name, active] : Scripts) {

            auto entity_class = ScriptManager::Get()->GetScriptClass(script_name);
            auto entity_fields = ScriptManager::Get()->GetScriptFieldMap(entity.GetUUID(), script_name);
            if (!entity_class || !entity_fields)
            {
                continue;
            }

            out << YAML::Key << "Script_" + std::to_string(i) << YAML::Value;
            out << YAML::BeginMap;

            out << YAML::Key << "Active" << YAML::Value << active;
            out << YAML::Key << "Script Name" << YAML::Value << script_name;

            auto& class_fields = entity_class->fields;
            if (entity_class->field_count > 0) 
            {
                out << YAML::Key << "Fields" << YAML::Value;
                out << YAML::BeginSeq;
                for (const auto& [name, field] : *entity_fields) {


                    out << YAML::BeginMap;

                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << ScriptUtils::FieldTypeToString(field.field.type);

                    switch (field.field.type) 
                    {

                            // ECS Types
                        case ScriptFieldType::Entity:
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
                        case ScriptFieldType::AnimationClip:    out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<uint32_t>();          break;

                        case ScriptFieldType::Float:            out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<float>();             break;
                        case ScriptFieldType::Double:           out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<double>();            break;
                        case ScriptFieldType::Int8:             out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<int8_t>();            break;
                        case ScriptFieldType::Int16:            out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<int16_t>();           break;
                        case ScriptFieldType::Int32:            out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<int32_t>();           break;
                        case ScriptFieldType::Int64:            out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<int64_t>();           break;
                        case ScriptFieldType::UInt8:            out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<uint8_t>();           break;
                        case ScriptFieldType::UInt16:           out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<uint16_t>();          break;
                        case ScriptFieldType::UInt32:           out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<uint32_t>();          break;
                        case ScriptFieldType::UInt64:           out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<uint64_t>();          break;
                        case ScriptFieldType::Bool:             out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetValue<bool>();              break;
                        case ScriptFieldType::CString:          out << YAML::Key << "Data" << YAML::Value << entity_fields->at(name).GetCStringValue();             break;

                        case ScriptFieldType::Vector2:
                        {
                            glm::vec2 v = entity_fields->at(name).GetValue<glm::vec2>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::Vector3:
                        {
                            glm::vec3 v = entity_fields->at(name).GetValue<glm::vec3>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::Vector4:
                        {
                            glm::vec4 v = entity_fields->at(name).GetValue<glm::vec4>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << v.w
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::UVector2:
                        {
                            glm::uvec2 v = entity_fields->at(name).GetValue<glm::uvec2>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::UVector3:
                        {
                            glm::uvec3 v = entity_fields->at(name).GetValue<glm::uvec3>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::UVector4:
                        {
                            glm::uvec4 v = entity_fields->at(name).GetValue<glm::uvec4>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << v.w
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::IVector2:
                        {
                            glm::ivec2 v = entity_fields->at(name).GetValue<glm::ivec2>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::IVector3:
                        {
                            glm::ivec3 v = entity_fields->at(name).GetValue<glm::ivec3>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::IVector4:
                        {
                            glm::ivec4 v = entity_fields->at(name).GetValue<glm::ivec4>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << v.w
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::DVector2:
                        {
                            glm::dvec2 v = entity_fields->at(name).GetValue<glm::dvec2>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::DVector3:
                        {
                            glm::dvec3 v = entity_fields->at(name).GetValue<glm::dvec3>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::DVector4:
                        {
                            glm::dvec4 v = entity_fields->at(name).GetValue<glm::dvec4>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << v.w
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::BVector2:
                        {
                            glm::bvec2 v = entity_fields->at(name).GetValue<glm::bvec2>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::BVector3:
                        {
                            glm::bvec3 v = entity_fields->at(name).GetValue<glm::bvec3>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << YAML::EndSeq;
                            break;
                        }
                        case ScriptFieldType::BVector4:
                        {
                            glm::bvec4 v = entity_fields->at(name).GetValue<glm::bvec4>();
                            out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                                << YAML::BeginSeq
                                << v.x
                                << v.y
                                << v.z
                                << v.w
                                << YAML::EndSeq;
                            break;
                        }


                    }

                    out << YAML::EndMap;


                }
                out << YAML::EndSeq;
            }

            out << YAML::EndMap;

            i++;
        }

        out << YAML::EndMap;
    }

    bool ScriptComponent::Deserialize(const YAML::Node data, UUID entity_uuid)
    {
        for (int i = 1; i < data.size() + 1; i++) {

            std::string index = "Script_" + std::to_string(i);
            YAML::Node script_node = data[index.c_str()];

            if (script_node) 
            {
                bool active = script_node["Active"].as<bool>();
                std::string script_name = script_node["Script Name"].as<std::string>();

                Scripts.push_back({ script_name, active });

                auto field_node = script_node["Fields"];
                if (field_node) 
                {
                    auto entity_class = ScriptManager::Get()->GetScriptClass(script_name);
                    if (entity_class) 
                    {
                        auto script_field_map = ScriptManager::Get()->GetScriptFieldMap(entity_uuid, script_name);

                        L_CORE_ASSERT(script_field_map, "Could Not Create Entry for Entity and Script Fields");

                        for (auto script_field_node : field_node) 
                        {
                            std::string field_name = script_field_node["Name"].as<std::string>();
                            ScriptFieldType type = ScriptUtils::StringToFieldType(script_field_node["Type"].as<std::string>());

                            // Validate Field Exists in Loaded Script Class
                            size_t field_index = -1;
                            for (size_t i = 0; i < entity_class->field_count; ++i)
                            {
                                if (entity_class->fields[i].name == field_name)
                                {
                                    field_index = i;
                                    break;
                                }
                            }

                            if (field_index == -1)
                            {
                                L_CORE_WARN("Field Name ({}) Not Found In Script Class({}).", field_name, script_name);
                                continue;
                            }

                            ScriptFieldInstance& field_instance = (*script_field_map)[field_name];

                            field_instance = entity_class->fields[field_index];
                            switch (type) {
                                // ECS & Asset types (UUID)
                            case ScriptFieldType::Entity:
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
                            case ScriptFieldType::Prefab:
                            case ScriptFieldType::Shader:
                            case ScriptFieldType::ComputeShader:
                            case ScriptFieldType::Material:
                            case ScriptFieldType::Texture2D:
                            case ScriptFieldType::TextureCubeMap:
                            case ScriptFieldType::StaticMesh:
                            case ScriptFieldType::AudioClip:
                            case ScriptFieldType::Skeleton:
                            case ScriptFieldType::AnimationClip: field_instance.SetValue(script_field_node["Data"].as<uint32_t>());                 break;

                            case ScriptFieldType::Float:    field_instance.SetValue(script_field_node["Data"].as<float>());                         break;
                            case ScriptFieldType::Double:   field_instance.SetValue(script_field_node["Data"].as<double>());                        break;
                            case ScriptFieldType::Int8:     field_instance.SetValue(script_field_node["Data"].as<int8_t>());                        break;
                            case ScriptFieldType::Int16:    field_instance.SetValue(script_field_node["Data"].as<int16_t>());                       break;
                            case ScriptFieldType::Int32:    field_instance.SetValue(script_field_node["Data"].as<int32_t>());                       break;
                            case ScriptFieldType::Int64:    field_instance.SetValue(script_field_node["Data"].as<int64_t>());                       break;
                            case ScriptFieldType::UInt8:    field_instance.SetValue(script_field_node["Data"].as<uint8_t>());                       break;
                            case ScriptFieldType::UInt16:   field_instance.SetValue(script_field_node["Data"].as<uint16_t>());                      break;
                            case ScriptFieldType::UInt32:   field_instance.SetValue(script_field_node["Data"].as<uint32_t>());                      break;
                            case ScriptFieldType::UInt64:   field_instance.SetValue(script_field_node["Data"].as<uint64_t>());                      break;
                            case ScriptFieldType::Bool:     field_instance.SetValue(script_field_node["Data"].as<bool>());                          break;
                            case ScriptFieldType::CString:  field_instance.SetCStringValue(script_field_node["Data"].as<std::string>().c_str());    break;

                            // Vectors
                            case ScriptFieldType::Vector2:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 2)
                                {
                                    glm::vec2 v;
                                    v.x = data_node[0].as<float>();
                                    v.y = data_node[1].as<float>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::Vector3:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 3)
                                {
                                    glm::vec3 v;
                                    v.x = data_node[0].as<float>();
                                    v.y = data_node[1].as<float>();
                                    v.z = data_node[2].as<float>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::Vector4:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 4)
                                {
                                    glm::vec4 v;
                                    v.x = data_node[0].as<float>();
                                    v.y = data_node[1].as<float>();
                                    v.z = data_node[2].as<float>();
                                    v.w = data_node[3].as<float>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::UVector2:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 2)
                                {
                                    glm::uvec2 v;
                                    v.x = data_node[0].as<uint32_t>();
                                    v.y = data_node[1].as<uint32_t>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::UVector3:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 3)
                                {
                                    glm::uvec3 v;
                                    v.x = data_node[0].as<uint32_t>();
                                    v.y = data_node[1].as<uint32_t>();
                                    v.z = data_node[2].as<uint32_t>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::UVector4:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 4)
                                {
                                    glm::uvec4 v;
                                    v.x = data_node[0].as<uint32_t>();
                                    v.y = data_node[1].as<uint32_t>();
                                    v.z = data_node[2].as<uint32_t>();
                                    v.w = data_node[3].as<uint32_t>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::IVector2:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 2)
                                {
                                    glm::ivec2 v;
                                    v.x = data_node[0].as<int32_t>();
                                    v.y = data_node[1].as<int32_t>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::IVector3:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 3)
                                {
                                    glm::ivec3 v;
                                    v.x = data_node[0].as<int32_t>();
                                    v.y = data_node[1].as<int32_t>();
                                    v.z = data_node[2].as<int32_t>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::IVector4:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 4)
                                {
                                    glm::ivec4 v;
                                    v.x = data_node[0].as<int32_t>();
                                    v.y = data_node[1].as<int32_t>();
                                    v.z = data_node[2].as<int32_t>();
                                    v.w = data_node[3].as<int32_t>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::DVector2:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 2)
                                {
                                    glm::dvec2 v;
                                    v.x = data_node[0].as<double>();
                                    v.y = data_node[1].as<double>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::DVector3:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 3)
                                {
                                    glm::dvec3 v;
                                    v.x = data_node[0].as<double>();
                                    v.y = data_node[1].as<double>();
                                    v.z = data_node[2].as<double>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::DVector4:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 4)
                                {
                                    glm::dvec4 v;
                                    v.x = data_node[0].as<double>();
                                    v.y = data_node[1].as<double>();
                                    v.z = data_node[2].as<double>();
                                    v.w = data_node[3].as<double>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::BVector2:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 2)
                                {
                                    glm::bvec2 v;
                                    v.x = data_node[0].as<bool>();
                                    v.y = data_node[1].as<bool>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::BVector3:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 3)
                                {
                                    glm::bvec3 v;
                                    v.x = data_node[0].as<bool>();
                                    v.y = data_node[1].as<bool>();
                                    v.z = data_node[2].as<bool>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::BVector4:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 4)
                                {
                                    glm::bvec4 v;
                                    v.x = data_node[0].as<bool>();
                                    v.y = data_node[1].as<bool>();
                                    v.z = data_node[2].as<bool>();
                                    v.w = data_node[3].as<bool>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }

                            // Mats
                            case ScriptFieldType::Mat3:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 9)
                                {
                                    glm::mat3 m;
                                    for (int j = 0; j < 9; ++j)
                                        reinterpret_cast<float*>(&m)[j] = data_node[j].as<float>();
                                    field_instance.SetValue(m);
                                }
                                break;
                            }
                            case ScriptFieldType::Mat4:
                            {
                                const auto& data_node = script_field_node["Data"];
                                if (data_node && data_node.IsSequence() && data_node.size() == 16)
                                {
                                    glm::mat4 m;
                                    for (int j = 0; j < 16; ++j)
                                        reinterpret_cast<float*>(&m)[j] = data_node[j].as<float>();
                                    field_instance.SetValue(m);
                                }
                                break;
                            }

                            case ScriptFieldType::Unknown:
                                L_CORE_WARN("Attempting to deserialize unknown field type!");
                                break;
                            }


                        }

                    }

                }

            }
        }
        return true;
    }

}