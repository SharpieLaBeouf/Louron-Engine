#pragma once

// Louron Core Headers
#include "Component Base.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace Louron
{
    enum class CameraClearFlags : uint8_t {
        COLOUR_ONLY = 0,
        SKYBOX = 1
    };

    class SceneCamera;
    struct CameraComponent : public ComponentBase {

        std::shared_ptr<SceneCamera> CameraInstance = nullptr;

        bool Primary = false;

        CameraClearFlags ClearFlags = CameraClearFlags::COLOUR_ONLY;
        glm::vec4 ClearColour = { 0.1764f, 0.3294f, 0.5607f, 1.0f };

        CameraComponent() = default;
        CameraComponent(const CameraComponent& other);
        CameraComponent(CameraComponent&& other) noexcept;
        CameraComponent& operator=(const CameraComponent& other);
        CameraComponent& operator=(CameraComponent&& other) noexcept;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

    };

}