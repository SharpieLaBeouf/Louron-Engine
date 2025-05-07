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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace Louron
{
	struct BoneLayout
	{
		/// <summary>
		/// The Name of the Bone - this will be used as a key 
		/// to the AnimationClip keyframe bone information
		/// 
		/// This is because AnimationClip's are Asset's that are 
		/// generalised across various Scenes/Prefabs where bone 
		/// entities will have varying UUID's.
		/// 
		/// So we keep the Bone Layout consistent with the name 
		/// mapping from the AnimationClip imported from ASSIMP.
		/// 
		/// When animations are processed, the AnimationClip bone
		/// references will key for the bone being transformed using 
		/// the name, and will change the transform of the 
		/// associated entity according to the BoneEntityID.
		/// </summary>
		std::string BoneName = "";

		/// <summary>
		/// The BoneID Reference
		/// </summary>
		UUID BoneID = NULL_UUID;

		/// <summary>
		/// Used for transforming vertices from Model Space to Bone Space
		/// </summary>
		glm::mat4 BoneOffsetMatrix = glm::mat4(1.0f);

		/// <summary>
		/// A Recursive Vector of Children References
		/// </summary>
		std::vector<BoneLayout> BoneChildren = {};

		BoneLayout() = default;
		BoneLayout(const BoneLayout& other) = default;
		BoneLayout(BoneLayout&& other) = default;

		BoneLayout& operator=(const BoneLayout& other) = default;
		BoneLayout& operator=(BoneLayout&& other) = default;

		size_t size() const;

		BoneLayout* find(const std::string& bone_name);

	};

	class Skeleton : public Asset
	{

	public :

		// --- Constructors & Assignment Operators ---

		virtual AssetType GetType() const override { return AssetType::Skeleton; }

		Skeleton() = default;
		Skeleton(const Skeleton& other) = default;
		Skeleton(Skeleton&& other) = default;

		Skeleton& operator=(const Skeleton& other) = default;
		Skeleton& operator=(Skeleton&& other) = default;

		// --- Data of Skeleton Asset ---

		/// <summary>
		/// Recursive Layout of Bones w/ References to Bones in Scene
		/// </summary>
		BoneLayout SkeletonLayout = {};

	};

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
		/// This will persist even without an AnimatorComponent, hence being contained in this component
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