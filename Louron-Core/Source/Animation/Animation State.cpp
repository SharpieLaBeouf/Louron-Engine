#include "Animation State.h"

#include "Animations.h"

#include "../Asset/Asset Manager API.h"
#include "Animation Blend Tree.h"

#include <yaml-cpp/yaml.h>

namespace Louron::Animation
{

#pragma region Clip

    void AnimationState_Clip::Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params)
    {
        auto animation_clip = AssetManager::GetAsset<AnimationClip>(AnimClipHandle);
        if (!animation_clip)
            return;

        const float duration = animation_clip->GetDuration();
        if (duration <= 0.0f)
            return;

        if (IsPlaying)
        {
            float time_advance = ts * PlaybackSpeed * animation_clip->GetTicksPerSecond();
            float normalised_advance = time_advance / duration;

            NormalisedStateTime += normalised_advance;

            if (NormalisedStateTime >= 1.0f)
            {
                if (IsLooping)
                {
                    NormalisedStateTime = glm::mod<float>(NormalisedStateTime, 1.0f);
                }
                else
                {
                    IsPlaying = false;
                    NormalisedStateTime = 1.0f;
                }
            }
        }
    }

    void AnimationState_Clip::CleanState()
    {
        IsPlaying = true;
        NormalisedStateTime = 0.0f;
    }

    void AnimationState_Clip::EvaluatePose(Louron::AnimationPose& evaluated_pose, bool additive)
    {
		auto animation_clip = AssetManager::GetAsset<AnimationClip>(AnimClipHandle);
		if (!animation_clip)
			return;

        AnimationPose current_pose;
        animation_clip->SamplePose(NormalisedStateTime * animation_clip->GetDuration(), current_pose);

        if (additive)
        {
            // TODO: CACHE THIS REFERENCE POSE INTO AnimationState_Clip
            AnimationPose reference_pose;

            auto ref_animation_clip = AssetManager::GetAsset<AnimationClip>(ReferenceClipHandle);
            if (!ref_animation_clip)
            {
                // Additive against current clip if no reference clip provided
                animation_clip->SamplePose(glm::clamp<float>(ReferencePoseFrame, 0.0f, animation_clip->GetDuration()), reference_pose);
                evaluated_pose = AnimationPose::ComputeDelta(current_pose, reference_pose);
            }
            else
            {
                // Additive against reference clip
                ref_animation_clip->SamplePose(glm::clamp<float>(ReferencePoseFrame, 0.0f, ref_animation_clip->GetDuration()), reference_pose);
                evaluated_pose = AnimationPose::ComputeDelta(current_pose, reference_pose);
            }
        }
        else
        {
            evaluated_pose = std::move(current_pose);
        }
    }

    void AnimationState_Clip::Serialise(YAML::Emitter& out)
    {
        out << YAML::Key << "Asset Handle"          << YAML::Value << AnimClipHandle;

        out << YAML::Key << "Reference Handle"      << YAML::Value << ReferenceClipHandle;
        out << YAML::Key << "Reference Pose Frame"  << YAML::Value << ReferencePoseFrame;

        out << YAML::Key << "Should Loop"           << YAML::Value << IsLooping;
        out << YAML::Key << "Playback Speed"        << YAML::Value << PlaybackSpeed;
    }
    
    void AnimationState_Clip::Deserialise(const YAML::Node& data)
    {
        if(data["Asset Handle"])
            AnimClipHandle = data["Asset Handle"].as<uint32_t>();

        if(data["Reference Handle"])
            ReferenceClipHandle = data["Reference Handle"].as<uint32_t>();

        if(data["Reference Pose Frame"])
            ReferencePoseFrame = data["Reference Pose Frame"].as<uint32_t>();

        if(data["Should Loop"])
            IsLooping = data["Should Loop"].as<bool>();
            
        if(data["Playback Speed"])
            PlaybackSpeed = data["Playback Speed"].as<float>();
    }

#pragma endregion

#pragma region Blend Tree
    
    AnimationState_BlendTree::AnimationState_BlendTree(const AnimationState_BlendTree &other)
    {
        Name = other.Name;
        NormalisedStateTime = other.NormalisedStateTime;
        AnimBlendTree = other.AnimBlendTree;
    }

    AnimationState_BlendTree &AnimationState_BlendTree::operator=(const AnimationState_BlendTree &other)
    {
        if (this == &other)
            return *this;
        
        Name = other.Name;
        NormalisedStateTime = other.NormalisedStateTime;
        AnimBlendTree = other.AnimBlendTree;

        return *this;
    }
    
    void AnimationState_BlendTree::Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params)
    {
        AnimBlendTree.Update(ts, NormalisedStateTime, state_params);
    }

    void AnimationState_BlendTree::CleanState()
    {
        AnimBlendTree.CleanBlendNode();
        NormalisedStateTime = 0.0f;
    }

    void AnimationState_BlendTree::EvaluatePose(Louron::AnimationPose& evaluated_pose, bool additive)
    {
        AnimBlendTree.EvaluatePose(evaluated_pose, NormalisedStateTime, additive);
    }

    void AnimationState_BlendTree::Serialise(YAML::Emitter& out)
    {
        AnimBlendTree.Serialise(out);
    }
    
    void AnimationState_BlendTree::Deserialise(const YAML::Node& data)
    {
        AnimBlendTree.Deserialise(data);        
    }

#pragma endregion

}