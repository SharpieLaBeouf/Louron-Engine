#pragma once

// Louron Core Headers
#include "Component Base.h"

#include "../../Asset/Asset.h"

// C++ Standard Library Headers
#include <vector>
#include <unordered_map>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace Louron
{

	struct BoneLayout; // Forward Declare
	class AnimationClip;
	class Skeleton;

	struct AnimatorComponent : public ComponentBase
	{
		// --- Constructors & Assignment Operators ---

		AnimatorComponent() = default;
		AnimatorComponent(const AnimatorComponent& other) = default;
		AnimatorComponent(AnimatorComponent&& other) noexcept = default;

		AnimatorComponent& operator=(const AnimatorComponent& other) = default;
		AnimatorComponent& operator=(AnimatorComponent&& other) noexcept = default;

		// --- AnimatorComponent Functions ---

		void Play(int clip_index, bool should_loop = true);

		void Pause() { IsPlaying = false; }
		void Resume() { IsPlaying = true; }
		void Stop() { IsPlaying = false; CurrentTime = 0.0f; }
		void SetPlaybackSpeed(float speed) { PlaybackSpeed = speed; }

		/// <summary>
		/// Use This to Update the Bone Entity Transforms Whilst Iterating Bone Hierarchy
		/// </summary>
		void Update();

		/// <summary>
		/// Use This to Provide a Mapping of Entity's and their New Transforms Based on Animation Updates
		/// </summary>
		std::unordered_map<UUID, glm::mat4> UpdateDeferred();

		/// <summary>
		/// Step the animation timer
		/// </summary>
		void StepAnimationTimer(const std::shared_ptr<AnimationClip>& animation_clip);

		// --- Data of SkinnedMeshComponent Struct ---

		/// <summary>
		/// A Vector of References to Animation Assets
		/// </summary>
		std::vector<AssetHandle> AnimationClipHandles;

		/// <summary>
		/// Index of the Vector to Which Animation is Currently Playing
		/// </summary>
		int CurrentClipIndex = -1;

		/// <summary>
		/// The Current Time of the Animation Being Played
		/// </summary>
		float CurrentTime = 0.0f;

		/// <summary>
		/// The Speed of the Animation Playback
		/// </summary>
		float PlaybackSpeed = 1.0f;

		/// <summary>
		/// Flag - should the animation be playing
		/// </summary>
		bool IsPlaying = false;

		/// <summary>
		/// Flag - should the animation be looping
		/// </summary>
		bool IsLooping = true;

		enum class AnimationCullingMode : uint8_t
		{
			AlwaysAnimate = 0,					// Animations are procesed 
			NoAnimateOffScreenContinueTimer,	// No Animations are processed - animation timer is continued
			NoAnimateOffScreenStopTimer			// No Animations are processed - animation timer is stopped
		};

		AnimationCullingMode CullingMode = AnimationCullingMode::AlwaysAnimate;

		void Serialize(YAML::Emitter& out);
		bool Deserialize(const YAML::Node data);

	private:

		// --- Helper Functions ---

		void UpdateBoneHierarchy(const BoneLayout& current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<UUID, UUID>& bone_mapping, const AnimationClip& animation_clip, float time);
		void UpdateBoneHierarchyDeferred(std::unordered_map<UUID, glm::mat4>& transform_update_map, const BoneLayout& current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<UUID, UUID>& bone_mapping, const AnimationClip& animation_clip, float time);

		glm::vec3 InterpolatePositionTransform(const AnimationClip& animation_clip, const std::string& bone_name, float time);
		glm::quat InterpolateRotationTransform(const AnimationClip& animation_clip, const std::string& bone_name, float time);
		glm::vec3 InterpolateScaleTransform(const AnimationClip& animation_clip, const std::string& bone_name, float time);

		float GetInterpolationFactor(float last_key_time, float next_key_time, float total_animation_time)
		{
			float interpolation_factor = 0.0f;
			float mid_way_length = total_animation_time - last_key_time;
			float frames_diff = next_key_time - last_key_time;
			interpolation_factor = mid_way_length / frames_diff;
			return interpolation_factor;
		}
	};

}