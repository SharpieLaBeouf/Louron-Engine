#include "Script Defines.h"

namespace Louron 
{

    ScriptFieldInstance::ScriptFieldInstance()
    {
        memset(m_Buffer, 0, sizeof(m_Buffer));
    }

    ScriptFieldInstance::ScriptFieldInstance(const ScriptField& script_field) : field(script_field)
    {
        memset(m_Buffer, 0, sizeof(m_Buffer));
    }

    ScriptClassInstance::ScriptClassInstance(std::shared_ptr<ScriptClass> klass, uint32_t entity_uuid)
        : m_Class(klass), m_EntityID(entity_uuid)
    {
        m_InstancePtr = (IScript*)klass->factory_function(m_EntityID);
    }

    ScriptClassInstance::~ScriptClassInstance()
    {
        if(auto klass = m_Class.lock(); klass)
            klass->release_function(m_EntityID);
        m_InstancePtr = nullptr;
    }

    IScript* ScriptClassInstance::GetInstanceRawPointer()
    {
        return m_InstancePtr;
    }

    std::weak_ptr<ScriptClass> ScriptClassInstance::GetScriptClass() const
    {
        return m_Class;
    }

    bool ScriptClassInstance::SetFieldRaw(const std::string& field_name, const void* data, size_t size)
    {
        if (auto ref = m_Class.lock())
        {
            for (int i = 0; i < ref->field_count; ++i)
            {
                if (ref->fields[i].name == field_name)
                {
                    std::memcpy((uint8_t*)m_InstancePtr + ref->fields[i].offset, data, size);
                    return true;
                }
            }
        }
        return false;
    }

    namespace ScriptUtils
    {
        std::string FieldTypeToString(ScriptFieldType type)
        {
            switch (type)
            {
                case ScriptFieldType::Float: return "Float";
                case ScriptFieldType::Double: return "Double";
                case ScriptFieldType::Int8: return "Int8";
                case ScriptFieldType::Int16: return "Int16";
                case ScriptFieldType::Int32: return "Int32";
                case ScriptFieldType::Int64: return "Int64";
                case ScriptFieldType::UInt8: return "UInt8";
                case ScriptFieldType::UInt16: return "UInt16";
                case ScriptFieldType::UInt32: return "UInt32";
                case ScriptFieldType::UInt64: return "UInt64";
                case ScriptFieldType::Bool: return "Bool";
                case ScriptFieldType::CString: return "CString";

                case ScriptFieldType::Vector2: return "Vector2";
                case ScriptFieldType::Vector3: return "Vector3";
                case ScriptFieldType::Vector4: return "Vector4";
                case ScriptFieldType::UVector2: return "UVector2";
                case ScriptFieldType::UVector3: return "UVector3";
                case ScriptFieldType::UVector4: return "UVector4";
                case ScriptFieldType::IVector2: return "IVector2";
                case ScriptFieldType::IVector3: return "IVector3";
                case ScriptFieldType::IVector4: return "IVector4";
                case ScriptFieldType::DVector2: return "DVector2";
                case ScriptFieldType::DVector3: return "DVector3";
                case ScriptFieldType::DVector4: return "DVector4";
                case ScriptFieldType::BVector2: return "BVector2";
                case ScriptFieldType::BVector3: return "BVector3";
                case ScriptFieldType::BVector4: return "BVector4";

                case ScriptFieldType::Mat3: return "Mat3";
                case ScriptFieldType::Mat4: return "Mat4";

                case ScriptFieldType::Entity: return "Entity";

                case ScriptFieldType::IDComponent: return "IDComponent";
                case ScriptFieldType::TagComponent: return "TagComponent";
                case ScriptFieldType::HierarchyComponent: return "HierarchyComponent";
                case ScriptFieldType::ScriptComponent: return "ScriptComponent";
                case ScriptFieldType::TransformComponent: return "TransformComponent";
                case ScriptFieldType::CameraComponent: return "CameraComponent";
                case ScriptFieldType::AudioListenerComponent: return "AudioListenerComponent";
                case ScriptFieldType::AudioEmitterComponent: return "AudioEmitterComponent";
                case ScriptFieldType::MeshFilterComponent: return "MeshFilterComponent";
                case ScriptFieldType::MeshRendererComponent: return "MeshRendererComponent";
                case ScriptFieldType::LODMeshComponent: return "LODMeshComponent";
                case ScriptFieldType::SkinnedMeshComponent: return "SkinnedMeshComponent";
                case ScriptFieldType::BasicAnimationComponent: return "BasicAnimationComponent";
                case ScriptFieldType::AnimatorComponent: return "AnimatorComponent";
                case ScriptFieldType::SkyboxComponent: return "SkyboxComponent";
                case ScriptFieldType::PointLightComponent: return "PointLightComponent";
                case ScriptFieldType::SpotLightComponent: return "SpotLightComponent";
                case ScriptFieldType::DirectionalLightComponent: return "DirectionalLightComponent";
                case ScriptFieldType::RigidbodyComponent: return "RigidbodyComponent";
                case ScriptFieldType::BoxColliderComponent: return "BoxColliderComponent";
                case ScriptFieldType::SphereColliderComponent: return "SphereColliderComponent";
                case ScriptFieldType::Component: return "Component";

                case ScriptFieldType::Prefab: return "Prefab";
                case ScriptFieldType::Shader: return "Shader";
                case ScriptFieldType::ComputeShader: return "ComputeShader";
                case ScriptFieldType::Material: return "Material";
                case ScriptFieldType::Texture2D: return "Texture2D";
                case ScriptFieldType::TextureCubeMap: return "TextureCubeMap";
                case ScriptFieldType::StaticMesh: return "StaticMesh";
                case ScriptFieldType::AudioClip: return "AudioClip";
                case ScriptFieldType::Skeleton: return "Skeleton";
                case ScriptFieldType::AnimationClip: return "AnimationClip";

                case ScriptFieldType::StateMachine: return "StateMachine";

                case ScriptFieldType::Humanoid: return "Humanoid";
                case ScriptFieldType::HumanoidMask: return "HumanoidMask";

                case ScriptFieldType::Unknown: return "Unknown";
                default: return "Unknown";
            }
        }

        ScriptFieldType StringToFieldType(const std::string& type)
        {
            #define MATCH_TYPE(str, enumVal) if (type == str) return ScriptFieldType::enumVal

            MATCH_TYPE("Float", Float);
            MATCH_TYPE("Double", Double);
            MATCH_TYPE("Int8", Int8);
            MATCH_TYPE("Int16", Int16);
            MATCH_TYPE("Int32", Int32);
            MATCH_TYPE("Int64", Int64);
            MATCH_TYPE("UInt8", UInt8);
            MATCH_TYPE("UInt16", UInt16);
            MATCH_TYPE("UInt32", UInt32);
            MATCH_TYPE("UInt64", UInt64);
            MATCH_TYPE("Bool", Bool);
            MATCH_TYPE("CString", CString);

            MATCH_TYPE("Vector2", Vector2);
            MATCH_TYPE("Vector3", Vector3);
            MATCH_TYPE("Vector4", Vector4);
            MATCH_TYPE("UVector2", UVector2);
            MATCH_TYPE("UVector3", UVector3);
            MATCH_TYPE("UVector4", UVector4);
            MATCH_TYPE("IVector2", IVector2);
            MATCH_TYPE("IVector3", IVector3);
            MATCH_TYPE("IVector4", IVector4);
            MATCH_TYPE("DVector2", DVector2);
            MATCH_TYPE("DVector3", DVector3);
            MATCH_TYPE("DVector4", DVector4);
            MATCH_TYPE("BVector2", BVector2);
            MATCH_TYPE("BVector3", BVector3);
            MATCH_TYPE("BVector4", BVector4);

            MATCH_TYPE("Mat3", Mat3);
            MATCH_TYPE("Mat4", Mat4);

            MATCH_TYPE("Entity", Entity);

            MATCH_TYPE("IDComponent", IDComponent);
            MATCH_TYPE("TagComponent", TagComponent);
            MATCH_TYPE("HierarchyComponent", HierarchyComponent);
            MATCH_TYPE("ScriptComponent", ScriptComponent);
            MATCH_TYPE("TransformComponent", TransformComponent);
            MATCH_TYPE("CameraComponent", CameraComponent);
            MATCH_TYPE("AudioListenerComponent", AudioListenerComponent);
            MATCH_TYPE("AudioEmitterComponent", AudioEmitterComponent);
            MATCH_TYPE("MeshFilterComponent", MeshFilterComponent);
            MATCH_TYPE("MeshRendererComponent", MeshRendererComponent);
            MATCH_TYPE("LODMeshComponent", LODMeshComponent);
            MATCH_TYPE("SkinnedMeshComponent", SkinnedMeshComponent);
            MATCH_TYPE("BasicAnimationComponent", BasicAnimationComponent);
            MATCH_TYPE("AnimatorComponent", AnimatorComponent);
            MATCH_TYPE("SkyboxComponent", SkyboxComponent);
            MATCH_TYPE("PointLightComponent", PointLightComponent);
            MATCH_TYPE("SpotLightComponent", SpotLightComponent);
            MATCH_TYPE("DirectionalLightComponent", DirectionalLightComponent);
            MATCH_TYPE("RigidbodyComponent", RigidbodyComponent);
            MATCH_TYPE("BoxColliderComponent", BoxColliderComponent);
            MATCH_TYPE("SphereColliderComponent", SphereColliderComponent);
            MATCH_TYPE("Component", Component);

            MATCH_TYPE("Prefab", Prefab);
            MATCH_TYPE("Shader", Shader);
            MATCH_TYPE("ComputeShader", ComputeShader);
            MATCH_TYPE("Material", Material);
            MATCH_TYPE("Texture2D", Texture2D);
            MATCH_TYPE("TextureCubeMap", TextureCubeMap);
            MATCH_TYPE("StaticMesh", StaticMesh);
            MATCH_TYPE("AudioClip", AudioClip);
            MATCH_TYPE("Skeleton", Skeleton);
            MATCH_TYPE("AnimationClip", AnimationClip);

            MATCH_TYPE("StateMachine", StateMachine);
            MATCH_TYPE("Humanoid", Humanoid);
            MATCH_TYPE("HumanoidMask", HumanoidMask);

            #undef MATCH_TYPE

            return ScriptFieldType::Unknown;
        }

        size_t GetFieldSize(ScriptFieldType type)
        {
            switch (type)
            {
                case ScriptFieldType::Float:    return sizeof(float);
                case ScriptFieldType::Double:   return sizeof(double);
                case ScriptFieldType::Int8:     return sizeof(int8_t);
                case ScriptFieldType::Int16:    return sizeof(int16_t);
                case ScriptFieldType::Int32:    return sizeof(int32_t);
                case ScriptFieldType::Int64:    return sizeof(int64_t);
                case ScriptFieldType::UInt8:    return sizeof(uint8_t);
                case ScriptFieldType::UInt16:   return sizeof(uint16_t);
                case ScriptFieldType::UInt32:   return sizeof(uint32_t);
                case ScriptFieldType::UInt64:   return sizeof(uint64_t);
                case ScriptFieldType::Bool:     return sizeof(bool);
                case ScriptFieldType::CString:  return sizeof(const char*);

                    // Vectors
                case ScriptFieldType::Vector2: return sizeof(float) * 2;
                case ScriptFieldType::Vector3: return sizeof(float) * 3;
                case ScriptFieldType::Vector4: return sizeof(float) * 4;
                case ScriptFieldType::UVector2: return sizeof(uint32_t) * 2;
                case ScriptFieldType::UVector3: return sizeof(uint32_t) * 3;
                case ScriptFieldType::UVector4: return sizeof(uint32_t) * 4;
                case ScriptFieldType::IVector2: return sizeof(int32_t) * 2;
                case ScriptFieldType::IVector3: return sizeof(int32_t) * 3;
                case ScriptFieldType::IVector4: return sizeof(int32_t) * 4;
                case ScriptFieldType::DVector2: return sizeof(double) * 2;
                case ScriptFieldType::DVector3: return sizeof(double) * 3;
                case ScriptFieldType::DVector4: return sizeof(double) * 4;
                case ScriptFieldType::BVector2: return sizeof(bool) * 2;
                case ScriptFieldType::BVector3: return sizeof(bool) * 3;
                case ScriptFieldType::BVector4: return sizeof(bool) * 4;

                case ScriptFieldType::Mat3: return sizeof(float) * 9;
                case ScriptFieldType::Mat4: return sizeof(float) * 16;

                    // ECS and Asset types are always the size of uint32_t
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
                case ScriptFieldType::AnimationClip:
                case ScriptFieldType::StateMachine:
                case ScriptFieldType::Humanoid:
                case ScriptFieldType::HumanoidMask:
                    return sizeof(uint32_t);

                default: return 0;
            }
        }


    }

}
