#pragma once

#include "../Core/UUID.h"
#include "../Asset/Asset.h"

#include <string>
#include <vector>

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

}