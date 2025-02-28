#include "Camera Component.h"

// Louron Core Headers
#include "../Entity.h"
#include "../../Renderer/Camera.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron
{
    CameraComponent::CameraComponent(const CameraComponent& other) 
    {
        if (other.GetEntity())
            SetEntity(*other.GetEntity());

        this->Primary = other.Primary;
        this->ClearFlags = other.ClearFlags;
        this->ClearColour = other.ClearColour;

        this->CameraInstance = std::make_shared<SceneCamera>(*other.CameraInstance);

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
                    const char* type_string = (CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Perspective) ? "Perspective" : "Orthographic";
                    out << YAML::Key << "Projection Type" << YAML::Value << type_string;
                    if (type_string == "Perspective")
                    {
                        out << YAML::Key << "FOV" << YAML::Value << glm::degrees(CameraInstance->GetPerspectiveVerticalFOV());
                        out << YAML::Key << "Near" << YAML::Value << CameraInstance->GetPerspectiveNearClip();
                        out << YAML::Key << "Far" << YAML::Value << CameraInstance->GetPerspectiveFarClip();
                    }
                    else {
                        out << YAML::Key << "FOV" << YAML::Value << CameraInstance->GetOrthographicSize();
                        out << YAML::Key << "Near" << YAML::Value << CameraInstance->GetOrthographicNearClip();
                        out << YAML::Key << "Far" << YAML::Value << CameraInstance->GetOrthographicFarClip();
                    }
                }
                out << YAML::EndMap;
            }

            out << YAML::Key << "Primary" << YAML::Value << Primary;

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

            out << YAML::EndMap;
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
                else {
                    return false;
                }
                if (cameraNode["Near"]) {
                    CameraInstance->SetPerspectiveNearClip(cameraNode["Near"].as<float>());
                }
                else {
                    return false;
                }
                if (cameraNode["Far"]) {
                    CameraInstance->SetPerspectiveFarClip(cameraNode["Far"].as<float>());
                }
                else {
                    return false;
                }
            }
            else if (cameraNode["Projection Type"] && cameraNode["Projection Type"].as<std::string>() == "Orthographic")
            {
                CameraInstance->SetProjectionType(SceneCamera::ProjectionType::Orthographic);

                if (cameraNode["FOV"]) {
                    CameraInstance->SetOrthographicSize(cameraNode["FOV"].as<float>());
                }
                else {
                    return false;
                }
                if (cameraNode["Near"]) {
                    CameraInstance->SetOrthographicNearClip(cameraNode["Near"].as<float>());
                }
                else {
                    return false;
                }
                if (cameraNode["Far"]) {
                    CameraInstance->SetOrthographicFarClip(cameraNode["Far"].as<float>());
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        if (component["Primary"]) {
            Primary = component["Primary"].as<bool>();
        }
        else {
            return false;
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
        else {
            return false;
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

        return true;
    }

}