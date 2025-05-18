#include "Camera Component.h"

// Louron Core Headers
#include "../Entity.h"
#include "../../Renderer/Camera.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <yaml-cpp/yaml.h>

namespace Louron
{

    void CameraComponent::SetViewport(const glm::vec4& viewport_dimensions, const glm::uvec2& viewport_size)
    {
        m_CameraViewport = viewport_dimensions;

        glm::uvec2 new_size = { std::max<uint32_t>(static_cast<uint32_t>(viewport_size.x * m_CameraViewport.z), 1), std::max<uint32_t>(static_cast<uint32_t>(viewport_size.y * m_CameraViewport.w), 1) };


        if (CameraFramebuffer)
        {
            CameraFramebuffer->Resize(new_size);
        }
        if (CameraInstance)
        {
            CameraInstance->SetViewportSize(new_size.x, new_size.y);
        }
    }

    CameraComponent::CameraComponent()
    {
        FrameBufferConfig config = {};
        config.Width = 800;
        config.Height = 600;
        config.RenderToScreen = false;
        config.Samples = 4;

        CameraInstance = std::make_shared<SceneCamera>();
        CameraFramebuffer = std::make_unique<FrameBuffer>(config);
    }

    CameraComponent::CameraComponent(const CameraComponent& other)
    {
        if (other.GetEntity())
            SetEntity(*other.GetEntity());

        this->Primary = other.Primary;
        this->ClearFlags = other.ClearFlags;
        this->ClearColour = other.ClearColour;

        this->CameraInstance = std::make_shared<SceneCamera>(*other.CameraInstance);

        this->m_CameraViewport = other.m_CameraViewport;
        this->CameraDepth = CameraDepth;
        this->CameraFramebuffer = std::make_unique<FrameBuffer>(other.CameraFramebuffer->GetConfig());
        this->DisplayToMainViewport = other.DisplayToMainViewport;

    }

    CameraComponent::CameraComponent(CameraComponent&& other) noexcept
    {
        if (other.GetEntity())
            SetEntity(*other.GetEntity()); 
        other.SetEntity({});

        this->Primary = other.Primary; other.Primary = false;
        this->ClearFlags = other.ClearFlags; other.ClearFlags = CameraClearFlags::COLOUR_ONLY;
        this->ClearColour = other.ClearColour; other.ClearColour = { 0.1764f, 0.3294f, 0.5607f, 1.0f };
        this->CameraInstance = std::move(other.CameraInstance); other.CameraInstance = nullptr;

        this->m_CameraViewport = other.m_CameraViewport; other.m_CameraViewport = { 0.0f, 0.0f, 1.0f, 1.0f };
        this->CameraDepth = CameraDepth; other.CameraDepth = 0;
        this->CameraFramebuffer = std::move(other.CameraFramebuffer); other.CameraFramebuffer = nullptr;
        this->DisplayToMainViewport = other.DisplayToMainViewport; other.DisplayToMainViewport = true;
    }

    CameraComponent& CameraComponent::operator=(const CameraComponent& other)
    {
        if (this == &other)
            return *this;

        if (other.GetEntity())
            SetEntity(*other.GetEntity());

        this->Primary = other.Primary;
        this->ClearFlags = other.ClearFlags;
        this->ClearColour = other.ClearColour;

        this->CameraInstance = std::make_shared<SceneCamera>(*other.CameraInstance);

        this->m_CameraViewport = other.m_CameraViewport;
        this->CameraDepth = CameraDepth;
        this->CameraFramebuffer = std::make_unique<FrameBuffer>(other.CameraFramebuffer->GetConfig());
        this->DisplayToMainViewport = other.DisplayToMainViewport;

        return *this;
    }

    CameraComponent& CameraComponent::operator=(CameraComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        if (other.GetEntity())
            SetEntity(*other.GetEntity());
        other.SetEntity({});

        this->Primary = other.Primary; other.Primary = false;
        this->ClearFlags = other.ClearFlags; other.ClearFlags = CameraClearFlags::COLOUR_ONLY;
        this->ClearColour = other.ClearColour; other.ClearColour = { 0.1764f, 0.3294f, 0.5607f, 1.0f };
        this->CameraInstance = std::move(other.CameraInstance); other.CameraInstance = nullptr;

        this->m_CameraViewport = other.m_CameraViewport; other.m_CameraViewport = { 0.0f, 0.0f, 1.0f, 1.0f };
        this->CameraDepth = CameraDepth; other.CameraDepth = 0;
        this->CameraFramebuffer = std::move(other.CameraFramebuffer); other.CameraFramebuffer = nullptr;
        this->DisplayToMainViewport = other.DisplayToMainViewport; other.DisplayToMainViewport = true;

        return *this;
    }

    void CameraComponent::Serialize(YAML::Emitter& out)
    {
        if (CameraInstance) {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            {

                out << YAML::Key << "Camera" << YAML::Value;
                out << YAML::BeginMap;
                {
                    out << YAML::Key << "Projection Type" << YAML::Value << ((CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Perspective) ? "Perspective" : "Orthographic");

                    switch(CameraInstance->GetProjectionType())
                    {
                        case SceneCamera::ProjectionType::Perspective:
                        {
                            out << YAML::Key << "FOV" << YAML::Value << glm::degrees(CameraInstance->GetPerspectiveVerticalFOV());
                            out << YAML::Key << "Near" << YAML::Value << CameraInstance->GetPerspectiveNearClip();
                            out << YAML::Key << "Far" << YAML::Value << CameraInstance->GetPerspectiveFarClip();
                            break;
                        }
                        case SceneCamera::ProjectionType::Orthographic:
                        {
                            out << YAML::Key << "FOV" << YAML::Value << CameraInstance->GetOrthographicSize();
                            out << YAML::Key << "Near" << YAML::Value << CameraInstance->GetOrthographicNearClip();
                            out << YAML::Key << "Far" << YAML::Value << CameraInstance->GetOrthographicFarClip();

                            break;
                        }
                    }
                }
                out << YAML::EndMap;
            }

            out << YAML::Key << "Primary" << YAML::Value << Primary;
            out << YAML::Key << "DisplayToMainViewport" << YAML::Value << DisplayToMainViewport;

            const char* clear_string = (ClearFlags == CameraClearFlags::SKYBOX) ? "Skybox" : "Colour";
            out << YAML::Key << "ClearFlag" << YAML::Value << clear_string;

            glm::vec4 v = ClearColour;
            out << YAML::Key << "ClearColour" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << v.r
                << v.g
                << v.b
                << v.a
                << YAML::EndSeq;

            v = m_CameraViewport;
            out << YAML::Key << "CameraViewport" << YAML::Value << YAML::Flow
                << YAML::BeginSeq
                << v.x
                << v.y
                << v.z
                << v.w
                << YAML::EndSeq;

            out << YAML::EndMap;

            out << YAML::Key << "CameraDepth" << YAML::Value << CameraDepth;
        }
    }

    bool CameraComponent::Deserialize(const YAML::Node data) {

        YAML::Node component = data;

        if (component["Camera"]) {
            YAML::Node cameraNode = component["Camera"];

            if (cameraNode["Projection Type"] && cameraNode["Projection Type"].as<std::string>() == "Perspective")
            {
                CameraInstance->SetProjectionType(SceneCamera::ProjectionType::Perspective);

                if (cameraNode["FOV"]) {
                    CameraInstance->SetPerspectiveVerticalFOV(glm::radians(cameraNode["FOV"].as<float>()));
                }

                if (cameraNode["Near"]) {
                    CameraInstance->SetPerspectiveNearClip(cameraNode["Near"].as<float>());
                }

                if (cameraNode["Far"]) {
                    CameraInstance->SetPerspectiveFarClip(cameraNode["Far"].as<float>());
                }

            }
            else if (cameraNode["Projection Type"] && cameraNode["Projection Type"].as<std::string>() == "Orthographic")
            {
                CameraInstance->SetProjectionType(SceneCamera::ProjectionType::Orthographic);

                if (cameraNode["FOV"]) {
                    CameraInstance->SetOrthographicSize(cameraNode["FOV"].as<float>());
                }

                if (cameraNode["Near"]) {
                    CameraInstance->SetOrthographicNearClip(cameraNode["Near"].as<float>());
                }

                if (cameraNode["Far"]) {
                    CameraInstance->SetOrthographicFarClip(cameraNode["Far"].as<float>());
                }

            }
        }

        if (component["Primary"]) {
            Primary = component["Primary"].as<bool>();
        }
        if (component["ClearFlag"]) {
            std::string clearFlagStr = component["ClearFlag"].as<std::string>();
            if (clearFlagStr == "Skybox") {
                ClearFlags = CameraClearFlags::SKYBOX;
            }
            else if (clearFlagStr == "Colour") {
                ClearFlags = CameraClearFlags::COLOUR_ONLY;
            }
            else {
                return false;
            }
        }

        if (component["ClearColour"]) {
            auto clearColourSeq = component["ClearColour"];
            if (clearColourSeq.IsSequence() && clearColourSeq.size() == 4) {
                ClearColour.r = clearColourSeq[0].as<float>();
                ClearColour.g = clearColourSeq[1].as<float>();
                ClearColour.b = clearColourSeq[2].as<float>();
                ClearColour.a = clearColourSeq[3].as<float>();
            }
        }

        if (component["CameraViewport"]) {
            auto viewPortSeq = component["CameraViewport"];
            if (viewPortSeq.IsSequence() && viewPortSeq.size() == 4) {
                m_CameraViewport.x = viewPortSeq[0].as<float>();
                m_CameraViewport.y = viewPortSeq[1].as<float>();
                m_CameraViewport.z = viewPortSeq[2].as<float>();
                m_CameraViewport.w = viewPortSeq[3].as<float>();
            }
        }

        if (component["DisplayToMainViewport"]) {
            DisplayToMainViewport = component["DisplayToMainViewport"].as<bool>();
        }

        if (component["CameraDepth"]) {
            CameraDepth = component["CameraDepth"].as<uint8_t>();
        }

        return true;
    }

}