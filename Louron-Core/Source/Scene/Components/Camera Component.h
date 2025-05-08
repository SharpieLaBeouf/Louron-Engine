#pragma once

// Louron Core Headers
#include "Component Base.h"

#include "../../OpenGL/Framebuffer.h"

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

        // TODO: Add bool Active = true;

        std::shared_ptr<SceneCamera> CameraInstance = nullptr;

        bool Primary = false;

        CameraClearFlags ClearFlags = CameraClearFlags::COLOUR_ONLY;
        glm::vec4 ClearColour = { 0.1764f, 0.3294f, 0.5607f, 1.0f };

        /// <summary>
        /// Depth of Camera
        /// 
        /// Higher the depth, means this camera will be rendered ontop of other camera's in final result
        /// </summary>
        uint8_t CameraDepth = 0;

        /// <summary>
        /// Should the final image of this camera texture be rendered to the scene's viewport?
        /// </summary>
        bool DisplayToMainViewport = true;

        const glm::vec4& GetViewport() const { return m_CameraViewport; }
        void SetViewport(const glm::vec4& viewport_dimensions, const glm::uvec2& viewport_size);

        /// <summary>
        /// FrameBuffer the Camera Will Render Into
        /// </summary>
        std::unique_ptr<FrameBuffer> CameraFramebuffer = nullptr;

        CameraComponent();
        CameraComponent(const CameraComponent& other);
        CameraComponent(CameraComponent&& other) noexcept;
        CameraComponent& operator=(const CameraComponent& other);
        CameraComponent& operator=(CameraComponent&& other) noexcept;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

    private:

        /// <summary>
        /// X = Pos X
        /// Y = Pos Y
        /// Z = Width
        /// W = Height
        /// 
        /// All Values Are Normalised Between 0.0 -> 1.0f.
        /// </summary>
        glm::vec4 m_CameraViewport = { 0.0f, 0.0f, 1.0f, 1.0f };

    };

}