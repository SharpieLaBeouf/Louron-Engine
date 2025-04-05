#pragma once

// Louron Core Headers
#include "Component Base.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Louron
{

    struct IDComponent : public ComponentBase
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(UUID uuid) : ID(uuid) {}
        IDComponent(const IDComponent&) = default;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);
    };

    struct TagComponent : public ComponentBase
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& name) : Tag(name) {}

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

        void SetUniqueName(const std::string& name);
    };

    struct HierarchyComponent : public ComponentBase 
    {

    public:

        HierarchyComponent() = default;
        HierarchyComponent(const HierarchyComponent& other);
        HierarchyComponent(HierarchyComponent&& other) noexcept;

        HierarchyComponent& operator=(const HierarchyComponent& other);
        HierarchyComponent& operator=(HierarchyComponent&& other) noexcept;

        void AttachParent(const UUID& newParentID);
        void DetachParent();

        void DetachChildren();
        void RehomeChildren(const UUID& newParentID);

        Entity FindChild(const UUID& childUUID) const;
        Entity FindChild(const std::string& childName) const;
        const std::vector<UUID>& GetChildren() const;

        Entity GetParentEntity() const;
        const UUID& GetParentID() const;
        bool HasParent() const;
        bool HasChildren() const;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

        Entity GetRootParentEntity() const;

    private:

        UUID m_Parent = NULL_UUID;
        std::vector<UUID> m_Children;

        // This is for the editor hierarchy panel ordering
        uint32_t m_HierarchyOrderIndex = -1;

        friend class Prefab;
        friend class ModelImporter;
    };

    /// <summary>
    /// Flags that are set to determine what state changes have occured
    /// each frame so the TransformSystem may process these changes.
    /// </summary>
    enum TransformFlags : uint8_t {

        TransformFlag_None = 0,

        TransformFlag_PropertiesUpdated = 1U << 0,  // Only add this flag where there have been changes made to the transform proprties
        TransformFlag_GlobalTransformUpdated = 1U << 1,  // Only add this flag where there have been changes made to a parent in the hierarchy
    };

    struct TransformComponent : public ComponentBase {

    private:

        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::vec3 m_Rotation = glm::vec3(0.0f);
        glm::vec3 m_Scale = glm::vec3(1.0f);

        glm::mat4 m_LocalTransform = glm::mat4(1.0f);
        glm::mat4 m_GlobalTransform = glm::mat4(1.0f);

        TransformFlags m_StateFlags = TransformFlag_None;

        void OnTransformUpdated();

    public:

        TransformComponent();
        TransformComponent(const TransformComponent& other);
        TransformComponent(TransformComponent&& other) noexcept;
        TransformComponent(const glm::vec3& translation);

        TransformComponent& operator=(const TransformComponent& other);
        TransformComponent& operator=(TransformComponent&& other) noexcept;

        // FLAGS
        void AddFlag(TransformFlags flag);
        void RemoveFlag(TransformFlags flag);
        bool CheckFlag(TransformFlags flag) const;
        bool NoFlagsSet() const;
        void ClearFlags();
        TransformFlags GetFlags() const;

        void SetPosition(const glm::vec3& newPosition);
        void SetGlobalPosition(const glm::vec3& newPosition);
        void SetPositionX(const float& newXPosition);
        void SetPositionY(const float& newYPosition);
        void SetPositionZ(const float& newZPosition);

        void SetRotation(const glm::vec3& newRotation);
        void SetGlobalRotation(const glm::vec3& newRotation);
        void SetRotationX(const float& newXRotation);
        void SetRotationY(const float& newYRotation);
        void SetRotationZ(const float& newZRotation);

        void SetScale(const glm::vec3& newScale);
        void SetGlobalScale(const glm::vec3& newScale);
        void SetScaleX(const float& newXScale);
        void SetScaleY(const float& newYScale);
        void SetScaleZ(const float& newZScale);

        void Translate(const glm::vec3& vector);
        void TranslateX(const float& deltaTranslationX);
        void TranslateY(const float& deltaTranslationY);
        void TranslateZ(const float& deltaTranslationZ);

        void Rotate(const glm::vec3& vector);
        void RotateX(const float& deltaRotationX);
        void RotateY(const float& deltaRotationY);
        void RotateZ(const float& deltaRotationZ);

        void Scale(const glm::vec3& vector);
        void ScaleX(const float& deltaScaleX);
        void ScaleY(const float& deltaScaleY);
        void ScaleZ(const float& deltaScaleZ);

        glm::vec3 GetGlobalPosition();
        glm::vec3 GetGlobalRotation();
        glm::vec3 GetGlobalScale();

        void SetForwardDirection(const glm::vec3& direction);
        void SetRightDirection(const glm::vec3& direction);
        void SetUpDirection(const glm::vec3& direction);

        glm::vec3 GetForwardDirection();
        glm::vec3 GetRightDirection();
        glm::vec3 GetUpDirection();

        const glm::vec3& GetLocalPosition() const;
        const glm::vec3& GetLocalRotation() const;
        const glm::vec3& GetLocalScale() const;

        const glm::mat4& GetGlobalTransform();
        const glm::mat4& GetLocalTransform(bool update_local_transform = true);

        void SetTransform(const glm::mat4& transform);

        operator const glm::mat4()&;
        glm::mat4 operator*(const TransformComponent& other) const;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

        // Helper function, converts transform matrix to PURE rotation matrix
        static glm::vec3 GetPositionFromMatrix(const glm::mat4& transform);
        static glm::vec3 GetRotationFromMatrix(const glm::mat4& transform);
        static glm::vec3 GetScaleFromMatrix(const glm::mat4& transform);

        void UpdateLocalTransformMatrix();
    
    private:

        friend class Scene;
        friend class SceneSerializer;
        friend class ScriptRegister;
        friend class TransformSystem;
        friend class PhysicsSystem;

        friend struct HierarchyComponent;

    };

}