#include "Script Component.h"

// Louron Core Headers
#include "../Entity.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
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

            out << YAML::Key << "Script_" + std::to_string(i) << YAML::Value;
            out << YAML::BeginMap;

            out << YAML::Key << "Active" << YAML::Value << active;
            out << YAML::Key << "Script Name" << YAML::Value << script_name;

            auto entity_class = ScriptManager::GetEntityClass(script_name);

            if (!entity_class)
                continue;

            auto& class_fields = entity_class->GetFields();
            if (class_fields.size() > 0) {

                auto& entity_fields = ScriptManager::GetScriptFieldMap(entity.GetUUID(), script_name);

                out << YAML::Key << "Fields" << YAML::Value;
                out << YAML::BeginSeq;
                for (const auto& [name, field] : entity_fields) {


                    out << YAML::BeginMap;

                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << ScriptingUtils::ScriptFieldTypeToString(field.Field.Type);

                    switch (field.Field.Type) {

                    case ScriptFieldType::Entity:
                    case ScriptFieldType::TransformComponent:
                    case ScriptFieldType::TagComponent:
                    case ScriptFieldType::ScriptComponent:
                    case ScriptFieldType::PointLightComponent:
                    case ScriptFieldType::SpotLightComponent:
                    case ScriptFieldType::DirectionalLightComponent:
                    case ScriptFieldType::RigidbodyComponent:
                    case ScriptFieldType::BoxColliderComponent:
                    case ScriptFieldType::SphereColliderComponent:
                    case ScriptFieldType::Component:
                    case ScriptFieldType::ComputeShader:
                    case ScriptFieldType::Prefab:   out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<UUID>();           break;

                    case ScriptFieldType::Bool:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<bool>();           break;
                    case ScriptFieldType::Byte:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<uint16_t>();       break; // Have to get data as uint16_t and not uint8_t because uint8_t is unsigned char which serialises as an alphanumeric character
                    case ScriptFieldType::Sbyte:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int8_t>();         break;
                    case ScriptFieldType::Char:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<char>();           break;
                    case ScriptFieldType::Decimal:  out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<double>();         break;
                    case ScriptFieldType::Double:   out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<double>();         break;
                    case ScriptFieldType::Float:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<float>();          break;
                    case ScriptFieldType::Int:      out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int>();            break;
                    case ScriptFieldType::Uint:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<unsigned int>();   break;
                    case ScriptFieldType::Long:     out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int64_t>();        break;
                    case ScriptFieldType::Ulong:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<uint64_t>();       break;
                    case ScriptFieldType::Short:    out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<int16_t>();        break;
                    case ScriptFieldType::Ushort:   out << YAML::Key << "Data" << YAML::Value << entity_fields.at(name).GetValue<uint16_t>();       break;

                    case ScriptFieldType::Vector2:
                    {
                        glm::vec2 v = entity_fields.at(name).GetValue<glm::vec2>();
                        out << YAML::Key << "Data" << YAML::Value << YAML::Flow
                            << YAML::BeginSeq
                            << v.x
                            << v.y
                            << YAML::EndSeq;
                        break;
                    }
                    case ScriptFieldType::Vector3:
                    {
                        glm::vec3 v = entity_fields.at(name).GetValue<glm::vec3>();
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
                        glm::vec4 v = entity_fields.at(name).GetValue<glm::vec4>();
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

            if (script_node) {

                bool active = script_node["Active"].as<bool>();
                std::string script_name = script_node["Script Name"].as<std::string>();

                Scripts.push_back({ script_name, active });

                auto field_node = script_node["Fields"];
                if (field_node) {

                    auto entity_class = ScriptManager::GetEntityClass(script_name);
                    if (entity_class) {

                        const auto& fields = entity_class->GetFields();
                        auto& entity_fields = ScriptManager::GetScriptFieldMap(entity_uuid, script_name);

                        for (auto script_field : field_node) {

                            std::string field_name = script_field["Name"].as<std::string>();
                            ScriptFieldType type = ScriptingUtils::ScriptFieldTypeFromString(script_field["Type"].as<std::string>());

                            ScriptFieldInstance& field_instance = entity_fields[field_name];

                            if (fields.find(field_name) == fields.end())
                            {
                                L_CORE_WARN("Field Name ({}) Not Found In Script Class({}).", field_name, script_name);
                                continue;
                            }

                            field_instance.Field = fields.at(field_name);

                            switch (type) {

                            case ScriptFieldType::Entity:
                            case ScriptFieldType::TransformComponent:
                            case ScriptFieldType::TagComponent:
                            case ScriptFieldType::ScriptComponent:
                            case ScriptFieldType::PointLightComponent:
                            case ScriptFieldType::SpotLightComponent:
                            case ScriptFieldType::DirectionalLightComponent:
                            case ScriptFieldType::RigidbodyComponent:
                            case ScriptFieldType::BoxColliderComponent:
                            case ScriptFieldType::SphereColliderComponent:
                            case ScriptFieldType::ComputeShader:
                            case ScriptFieldType::Component:
                            case ScriptFieldType::Prefab: { Louron::UUID data = script_field["Data"].as<uint32_t>(); field_instance.SetValue(data); break; }

                            case ScriptFieldType::Bool: { bool data = script_field["Data"].as<bool>(); field_instance.SetValue(data);                 break; }
                            case ScriptFieldType::Byte: { uint8_t data = script_field["Data"].as<uint8_t>(); field_instance.SetValue(data);           break; }
                            case ScriptFieldType::Sbyte: { int8_t data = script_field["Data"].as<int8_t>(); field_instance.SetValue(data);             break; }
                            case ScriptFieldType::Char: { char data = script_field["Data"].as<char>(); field_instance.SetValue(data);                 break; }
                            case ScriptFieldType::Decimal: { double data = script_field["Data"].as<double>(); field_instance.SetValue(data);             break; }
                            case ScriptFieldType::Double: { double data = script_field["Data"].as<double>(); field_instance.SetValue(data);             break; }
                            case ScriptFieldType::Float: { float data = script_field["Data"].as<float>(); field_instance.SetValue(data);               break; }
                            case ScriptFieldType::Int: { int data = script_field["Data"].as<int>(); field_instance.SetValue(data);                   break; }
                            case ScriptFieldType::Uint: { unsigned int data = script_field["Data"].as<unsigned int>(); field_instance.SetValue(data); break; }
                            case ScriptFieldType::Long: { int64_t data = script_field["Data"].as<int64_t>(); field_instance.SetValue(data);           break; }
                            case ScriptFieldType::Ulong: { uint64_t data = script_field["Data"].as<uint64_t>(); field_instance.SetValue(data);         break; }
                            case ScriptFieldType::Short: { int16_t data = script_field["Data"].as<int16_t>(); field_instance.SetValue(data);           break; }
                            case ScriptFieldType::Ushort: { uint16_t data = script_field["Data"].as<uint16_t>(); field_instance.SetValue(data);         break; }

                            case ScriptFieldType::Vector2:
                            {
                                if (script_field["Data"].IsSequence() && script_field["Data"].size() == 2)
                                {
                                    glm::vec2 v;
                                    v.x = script_field["Data"][0].as<float>();
                                    v.y = script_field["Data"][1].as<float>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::Vector3:
                            {
                                if (script_field["Data"].IsSequence() && script_field["Data"].size() == 3)
                                {
                                    glm::vec3 v;
                                    v.x = script_field["Data"][0].as<float>();
                                    v.y = script_field["Data"][1].as<float>();
                                    v.z = script_field["Data"][2].as<float>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            case ScriptFieldType::Vector4:
                            {
                                if (script_field["Data"].IsSequence() && script_field["Data"].size() == 4)
                                {
                                    glm::vec4 v;
                                    v.x = script_field["Data"][0].as<float>();
                                    v.y = script_field["Data"][1].as<float>();
                                    v.z = script_field["Data"][2].as<float>();
                                    v.w = script_field["Data"][3].as<float>();
                                    field_instance.SetValue(v);
                                }
                                break;
                            }
                            }

                        }

                    }

                }

            }
        }
        return true;
    }

}