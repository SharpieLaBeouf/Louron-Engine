#pragma once

#include "Script Core API/ScriptAPI.h"

#include <format>

using namespace Louron;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class NativeScriptingTypesTest : public IScript
{

public:

	// Inbuilt Types
	[[ExposedInEditor]] float my_float = 3.14f;
	[[ExposedInEditor]] double my_double = 2.71828;
	[[ExposedInEditor]] int8_t my_int8 = -8;
	[[ExposedInEditor]] int16_t my_int16 = -16000;
	[[ExposedInEditor]] int32_t my_int32 = -123456;
	[[ExposedInEditor]] int64_t my_int64 = -9876543210;
	[[ExposedInEditor]] uint8_t my_uint8 = 8;
	[[ExposedInEditor]] uint16_t my_uint16 = 16000;
	[[ExposedInEditor]] uint32_t my_uint32 = 123456;
	[[ExposedInEditor]] uint64_t my_uint64 = 9876543210;
	[[ExposedInEditor]] bool my_bool = true;
	[[ExposedInEditor]] const char* my_cstring = "Hello Editor!";

	// Custom Types
	[[ExposedInEditor]] Vector2 my_vector2 = { 1.0f, 2.0f };
	[[ExposedInEditor]] Vector3 my_vector3 = { 3.0f, 4.0f, 5.0f };
	[[ExposedInEditor]] Vector4 my_vector4 = { 6.0f, 7.0f, 8.0f, 9.0f };
	[[ExposedInEditor]] UVector2 my_uvector2 = { 10, 11 };
	[[ExposedInEditor]] UVector3 my_uvector3 = { 12, 13, 14 };
	[[ExposedInEditor]] UVector4 my_uvector4 = { 15, 16, 17, 18 };
	[[ExposedInEditor]] IVector2 my_ivector2 = { -1, -2 };
	[[ExposedInEditor]] IVector3 my_ivector3 = { -3, -4, -5 };
	[[ExposedInEditor]] IVector4 my_ivector4 = { -6, -7, -8, -9 };
	[[ExposedInEditor]] DVector2 my_dvector2 = { 1.1, 2.2 };
	[[ExposedInEditor]] DVector3 my_dvector3 = { 3.3, 4.4, 5.5 };
	[[ExposedInEditor]] DVector4 my_dvector4 = { 6.6, 7.7, 8.8, 9.9 };
	[[ExposedInEditor]] BVector2 my_bvector2 = { true, false };
	[[ExposedInEditor]] BVector3 my_bvector3 = { false, true, true };
	[[ExposedInEditor]] BVector4 my_bvector4 = { true, false, true, false };

	ComputeBuffer buffer{};
	void OnCreate() override
	{
		buffer.Create(10, sizeof(float));
		Debug::Log(std::format("Hello, World! My Name Is {} :)", GetTag()));
	}

	float timer = 1.0f;

	int counter = 1;
	void OnUpdate() override
	{
		if (timer > 0.0f)
		{
			timer -= Time::GetDeltaTime(); // convert milliseconds to seconds
		}
		else
		{
			// === DEBUG LOG ===
			Debug::Log("==== Testing All Exposed Types ====");
			Debug::Log(std::format(" - my_float: {}", my_float));
			Debug::Log(std::format(" - my_double: {}", my_double));
			Debug::Log(std::format(" - my_int8: {}", my_int8));
			Debug::Log(std::format(" - my_int16: {}", my_int16));
			Debug::Log(std::format(" - my_int32: {}", my_int32));
			Debug::Log(std::format(" - my_int64: {}", my_int64));
			Debug::Log(std::format(" - my_uint8: {}", my_uint8));
			Debug::Log(std::format(" - my_uint16: {}", my_uint16));
			Debug::Log(std::format(" - my_uint32: {}", my_uint32));
			Debug::Log(std::format(" - my_uint64: {}", my_uint64));
			Debug::Log(std::format(" - my_bool: {}", my_bool ? "True" : "False"));
			Debug::Log(std::format(" - my_cstring: {}", my_cstring));

			Debug::Log(std::format(" - my_vector2: {}", my_vector2.ToString()));
			Debug::Log(std::format(" - my_vector3: {}", my_vector3.ToString()));
			Debug::Log(std::format(" - my_vector4: {}", my_vector4.ToString()));
			Debug::Log(std::format(" - my_uvector2: {}", my_uvector2.ToString()));
			Debug::Log(std::format(" - my_uvector3: {}", my_uvector3.ToString()));
			Debug::Log(std::format(" - my_uvector4: {}", my_uvector4.ToString()));
			Debug::Log(std::format(" - my_ivector2: {}", my_ivector2.ToString()));
			Debug::Log(std::format(" - my_ivector3: {}", my_ivector3.ToString()));
			Debug::Log(std::format(" - my_ivector4: {}", my_ivector4.ToString()));
			Debug::Log(std::format(" - my_dvector2: {}", my_dvector2.ToString()));
			Debug::Log(std::format(" - my_dvector3: {}", my_dvector3.ToString()));
			Debug::Log(std::format(" - my_dvector4: {}", my_dvector4.ToString()));
			Debug::Log(std::format(" - my_bvector2: {}", my_bvector2.ToString()));
			Debug::Log(std::format(" - my_bvector3: {}", my_bvector3.ToString()));
			Debug::Log(std::format(" - my_bvector4: {}", my_bvector4.ToString()));

			// === MODULATION ===
			my_float += 0.01f;
			my_double += 0.005;
			my_int8++;
			my_int16++;
			my_int32 += 2;
			my_int64 += 3;
			my_uint8++;
			my_uint16++;
			my_uint32 += 5;
			my_uint64 += 7;

			if (static_cast<int>(timer * 2.0f) % 2 == 0) {
				my_bool = !my_bool;
			}

			my_vector2.x += 0.1f;
			my_vector3.y += 0.2f;
			my_vector4.z += 0.3f;

			my_uvector2.x += 1;
			my_uvector3.y += 1;
			my_uvector4.z += 1;

			my_ivector2.x *= -1;
			my_ivector3.y *= -1;
			my_ivector4.z *= -1;

			my_dvector2.x += 0.15;
			my_dvector3.y += 0.25;
			my_dvector4.z += 0.35;

			my_bvector2.x = (static_cast<int>(Time::GetDeltaTime()) % 2 == 0);
			my_bvector3.y = (static_cast<int>(Time::GetDeltaTime() * 0.5f) % 2 != 0);
			my_bvector4.z = !my_bvector4.z;
			
			timer = 1.0f;
		}
	}

	void OnLateUpdate() override
	{

	}

	void OnFixedUpdate() override
	{

	}

	void OnDestroy() override
	{
		Debug::Log("Goodbye, World! :(");
	}	

private:

};