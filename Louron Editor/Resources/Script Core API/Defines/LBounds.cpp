#include "LBounds.h"

#include "LMath.h"

namespace Louron
{

	using namespace Vectors;

	namespace Partitions
	{

		IntersectionResult Bounds_AABB::Contains(const Bounds_Sphere& sphere, float looseness) const
		{
			Vector3 center = Center();
			Vector3 half_size = Size() * 0.5f * looseness;

			Vector3 clamped_center = Vector3::Clamp(sphere.Centre, Min - half_size, Max + half_size);
			float distance_squared = Vector3(clamped_center - sphere.Centre).LengthSquared();

			if (distance_squared > sphere.Radius * sphere.Radius) 
				return IntersectionResult::DoesNotContain;

			bool contains = (Min.x - half_size.x <= sphere.Centre.x - sphere.Radius) &&
				(Max.x + half_size.x >= sphere.Centre.x + sphere.Radius) &&
				(Min.y - half_size.y <= sphere.Centre.y - sphere.Radius) &&
				(Max.y + half_size.y >= sphere.Centre.y + sphere.Radius) &&
				(Min.z - half_size.z <= sphere.Centre.z - sphere.Radius) &&
				(Max.z + half_size.z >= sphere.Centre.z + sphere.Radius);

			if (contains) return IntersectionResult::Contains;

			return IntersectionResult::Intersects;
		}

		IntersectionResult Bounds_AABB::Contains(const Bounds_AABB& other, float looseness) const
		{
			Vector3 center = Center();
			Vector3 half_size = Size() * 0.5f;
			Vector3 adjusted_half_size = half_size * looseness;

			Vector3 loose_min = center - adjusted_half_size;
			Vector3 loose_max = center + adjusted_half_size;

			bool contains = (loose_min.x <= other.Min.x && loose_max.x >= other.Max.x) &&
				(loose_min.y <= other.Min.y && loose_max.y >= other.Max.y) &&
				(loose_min.z <= other.Min.z && loose_max.z >= other.Max.z);

			if (contains) return IntersectionResult::Contains;

			bool intersects = (loose_min.x <= other.Max.x && loose_max.x >= other.Min.x) &&
				(loose_min.y <= other.Max.y && loose_max.y >= other.Min.y) &&
				(loose_min.z <= other.Max.z && loose_max.z >= other.Min.z);

			if (intersects) return IntersectionResult::Intersects;

			return IntersectionResult::DoesNotContain;

		}

		IntersectionResult Bounds_Sphere::Contains(const Bounds_Sphere& sphere, float looseness) const
		{
			float distance_squared = Vector3(sphere.Centre - Centre).LengthSquared();
			float radii_sum = Radius * looseness + sphere.Radius;

			if (distance_squared > radii_sum * radii_sum) return IntersectionResult::DoesNotContain;
			if (distance_squared <= (Radius - sphere.Radius) * (Radius - sphere.Radius)) return IntersectionResult::Contains;

			return IntersectionResult::Intersects;

		}

		IntersectionResult Bounds_Sphere::Contains(const Bounds_AABB& other, float looseness) const
		{
			return other.Contains(*this);
		}

		IntersectionResult Frustum::Contains(const Bounds_AABB& bounds)
		{
			bool all_points_inside = true;
			bool any_points_inside = false;

			for (const auto& plane : Planes) {
				Vector3 positive_vertex = bounds.Min;
				Vector3 negative_vertex = bounds.Max;

				if (plane.Normal.x >= 0) {
					positive_vertex.x = bounds.Max.x;
					negative_vertex.x = bounds.Min.x;
				}
				if (plane.Normal.y >= 0) {
					positive_vertex.y = bounds.Max.y;
					negative_vertex.y = bounds.Min.y;
				}
				if (plane.Normal.z >= 0) {
					positive_vertex.z = bounds.Max.z;
					negative_vertex.z = bounds.Min.z;
				}

				float distance_to_positive = Vector3::Dot(plane.Normal, positive_vertex) + plane.Distance;
				float distance_to_negative = Vector3::Dot(plane.Normal, negative_vertex) + plane.Distance;

				if (distance_to_positive < 0 && distance_to_negative < 0) 
					return IntersectionResult::DoesNotContain;

				if (distance_to_positive >= 0 || distance_to_negative >= 0) 
					any_points_inside = true;

				if (!(distance_to_positive >= 0 && distance_to_negative >= 0)) 
					all_points_inside = false;
			}

			if (all_points_inside) return IntersectionResult::Contains;
			if (any_points_inside) return IntersectionResult::Intersects;

			return IntersectionResult::DoesNotContain;
		}

		IntersectionResult Frustum::Contains(const Bounds_Sphere& bounds)
		{
			bool all_inside = true;
			bool intersects = false;

			for (const auto& plane : Planes) 
			{
				float distance_to_center = Vector3::Dot(plane.Normal, bounds.Centre) + plane.Distance;

				if (distance_to_center < -bounds.Radius)
					return IntersectionResult::DoesNotContain;
			
				if (distance_to_center < bounds.Radius) 
					intersects = true;
			
				if (distance_to_center <= 0) 
					all_inside = false;
			}

			if (all_inside) return IntersectionResult::Contains;
			if (intersects) return IntersectionResult::Intersects;
		
			return IntersectionResult::DoesNotContain;
		}

	}
}
