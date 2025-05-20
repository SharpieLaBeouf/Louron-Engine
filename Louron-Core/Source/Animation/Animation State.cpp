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

        if (IsPlaying)
        {
            if (CurrentTime >= animation_clip->GetDuration())
            {
                if (IsLooping)
                {
                    CurrentTime = 0.0f;
                }
                else
                {
                    IsPlaying = false;
                    CurrentTime = 0.0f;
                }
            }
            else
            {    
                CurrentTime += ts * PlaybackSpeed + animation_clip->GetTicksPerSecond();
            }
        }
    }

    void AnimationState_Clip::CleanState()
    {
        IsPlaying = true;
        CurrentTime = 0.0f;
    }

    void AnimationState_Clip::EvaluatePose(Louron::AnimationPose& evaluated_pose)
    {
		auto clip = AssetManager::GetAsset<AnimationClip>(AnimClipHandle);
		if (!clip || !IsPlaying)
			return;
	
		clip->SamplePose(CurrentTime, evaluated_pose);
    }

    void AnimationState_Clip::Serialise(YAML::Emitter& out)
    {
        out << YAML::Key << "Asset Handle"   << YAML::Value << AnimClipHandle;
        out << YAML::Key << "Should Loop"    << YAML::Value << IsLooping;
        out << YAML::Key << "Playback Speed" << YAML::Value << PlaybackSpeed;
    }
    
    void AnimationState_Clip::Deserialise(const YAML::Node& data)
    {
        if(data["Asset Handle"])
            AnimClipHandle = data["Asset Handle"].as<uint32_t>();

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
        AnimBlendTree.reset();

        if(other.AnimBlendTree)
        {
            AnimBlendTree = std::make_unique<BlendTree>();
            AnimBlendTree->RootNode = other.AnimBlendTree->RootNode;
        }
    }

    AnimationState_BlendTree &AnimationState_BlendTree::operator=(const AnimationState_BlendTree &other)
    {
        if (this == &other)
            return *this;
        
        Name = other.Name;
        AnimBlendTree.reset();

        if(other.AnimBlendTree)
        {
            AnimBlendTree = std::make_unique<BlendTree>(*other.AnimBlendTree.get());
        }

        return *this;
    }
    
    void AnimationState_BlendTree::Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params)
    {
        if(AnimBlendTree)
            AnimBlendTree->Update(ts, state_params);
    }

    void AnimationState_BlendTree::CleanState()
    {
        if(AnimBlendTree)
            AnimBlendTree->CleanBlendTree();
    }

    void AnimationState_BlendTree::EvaluatePose(Louron::AnimationPose& evaluated_pose)
    {
        if(AnimBlendTree)
            AnimBlendTree->EvaluatePose(evaluated_pose);
    }

    void AnimationState_BlendTree::Serialise(YAML::Emitter& out)
    {
        if(AnimBlendTree)
            AnimBlendTree->Serialise(out);
    }
    
    void AnimationState_BlendTree::Deserialise(const YAML::Node& data)
    {
        if(AnimBlendTree)
            AnimBlendTree->Deserialise(data);        
    }

#pragma endregion

}