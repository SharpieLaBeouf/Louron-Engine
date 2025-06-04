#pragma once

// Louron Core Headers
#include "Component Base.h"
#include "../Prefab.h"
#include "../Spatial Partitioning/Bounds.h"

#include "../../Asset/Asset.h"
#include "../../Core/UUID.h"

// C++ Standard Library Headers
#include <memory>
#include <unordered_map>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace Louron
{
	class MaterialUniformBlock; // Forward Declare
	struct SkinnedMeshComponent : public ComponentBase
	{
		// --- Constructors & Assignment Operators ---

		SkinnedMeshComponent() = default;
		SkinnedMeshComponent(const SkinnedMeshComponent& other) = default;
		SkinnedMeshComponent(SkinnedMeshComponent&& other) noexcept = default;

		SkinnedMeshComponent& operator=(const SkinnedMeshComponent& other) = default;
		SkinnedMeshComponent& operator=(SkinnedMeshComponent&& other) noexcept = default;

		// --- Data of SkinnedMeshComponent Struct ---

		/// <summary>
		/// The Asset Handle to the Static Mesh of the Component
		/// </summary>
		AssetHandle StaticMeshHandle = NULL_UUID;

		/// <summary>
		/// The Vector of Materials and Material Uniform Blocks
		/// </summary>
		std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>> MaterialHandles = {};

		/// <summary>
		/// The Asset Handle to the Skeleton of the Component
		/// </summary>
		AssetHandle SkeletonHandle = NULL_UUID;

		/// <summary>
		/// Bone Mapping Between Skeleton Asset and Entities
		/// 
		/// Key: BoneID Reference
		/// Value: Entity UUID in Scene or Prefab
		/// </summary>
		std::unordered_map<UUID, UUID> SkeletonBoneMapping = {};

		/// <summary>
		/// Final vector of bone transformations to be provided to vertex shader for bone weight blending
		/// 
		/// This will persist even without an BasicAnimationComponent, hence being contained in this component
		/// </summary>
		std::vector<glm::mat4> FinalBoneTransformations = {};

		/// <summary>
		/// To Render or to not Render
		/// </summary>
		bool Active = true;

		/// <summary>
		/// Will Cast Shadows
		/// </summary>
		bool CastShadows = false;

		/// <summary>
		/// The Transformed AABB - Static Mesh AABB transformed by the TransformComponent attached to this Entity
		/// </summary>
		Bounds_AABB TransformedAABB = {};

		/// <summary>
		/// Flag - Whether AABB for this entity needs to be updated
		/// </summary>
		bool AABBNeedsUpdate = true;

		/// <summary>
		/// Flag - Whether the octree needs to update its position / AABB within the octree
		/// </summary>
		bool OctreeNeedsUpdate = true;

		/// <summary>
		/// Flag - Whether we should render the AABB for debugging/visualisation purposes
		/// </summary>
		bool DisplayDebugAABB = false;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);

		/// <summary>
		/// Transforms AABB of Instantiated Skinned Mesh to Transform in World
		/// </summary>
		void UpdateTransformedAABB();

		/// <summary>
		/// Used to Compute the Final Bone Transformations of the Skinned Mesh
		/// 
		/// If no updated bone transforms map is provided, you must ensure that 
		/// all transforms of bone entities are set in the scene to ensure this 
		/// is correct!
		/// 
		/// This merely updates the bone transforms based on the current skeleton 
		/// entity layout in the scene hierarchy.
		/// </summary>
		void ComputeFinalBoneTransformations(const std::unordered_map<UUID, glm::mat4>& updated_bone_transformations = {});

		/// <summary>
		/// Used to Compute the Final Bone Transformations of the Skinned Mesh
		/// </summary>
		void ComputeFinalBoneTransformations(std::shared_ptr<Prefab> model_prefab);
	};

}