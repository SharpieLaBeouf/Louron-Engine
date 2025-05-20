#pragma once

#include "../Core/UUID.h"

#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstring>

namespace Louron
{

    enum class ScriptFieldType : uint8_t
    {

        // Inbuilt Types
        Float,
        Double,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Bool,
        CString,

        // Custom Types
        Vector2,
        Vector3,
        Vector4,
        UVector2,
        UVector3,
        UVector4,
        IVector2,
        IVector3,
        IVector4,
        DVector2,
        DVector3,
        DVector4,
        BVector2,
        BVector3,
        BVector4,

        Mat3,
        Mat4,

        // ECS Types
        Entity,

        IDComponent,
        TagComponent,
        HierarchyComponent,
        ScriptComponent,
        TransformComponent,

        CameraComponent,

        AudioListenerComponent,
        AudioEmitterComponent,

        MeshFilterComponent,
        MeshRendererComponent,
        LODMeshComponent,

        SkinnedMeshComponent,
        BasicAnimationComponent,
        AnimatorComponent,

        SkyboxComponent,

        PointLightComponent,
        SpotLightComponent,
        DirectionalLightComponent,

        RigidbodyComponent,
        BoxColliderComponent,
        SphereColliderComponent,

        Component,

        // Asset Types
        Prefab,
        Shader,
        ComputeShader,
        Material,
        Texture2D,
        TextureCubeMap,
        StaticMesh,
        AudioClip,
        Skeleton,
        AnimationClip,
        StateMachine,

        // Unknown
        Unknown
    };

    struct ScriptField
    {
        const char* name = "";
        size_t offset = 0;
        ScriptFieldType type = ScriptFieldType::Unknown;
    };

    struct ScriptClass
    {
        const char* name;
        int field_count;
        const ScriptField* fields;
        void*(*factory_function)(uint32_t);
        void(*release_function)(uint32_t);
    };

    #define MAX_SCRIPT_CSTRING_SIZE 256
    struct ScriptFieldInstance
    {
        ScriptField field;

        ScriptFieldInstance();
        ScriptFieldInstance(const ScriptField& field);

        template<typename T>
        T GetValue()
        {
            static_assert(std::is_trivially_copyable_v<T>, "GetFieldValue only supports trivially copyable types.");
            return *(T*)m_Buffer;
        }

        template<typename T>
        void SetValue(T value)
        {
            static_assert(std::is_trivially_copyable_v<T>, "GetFieldValue only supports trivially copyable types.");
            memcpy(m_Buffer, &value, sizeof(T));
        }

        void SetValue(const void* value, size_t size)
        {
            memcpy(m_Buffer, value, size);
        }

        // Special Case <3
        const char* GetCStringValue() const
        {
            return m_StringValue.c_str();
        }

        void SetCStringValue(const std::string& value)
        {
            m_StringValue = value;
        }

        const void* GetRawBuffer() const { return m_Buffer; }

    private:
        uint8_t m_Buffer[64]{};

        // Special Case <3
        std::string m_StringValue;

        friend class ScriptManager;
    };

    class IAsset
    {

    public:

        IAsset() = default;
        virtual ~IAsset() = default;

        virtual uint8_t GetAssetType() const { return 0; }
        virtual operator uint32_t() const { return m_AssetHandle; }

    protected:

        void SetAssetHandle(uint32_t asset_handle) { m_AssetHandle = asset_handle; }

        uint32_t m_AssetHandle = NULL_UUID;

        friend class ScriptManager;
    };

    class IComponent
    {

    public:

        IComponent() : m_EntityID(NULL_UUID) { }

        static uint8_t GetType() { return 0; }

    protected:

        IComponent(uint32_t entity_uuid) : m_EntityID(entity_uuid) {}
        void SetEntity(uint32_t entity_uuid) { m_EntityID = entity_uuid; }

        uint32_t m_EntityID;

        friend class ScriptManager;
    };

    class IScript_Entity 
    {
    public:

        IScript_Entity() = default;
        virtual ~IScript_Entity() = default;

    protected: 
        uint32_t m_EntityID = NULL_UUID;

        friend class ScriptManager;
    };

    enum class Script_Collider_Type : uint8_t {
        Unknown = 0,
        Box_Collider,
        Sphere_Collider,
        Capsule_Collider,
        Mesh_Collider
    };

    enum class Script_Collision_Type : uint8_t {
        Unknown = 0,
        CollideEnter,
        CollideStay,
        CollideLeave,
        TriggerEnter,
        TriggerStay,
        TriggerLeave
    };

    struct Script_Collider {
        Script_Collider_Type type;
        IScript_Entity other_collider_uuid;
    };

    class IScript : public IScript_Entity
    {

    public:

        virtual void OnCreate() {}
        virtual void OnDestroy() {}

        virtual void OnUpdate() {}
        virtual void OnLateUpdate() {}
        virtual void OnFixedUpdate() {}

        virtual void OnCollideEnter(const Script_Collider& other) {};
        virtual void OnCollideStay(const Script_Collider& other) {};
        virtual void OnCollideLeave(const Script_Collider& other) {};

        virtual void OnTriggerEnter(const Script_Collider& other) {};
        virtual void OnTriggerStay(const Script_Collider& other) {};
        virtual void OnTriggerLeave(const Script_Collider& other) {};

        const uint32_t& GetEntityID() const { return m_EntityID; }

        friend class ScriptManager;
    };

    class ScriptClassInstance
    {

    public:

        ScriptClassInstance() = default;
        ScriptClassInstance(std::shared_ptr<ScriptClass> klass, uint32_t entity_uuid);
        ~ScriptClassInstance();

        ScriptClassInstance(const ScriptClassInstance&) = default;
        ScriptClassInstance(ScriptClassInstance&&) noexcept = default;
        ScriptClassInstance& operator=(const ScriptClassInstance&) = default;
        ScriptClassInstance& operator=(ScriptClassInstance&&) noexcept = default;

        IScript* GetInstanceRawPointer();

        std::weak_ptr<ScriptClass> GetScriptClass() const;

        template<typename T>
        bool GetFieldValue(const std::string& field_name, T& out)
        {
            if (auto ref = m_Class.lock())
            {
                for (int i = 0; i < ref->field_count; ++i)
                {
                    if (ref->fields[i].name == field_name)
                    {
                        std::memcpy(&out, (uint8_t*)m_InstancePtr + ref->fields[i].offset, sizeof(T));
                        return true;
                    }
                }
            }
            return false;
        }

        template<typename T>
        bool SetFieldValue(const std::string& field_name, const T& value, size_t additional_offset = 0)
        {
            if (auto ref = m_Class.lock())
            {
                for (int i = 0; i < ref->field_count; ++i)
                {
                    if (ref->fields[i].name == field_name)
                    {
                        std::memcpy((uint8_t*)m_InstancePtr + ref->fields[i].offset + additional_offset, &value, sizeof(T));
                        return true;
                    }
                }
            }
            return false;
        }

    private:

        bool SetFieldRaw(const std::string& field_name, const void* data, size_t size);

        std::weak_ptr<ScriptClass> m_Class;
        IScript* m_InstancePtr;
        uint32_t m_EntityID = NULL_UUID;

        friend class ScriptManager;
    };

    namespace ScriptUtils
    {
        std::string FieldTypeToString(ScriptFieldType type);
        ScriptFieldType StringToFieldType(const std::string& type);
        size_t GetFieldSize(ScriptFieldType type);
    }
}
