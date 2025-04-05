#pragma once

#include "LMath.h"
#include "LVectors.h"

#include <array>

namespace Louron
{

	namespace Partitions
	{
		enum class IntersectionResult : uint8_t
		{
			DoesNotContain,
			Intersects,
			Contains
		};

		struct Bounds_Sphere;
		struct Bounds_AABB;

		struct Bounds_AABB
		{
			Vectors::Vector3 Min;
			Vectors::Vector3 Max;

			Vectors::Vector3 Center() const { return (Min + Max) * 0.5f; }
			Vectors::Vector3 Size() const { return Max - Min; }
			float MaxExtent() const { Vectors::Vector3 extent = (Max - Min) * 0.5f; return extent.Length(); }

			IntersectionResult Contains(const Bounds_Sphere& sphere, float looseness = 1.0f) const;
			IntersectionResult Contains(const Bounds_AABB& other, float looseness = 1.0f) const;
		};

		struct Bounds_Sphere
		{
			Vectors::Vector3 Centre;
			float Radius;

			IntersectionResult Contains(const Bounds_Sphere& other, float looseness = 1.0f) const;
			IntersectionResult Contains(const Bounds_AABB& aabb, float looseness = 1.0f) const;
		};

		struct Frustum
		{
			struct Plane
			{
				Vectors::Vector3 Normal;
				float Distance;

				void Normalise() {
					float length = Normal.Length();
					Normal = Normal / length;
					Distance /= length;
				}

				float DistanceToPoint(const Vectors::Vector3& point) const
				{
					return Vectors::Vector3::Dot(Normal, point) + Distance;
				}
			};

			std::array<Plane, 6> Planes{};

			IntersectionResult Contains(const Bounds_AABB& bounds);
			IntersectionResult Contains(const Bounds_Sphere& bounds);
		};

	}
}