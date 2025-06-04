#include "Animator Component.h"

// Louron Core Headers
#include "../Entity.h"
#include "SkinnedMeshComponent.h"

#include "../../Core/Time.h"
#include "../../Asset/Asset Manager API.h"
#include "../../Animation/Animations.h"
#include "../../Animation/Skeleton.h"
#include "../../Animation/Humanoid.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <yaml-cpp/yaml.h>


namespace Louron
{
#pragma region Animator Component

    AnimatorComponent::AnimatorComponent(const AnimatorComponent &other)
    {
		StateMachineHandle = other.StateMachineHandle;
		CullingMode = other.CullingMode;

		m_StateMachineInstance.reset();
		m_StateMachineInstance = nullptr;

		if (other.m_StateMachineInstance && StateMachineHandle != NULL_UUID && AssetManager::IsAssetHandleValid(StateMachineHandle))
		{
			auto machine_asset = AssetManager::GetAsset<Animation::StateMachine>(StateMachineHandle);
			if(machine_asset)
			{
				m_StateMachineInstance = std::make_unique<Animation::StateMachine>(*machine_asset.get()); // Copy asset into component instance
			}
		}
    }

    AnimatorComponent &AnimatorComponent::operator=(const AnimatorComponent &other)
    {
		if (this == &other)
			return *this;

		StateMachineHandle = other.StateMachineHandle;
		CullingMode = other.CullingMode;

		m_StateMachineInstance.reset();
		m_StateMachineInstance = nullptr;

		if (other.m_StateMachineInstance && StateMachineHandle != NULL_UUID && AssetManager::IsAssetHandleValid(StateMachineHandle))
		{
			auto machine_asset = AssetManager::GetAsset<Animation::StateMachine>(StateMachineHandle);
			if(machine_asset)
			{
				m_StateMachineInstance = std::make_unique<Animation::StateMachine>(*machine_asset.get()); // Copy asset into component instance
			}
		}

		return *this;
    }

    void AnimatorComponent::Init()
    {
		m_StateMachineInstance.reset();
		m_StateMachineInstance = nullptr;

		if (StateMachineHandle != NULL_UUID && AssetManager::IsAssetHandleValid(StateMachineHandle))
		{
			auto machine_asset = AssetManager::GetAsset<Animation::StateMachine>(StateMachineHandle);
			if(machine_asset)
			{
				m_StateMachineInstance = std::make_unique<Animation::StateMachine>(*machine_asset.get()); // Copy asset into component instance
			}
		}
	}

	void AnimatorComponent::CleanUp()
	{
		m_StateMachineInstance.reset();
		m_StateMachineInstance = nullptr;		
	}

	void AnimatorComponent::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "AnimatorComponent" << YAML::Value;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "StateMachineHandle" << YAML::Value << StateMachineHandle;

			switch (CullingMode)
			{
				case AnimationCullingMode::AlwaysAnimate:
				{
					out << YAML::Key << "CullingMode" << YAML::Value << "AlwaysAnimate";
					break;
				}
				case AnimationCullingMode::NoAnimateOffScreenContinueTimer:
				{
					out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenContinueTimer";
					break;
				}
				default:
				case AnimationCullingMode::NoAnimateOffScreenStopTimer:
				{
					out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenStopTimer";
					break;
				}
			}

		}
		out << YAML::EndMap;
	}

	bool AnimatorComponent::Deserialize(const YAML::Node data)
	{
		YAML::Node component = data;

		if (component["StateMachineHandle"]) 
		{
			StateMachineHandle = component["StateMachineHandle"].as<uint32_t>();
		}

		if (component["CullingMode"]) 
		{
			std::string culling_mode = component["CullingMode"].as<std::string>();

			if (culling_mode == "AlwaysAnimate")
				CullingMode = AnimationCullingMode::AlwaysAnimate;
			else if (culling_mode == "NoAnimateOffScreenContinueTimer")
				CullingMode = AnimationCullingMode::NoAnimateOffScreenContinueTimer;
			else if (culling_mode == "NoAnimateOffScreenStopTimer")
				CullingMode = AnimationCullingMode::NoAnimateOffScreenStopTimer;
		}

		return true;
	}

#pragma endregion

#pragma region Basic Animation Component

	void BasicAnimationComponent::Play(int clip_index, bool should_loop)
	{
		if (clip_index < 0 || clip_index >= AnimationClipHandles.size())
		{
			L_CORE_WARN("BasicAnimationComponent::Play - Clip Index Invalid.");
			return;
		}

		CurrentClipIndex = clip_index;
		CurrentTime = 0.0f;
		IsPlaying = true;
		IsLooping = should_loop;
	}

	void BasicAnimationComponent::Play(const char* clip_name, bool should_loop)
	{
		uint32_t found_index = NULL_UUID;
		for (auto it = AnimationClipHandles.begin(); it != AnimationClipHandles.end(); )
		{
			if (*it == NULL_UUID)
			{
				++it;
				continue;
			}

			const auto& clip_meta_data = Project::GetActiveProject()->GetEditorAssetManager()->GetMetadata(*it);
			if(clip_meta_data.AssetName == clip_name)
			{
				found_index = std::distance(AnimationClipHandles.begin(), it);
				break;
			}
			++it;
		}

		if(found_index == NULL_UUID)
		{
			L_CORE_WARN("BasicAnimationComponent::Play - Could Not Find \"{}\" in Animation Clips.", clip_name);
			return;
		}

		CurrentClipIndex = found_index;
		CurrentTime = 0.0f;
		IsPlaying = true;
		IsLooping = should_loop;
	}

	void BasicAnimationComponent::Update()
	{
		// Check if Valid State to Animate
		if (!IsPlaying || CurrentClipIndex == -1) 
		{
			return;
		}

		// Retrieve Animation Clip Asset
		std::shared_ptr<AnimationClip> animation_clip_asset = nullptr;

		if (!AssetManager::IsAssetLoaded(AnimationClipHandles[CurrentClipIndex]))
			return;

		animation_clip_asset = AssetManager::GetAsset<AnimationClip>(AnimationClipHandles[CurrentClipIndex]);

		StepAnimationTimer(animation_clip_asset);

		// Get Skinned Mesh Component
		auto& skinned_mesh_component = GetComponent<SkinnedMeshComponent>();

		// Get Skeleton Asset
		auto asset_skeleton = AssetManager::GetAsset<Skeleton>(skinned_mesh_component.SkeletonHandle);

		// Update Bone Hierarchy of Skeleton
		UpdateBoneHierarchy(asset_skeleton->SkeletonLayout, asset_skeleton, skinned_mesh_component.SkeletonBoneMapping, *animation_clip_asset.get(), CurrentTime);
	}

	std::unordered_map<UUID, glm::mat4> BasicAnimationComponent::UpdateDeferred()
	{

		// Check if Valid State to Animate
		if (!IsPlaying || CurrentClipIndex == -1)
		{
			return {};
		}

		// Retrieve Animation Clip Asset
		std::shared_ptr<AnimationClip> animation_clip_asset = nullptr;

		if (!AssetManager::IsAssetLoaded(AnimationClipHandles[CurrentClipIndex]))
			return {};

		animation_clip_asset = AssetManager::GetAsset<AnimationClip>(AnimationClipHandles[CurrentClipIndex]);

		// Increment Current Animation Time
		CurrentTime += Time::GetDeltaTime() * animation_clip_asset->GetTicksPerSecond() * PlaybackSpeed;

		// Loop of Stop Animation
		if (CurrentTime > animation_clip_asset->GetDuration())
		{
			if (IsLooping)
			{
				CurrentTime = 0.0f;
			}
			else
			{
				Stop();
				return {};
			}
		}

		// Get Skinned Mesh Component
		auto& skinned_mesh_component = GetComponent<SkinnedMeshComponent>();

		// Get Skeleton Asset
		auto asset_skeleton = AssetManager::GetAsset<Skeleton>(skinned_mesh_component.SkeletonHandle);

		std::unordered_map<UUID, glm::mat4> transform_update_map;
		transform_update_map.reserve(skinned_mesh_component.SkeletonBoneMapping.size());
		// Update Bone Hierarchy of Skeleton
		UpdateBoneHierarchyDeferred(transform_update_map, asset_skeleton->SkeletonLayout, asset_skeleton, skinned_mesh_component.SkeletonBoneMapping, *animation_clip_asset.get(), CurrentTime);
		return transform_update_map;
	}

	void BasicAnimationComponent::StepAnimationTimer(const std::shared_ptr<AnimationClip>& animation_clip)
	{
		// Increment Current Animation Time
		CurrentTime += Time::GetDeltaTime() * animation_clip->GetTicksPerSecond() * PlaybackSpeed;

		// Loop of Stop Animation
		if (CurrentTime > animation_clip->GetDuration())
		{
			if (IsLooping)
			{
				CurrentTime = 0.0f;
			}
			else
			{
				Stop();
				return;
			}
		}
	}

	void BasicAnimationComponent::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "BasicAnimationComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "CurrentClipIndex" << YAML::Value << CurrentClipIndex;

		out << YAML::Key << "CurrentTime" << YAML::Value << CurrentTime;
		out << YAML::Key << "PlaybackSpeed" << YAML::Value << PlaybackSpeed;

		out << YAML::Key << "IsPlaying" << YAML::Value << IsPlaying;
		out << YAML::Key << "IsLooping" << YAML::Value << IsLooping;

		switch (CullingMode)
		{
			case AnimationCullingMode::AlwaysAnimate:
			{
				out << YAML::Key << "CullingMode" << YAML::Value << "AlwaysAnimate";
				break;
			}
			case AnimationCullingMode::NoAnimateOffScreenContinueTimer:
			{
				out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenContinueTimer";
				break;
			}
			default:
			case AnimationCullingMode::NoAnimateOffScreenStopTimer:
			{
				out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenStopTimer";
				break;
			}
		}

		{
			out << YAML::Key << "AnimationClipHandles" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& handle : AnimationClipHandles) {
				out << (uint32_t)handle;
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;
	}

	bool BasicAnimationComponent::Deserialize(const YAML::Node data)
	{
		YAML::Node component = data;

		if (component["CurrentClipIndex"]) 
		{
			CurrentClipIndex = component["CurrentClipIndex"].as<int>();
		}

		if (component["CurrentTime"]) 
		{
			CurrentTime = component["CurrentTime"].as<float>();
		}

		if (component["PlaybackSpeed"]) 
		{
			PlaybackSpeed = component["PlaybackSpeed"].as<float>();
		}

		if (component["IsPlaying"]) 
		{
			IsPlaying = component["IsPlaying"].as<bool>();
		}

		if (component["IsLooping"]) 
		{
			IsLooping = component["IsLooping"].as<bool>();
		}

		if (component["CullingMode"]) 
		{
			std::string culling_mode = component["CullingMode"].as<std::string>();

			if (culling_mode == "AlwaysAnimate")
				CullingMode = AnimationCullingMode::AlwaysAnimate;
			else if (culling_mode == "NoAnimateOffScreenContinueTimer")
				CullingMode = AnimationCullingMode::NoAnimateOffScreenContinueTimer;
			else if (culling_mode == "NoAnimateOffScreenStopTimer")
				CullingMode = AnimationCullingMode::NoAnimateOffScreenStopTimer;
		}

		if (component["AnimationClipHandles"]) 
		{
			YAML::Node handles = component["AnimationClipHandles"];
			AnimationClipHandles.clear();
			for (const auto& handle : handles) 
			{
				AnimationClipHandles.emplace_back(handle.as<uint32_t>());
			}
		}
		else 
		{
			return false;
		}

		return true;
	}

	void BasicAnimationComponent::UpdateBoneHierarchy(const BoneLayout& current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<UUID, UUID>& bone_mapping, const AnimationClip& animation_clip, float time)
	{
		// Check Current Entity Valid
		if (!GetEntity())
		{
			L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy Could Not Update Bone Hierarchy - Entity Reference is Invalid!");
			return;
		}

		// Check Current Scene Valid
		Scene* scene_ref = GetEntity()->GetScene();
		if (!scene_ref)
		{
			L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy Could Not Update Bone Hierarchy - Scene Reference is Invalid!");
			return;
		}

		if (bone_mapping.count(current_bone.BoneID) == 0)
		{
			L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy Could Not Find Entity Mapped to Bone.");
			return;
		}

		// Check Bone Entity Valid
		Entity bone_entity = scene_ref->FindEntityByUUID(bone_mapping.at(current_bone.BoneID));

		if(!bone_entity)
		{
			L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy Could Not Update Bone Hierarchy - Bone Entity ID in Skeleton Invalid {}!", std::to_string(bone_mapping.at(current_bone.BoneID)));
			return;
		}

		// Interpolate Transform from Keyframes
		glm::vec3 position	= InterpolatePositionTransform(animation_clip, current_bone.BoneName, time);
		glm::quat rotation	= InterpolateRotationTransform(animation_clip, current_bone.BoneName, time);
		glm::vec3 scale		= InterpolateScaleTransform(animation_clip, current_bone.BoneName, time);

		auto& transform_component = bone_entity.GetComponent<TransformComponent>();

		if (position == glm::vec3(FLT_MAX))
			position = transform_component.GetLocalPosition();

		if (rotation == glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX))
			rotation = transform_component.GetLocalRotation();

		if (scale == glm::vec3(FLT_MAX))
			scale = transform_component.GetLocalScale();

		glm::mat4 transform =
				glm::translate(glm::mat4(1.0f), position)
			*	glm::mat4_cast(rotation)
			*	glm::scale(glm::mat4(1.0f), scale);

		// Set the Bone's Local Transform
		bone_entity.GetComponent<TransformComponent>().SetTransform(transform);

		// Recurse through children
		for (const BoneLayout& child : current_bone.BoneChildren)
		{
			UpdateBoneHierarchy(child, skeleton_asset, bone_mapping, animation_clip, time);
		}
	}

	void BasicAnimationComponent::UpdateBoneHierarchyDeferred(std::unordered_map<UUID, glm::mat4>& transform_update_map, const BoneLayout& current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<UUID, UUID>& bone_mapping, const AnimationClip& animation_clip, float time)
	{
		// Validate scene reference early to prevent redundant calls
		Scene* scene_ref = GetEntity() ? GetEntity()->GetScene() : nullptr;
		if (!scene_ref)
		{
			static bool error_logged = false;
			if (!error_logged)
			{
				L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy: Scene reference is invalid!");
				error_logged = true;
			}
			return;
		}

		// Retrieve mapped entity UUID
		auto it = bone_mapping.find(current_bone.BoneID);
		if (it == bone_mapping.end())
		{
			L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy: BoneID {} not found in mapping.", std::to_string(current_bone.BoneID));
			return;
		}

		UUID bone_entity_uuid = it->second;
		Entity bone_entity = scene_ref->FindEntityByUUID(bone_entity_uuid);
		if (!bone_entity)
		{
			L_CORE_ERROR("BasicAnimationComponent::UpdateBoneHierarchy: Invalid Bone Entity ID {}!", std::to_string(bone_entity_uuid));
			return;
		}

		auto& transform_component = bone_entity.GetComponent<TransformComponent>();

		// Interpolate only when necessary
		glm::vec3 position = InterpolatePositionTransform(animation_clip, current_bone.BoneName, time);
		glm::quat rotation = InterpolateRotationTransform(animation_clip, current_bone.BoneName, time);
		glm::vec3 scale = InterpolateScaleTransform(animation_clip, current_bone.BoneName, time);

		bool position_changed = (position != glm::vec3(FLT_MAX));
		bool rotation_changed = (rotation != glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX));
		bool scale_changed = (scale != glm::vec3(FLT_MAX));

		if (!position_changed && !rotation_changed && !scale_changed)
		{
			// Skip if there's no change
			return;
		}

		if (!position_changed) position = transform_component.GetLocalPosition();
		if (!rotation_changed) rotation = transform_component.GetLocalRotation();
		if (!scale_changed) scale = transform_component.GetLocalScale();

		// Apply transformation only if it's necessary
		transform_update_map[bone_entity_uuid] =
			glm::translate(glm::mat4(1.0f), position) *
			glm::mat4_cast(rotation) *
			glm::scale(glm::mat4(1.0f), scale);

		// Recursively process children
		for (const BoneLayout& child : current_bone.BoneChildren)
		{
			UpdateBoneHierarchyDeferred(transform_update_map, child, skeleton_asset, bone_mapping, animation_clip, time);
		}
	}

	glm::vec3 BasicAnimationComponent::InterpolatePositionTransform(const AnimationClip& animation_clip, const std::string& bone_name, float time)
	{
		// Find keyframes for this bone
		const auto& bone_keyframes = animation_clip.GetBonePositionKeyframes(bone_name);
		if (bone_keyframes.size() > 1)
		{
			// Locate the two keyframes to interpolate between
			Keyframe_Position previous_key = bone_keyframes.front();
			Keyframe_Position next_key = bone_keyframes.back();
			for (size_t i = 0; i < bone_keyframes.size() - 1; i++)
			{
				if (bone_keyframes[i].Time <= time && bone_keyframes[i + 1].Time > time)
				{
					previous_key = bone_keyframes[i];
					next_key = bone_keyframes[i + 1];
					break;
				}
			}

			// Prevent INF Errors
			if (previous_key.Time == next_key.Time) return glm::vec3(FLT_MAX);

			// Determine Interpolation Factor
			float interpolation_factor = (time - previous_key.Time) / (next_key.Time - previous_key.Time);

			// Interpolate Translation, Rotation, and Scale
			return glm::mix(previous_key.Position, next_key.Position, interpolation_factor);
		}
		else if (bone_keyframes.size() == 1)
		{
			return bone_keyframes.back().Position;
		}

		return glm::vec3(FLT_MAX);
	}

	glm::quat BasicAnimationComponent::InterpolateRotationTransform(const AnimationClip& animation_clip, const std::string& bone_name, float time)
	{
		// Find keyframes for this bone
		const auto& bone_keyframes = animation_clip.GetBoneRotationKeyframes(bone_name);
		if (bone_keyframes.size() > 1)
		{
			// Locate the two keyframes to interpolate between
			Keyframe_Rotation previous_key = bone_keyframes.front();
			Keyframe_Rotation next_key = bone_keyframes.back();
			for (size_t i = 0; i < bone_keyframes.size() - 1; i++)
			{
				if (bone_keyframes[i].Time <= time && bone_keyframes[i + 1].Time > time)
				{
					previous_key = bone_keyframes[i];
					next_key = bone_keyframes[i + 1];
					break;
				}
			}

			// Prevent INF Errors
			if(previous_key.Time == next_key.Time) return glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

			// Determine Interpolation Factor
			float interpolation_factor = (time - previous_key.Time) / (next_key.Time - previous_key.Time);

			// Interpolate Translation, Rotation, and Scale
			return glm::slerp(previous_key.Rotation, next_key.Rotation, interpolation_factor);
		}
		else if (bone_keyframes.size() == 1)
		{
			return bone_keyframes.back().Rotation;
		}

		return glm::quat(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	}

	glm::vec3 BasicAnimationComponent::InterpolateScaleTransform(const AnimationClip& animation_clip, const std::string& bone_name, float time)
	{
		// Find keyframes for this bone
		const auto& bone_keyframes = animation_clip.GetBoneScaleKeyframes(bone_name);
		if (bone_keyframes.size() > 1)
		{
			// Locate the two keyframes to interpolate between
			Keyframe_Scale previous_key = bone_keyframes.front();
			Keyframe_Scale next_key = bone_keyframes.back();
			for (size_t i = 0; i < bone_keyframes.size() - 1; i++)
			{
				if (bone_keyframes[i].Time <= time && bone_keyframes[i + 1].Time > time)
				{
					previous_key = bone_keyframes[i];
					next_key = bone_keyframes[i + 1];
					break;
				}
			}

			// Prevent INF Errors
			if (previous_key.Time == next_key.Time) return glm::vec3(FLT_MAX);

			// Determine Interpolation Factor
			float interpolation_factor = (time - previous_key.Time) / (next_key.Time - previous_key.Time);

			// Interpolate Translation, Rotation, and Scale
			return glm::mix(previous_key.Scale, next_key.Scale, interpolation_factor);
		}
		else if (bone_keyframes.size() == 1)
		{
			return bone_keyframes.back().Scale;
		}

		return glm::vec3(FLT_MAX);
	}

#pragma endregion

}
