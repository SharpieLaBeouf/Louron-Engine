#pragma once

// Louron Core Headers
#include "Component Base.h"

// C++ Standard Library Headers
#include <string>
#include <unordered_map>
#include <array>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace YAML {

	class Emitter;
	class Node;
}

namespace Louron {

	enum class ShadowTypeFlag : uint8_t 
	{

		NoShadows	= 0,
		HardShadows = 1,
		SoftShadows = 2,

	};

	struct PointLightComponent : public ComponentBase 
	{
		bool Active = true;

		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float Radius = 10.0f;
		float Intensity = 1.0f;

		ShadowTypeFlag ShadowFlag = ShadowTypeFlag::NoShadows;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent(PointLightComponent&&) noexcept = default;

		PointLightComponent& operator=(const PointLightComponent&) = default;
		PointLightComponent& operator=(PointLightComponent&&) noexcept = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);
	};

	struct SpotLightComponent : public ComponentBase 
	{
		bool Active = true;

		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float Range = 20.0f;
		float Angle = 45.0f; // Full Angle of SpotLight Cone
		float Intensity = 1.0f;

		ShadowTypeFlag ShadowFlag = ShadowTypeFlag::NoShadows;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
		SpotLightComponent(SpotLightComponent&&) noexcept = default;

		SpotLightComponent& operator=(const SpotLightComponent&) = default;
		SpotLightComponent& operator=(SpotLightComponent&&) noexcept = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);
	};

	struct DirectionalLightComponent : public ComponentBase 
	{		
		bool Active = true;

		glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		float Intensity = 1.0f;

		float MaxShadowVisibleDistance = 1.0f; // Normalised 0 == near plane of camera, 1 == far plane of camera
		ShadowTypeFlag ShadowFlag = ShadowTypeFlag::NoShadows;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(DirectionalLightComponent&&) noexcept = default;

		DirectionalLightComponent& operator=(const DirectionalLightComponent&) = default;
		DirectionalLightComponent& operator=(DirectionalLightComponent&&) noexcept = default;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);
	};
}