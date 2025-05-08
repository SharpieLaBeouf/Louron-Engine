#include "SkinnedMeshComponent.h"

// Louron Core Headers
#include "../Entity.h"

#include "../../Core/Logging.h"
#include "../../Asset/Asset Manager API.h"
#include "../../OpenGL/Mesh.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron
{

	void SkinnedMeshComponent::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "SkinnedMeshComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "StaticMeshHandle" << YAML::Value << (uint32_t)StaticMeshHandle;
		out << YAML::Key << "SkeletonHandle" << YAML::Value << (uint32_t)SkeletonHandle;

		out << YAML::Key << "MeshActive" << YAML::Value << Active;
		out << YAML::Key << "CastShadows" << YAML::Value << CastShadows;

		{
			out << YAML::Key << "MaterialAssetCount" << YAML::Value << (uint32_t)MaterialHandles.size();
			out << YAML::Key << "MaterialAssetHandles" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, uniform_block] : MaterialHandles) {
				out << (uint32_t)handle;
			}
			out << YAML::EndSeq;
		} 
		
		{
			out << YAML::Key << "BoneMapping" << YAML::Value << YAML::BeginMap;

			int i = 0;
			for (const auto& [bone_index, entity_uuid] : SkeletonBoneMapping)
			{
				out << YAML::Key << ("Bone " + std::to_string(i)) << YAML::Value << YAML::BeginMap;

				out << YAML::Key << "Bone Index" << YAML::Value << static_cast<uint32_t>(bone_index);
				out << YAML::Key << "Bone Entity Reference" << YAML::Value << static_cast<uint32_t>(entity_uuid);

				out << YAML::EndMap;

				i++;
			}

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	bool SkinnedMeshComponent::Deserialize(const YAML::Node data)
	{
		YAML::Node component = data;

		if (component["StaticMeshHandle"]) 
		{
			StaticMeshHandle = component["StaticMeshHandle"].as<uint32_t>();
		}

		if (component["SkeletonHandle"]) 
		{
			SkeletonHandle = component["SkeletonHandle"].as<uint32_t>();
		}

		if (component["MeshActive"]) 
		{
			Active = component["MeshActive"].as<bool>();
		}

		if (component["CastShadows"]) 
		{
			CastShadows = component["CastShadows"].as<bool>();
		}

		if (component["MaterialAssetCount"] && component["MaterialAssetHandles"]) 
		{
			uint32_t count = component["MaterialAssetCount"].as<uint32_t>();
			YAML::Node handles = component["MaterialAssetHandles"];

			if (handles.size() != count) {
				return false;
			}

			MaterialHandles.clear();
			for (const auto& handle : handles) {
				MaterialHandles.push_back({ handle.as<uint32_t>() , nullptr });
			}
		}
		else 
		{
			return false;
		}

		if (component["BoneMapping"])
		{
			YAML::Node bone_mapping = component["BoneMapping"];

			for (const auto& bone : bone_mapping)
			{
				uint32_t bone_index = bone.second["Bone Index"].as<uint32_t>();
				uint32_t entity_reference = bone.second["Bone Entity Reference"].as<uint32_t>();

				SkeletonBoneMapping[bone_index] = entity_reference;
			}
		}

		if (AssetManager::IsAssetHandleValid(SkeletonHandle))
		{
			// Import the Model and Skeleton Now
			AssetManager::GetAsset<Skeleton>(SkeletonHandle);
		}

		ComputeFinalBoneTransformations();

		return true;
	}

	void SkinnedMeshComponent::UpdateTransformedAABB()
	{
		if (!GetEntity()) {
			L_CORE_ERROR("Cannot UpdateTransformedAABB - Current Entity Is Invalid!");
			return;
		}

		Entity entity = *GetEntity();

		if (!entity || !entity.GetScene())
			return;

		if (StaticMeshHandle == NULL_UUID)
			return;

		if (auto mesh_asset = AssetManager::GetAsset<StaticMesh>(StaticMeshHandle); mesh_asset) {

			// Define the 8 corner points of the AABB
			std::array<glm::vec3, 8> corners = {
				mesh_asset->MeshBounds.BoundsMin,
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMax.z)
			};

			// Transform the corner points
			glm::mat4 global_transform = GetComponent<TransformComponent>().GetGlobalTransform();
			for (int i = 0; i < 8; ++i) {
				glm::vec4 transformed_corner = global_transform * glm::vec4(corners[i], 1.0f);
				corners[i] = glm::vec3(transformed_corner);
			}

			// Find the new BoundsMin and BoundsMax
			glm::vec3 newMin = corners[0];
			glm::vec3 newMax = corners[0];
			for (int i = 1; i < 8; ++i) {
				newMin = glm::min(newMin, corners[i]);
				newMax = glm::max(newMax, corners[i]);
			}
			TransformedAABB.BoundsMin = newMin;
			TransformedAABB.BoundsMax = newMax;

			AABBNeedsUpdate = false;
		}
	}

	void SkinnedMeshComponent::ComputeFinalBoneTransformations(const std::unordered_map<UUID, glm::mat4>& updated_bone_transformations)
	{
		if (!GetEntity()) return;
		Scene* scene_ref = GetEntity()->GetScene();
		if (!scene_ref) return;

		std::shared_ptr<Skeleton> skeleton;
		
		if(AssetManager::IsAssetLoaded(SkeletonHandle))
			skeleton = AssetManager::GetAsset<Skeleton>(SkeletonHandle);
		
		if (!skeleton) 
			return;

		size_t skeleton_size = skeleton->SkeletonLayout.size();
		FinalBoneTransformations.resize(glm::min(skeleton_size, static_cast<size_t>(200)), glm::mat4(1.0f));

		// Recursive lambda for setting bone transformations
		std::function<void(const BoneLayout&, const glm::mat4&)> InitializeBoneTransformations = [&](const BoneLayout& bone, const glm::mat4& parent_transform)
		{
			auto it = SkeletonBoneMapping.find(bone.BoneID);
			if (it == SkeletonBoneMapping.end()) 
			{
				L_CORE_WARN("SkinnedMeshComponent::InitializeBindPose - Could Not Find Bone in Skeleton Mapping");
				return; // Ensure BoneID exists
			}

			Entity bone_entity = scene_ref->FindEntityByUUID(it->second);
			if (!bone_entity) return;

			auto found_transformation = updated_bone_transformations.find(bone_entity.GetUUID());
			if (found_transformation != updated_bone_transformations.end())
				bone_entity.GetComponent<TransformComponent>().SetTransform(found_transformation->second);

			glm::mat4 global_transform = parent_transform * bone_entity.GetComponent<TransformComponent>().GetLocalTransform(false); // Don't propograte update flags to all children yet

			if (bone.BoneID < FinalBoneTransformations.size())
			{
				FinalBoneTransformations[bone.BoneID] = global_transform * bone.BoneOffsetMatrix;
			}
			else
			{
				L_CORE_WARN("SkinnedMeshComponent::InitializeBindPose - Attempted to Index Outside FinalBoneTransformations Vector.");
			}

			for (const auto& child : bone.BoneChildren)
			{
				InitializeBoneTransformations(child, global_transform);
			}
		};

		// Start recursion from the root bone
		InitializeBoneTransformations(skeleton->SkeletonLayout, glm::mat4(1.0f));
		
		auto it = SkeletonBoneMapping.find(skeleton->SkeletonLayout.BoneID);
		if (it == SkeletonBoneMapping.end())
		{
			L_CORE_WARN("SkinnedMeshComponent::InitializeBindPose - Could Not Find Bone in Skeleton Mapping");
			return; // Ensure BoneID exists
		}

		Entity bone_entity = scene_ref->FindEntityByUUID(it->second);
		if (!bone_entity) 
			return;

		bone_entity.GetComponent<TransformComponent>().UpdateLocalTransformMatrix(); // Propogate update flags to all children 
	}

	void SkinnedMeshComponent::ComputeFinalBoneTransformations(std::shared_ptr<Prefab> model_prefab)
	{
		auto skeleton = AssetManager::GetAsset<Skeleton>(SkeletonHandle);
		if (!skeleton) return;

		size_t skeleton_size = skeleton->SkeletonLayout.size();
		FinalBoneTransformations.resize(glm::min(skeleton_size, static_cast<size_t>(200)), glm::mat4(1.0f));

		// Recursive lambda for setting bone transformations
		std::function<void(const BoneLayout&, const glm::mat4&)> InitializeBoneTransformations = [&](const BoneLayout& bone, const glm::mat4& parent_transform)
		{
			auto it = SkeletonBoneMapping.find(bone.BoneID);
			if (it == SkeletonBoneMapping.end())
			{
				L_CORE_WARN("SkinnedMeshComponent::InitializeBindPose - Could Not Find Bone in Skeleton Mapping");
				return; // Ensure BoneID exists
			}

			entt::entity bone_entity = model_prefab->FindEntityByUUID(it->second);
			if (bone_entity == entt::null) return;

			glm::mat4 global_transform = parent_transform * model_prefab->GetComponent<TransformComponent>(bone_entity).GetLocalTransform(false);

			if (bone.BoneID < FinalBoneTransformations.size())
			{
				FinalBoneTransformations[bone.BoneID] = global_transform * bone.BoneOffsetMatrix;
			}
			else
			{
				L_CORE_WARN("SkinnedMeshComponent::InitializeBindPose - Attempted to Index Outside FinalBoneTransformations Vector.");
			}

			for (const auto& child : bone.BoneChildren)
			{
				InitializeBoneTransformations(child, global_transform);
			}
		};

		// Start recursion from the root bone
		InitializeBoneTransformations(skeleton->SkeletonLayout, glm::mat4(1.0f));
	}

	size_t BoneLayout::size() const
	{
		size_t totalSize = 1;

		for (const auto& child : BoneChildren) 
		{
			totalSize += child.size();
		}

		return totalSize;
	}

	BoneLayout* BoneLayout::find(const std::string& bone_name)
	{
		if (BoneName == bone_name)
			return this;

		for (auto& child_bone : BoneChildren)
		{
			BoneLayout* bone_found = child_bone.find(bone_name);
			if (bone_found) return bone_found;
		}

		return nullptr;
	}

}
