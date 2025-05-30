#pragma once

// Louron Core Headers
#include "Animation Base Types.h"
#include "Animation Blend Tree.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers

namespace YAML
{
    class Emitter;
    class Node;
}

namespace Louron::Animation
{
    struct AnimationState
    {
        virtual ~AnimationState() = default;

        virtual StateType GetType() = 0;
        virtual void Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params) = 0;
        virtual void CleanState() = 0;
        virtual std::unique_ptr<AnimationState> Clone() const = 0;
        virtual void EvaluatePose(Louron::AnimationPose& evaluated_pose, bool additive = false) = 0;

        virtual void Serialise(YAML::Emitter& out) = 0;
        virtual void Deserialise(const YAML::Node& data) = 0;

        // Name of the State
        std::string Name = "";

        // This is the normalised time of the Animation/Blend Tree completion.
        // 0.0f == Start of Animation / Blend Tree
        // 1.0f == End of Animation / Blend Tree
        float NormalisedStateTime = 0.0f;
    };

    struct AnimationState_Clip : public AnimationState
    {
        StateType GetType() override { return StateType::Clip; }
        
        AnimationState_Clip() = default;
        ~AnimationState_Clip() = default;

        AnimationState_Clip(const AnimationState_Clip& other) = default;
        AnimationState_Clip(AnimationState_Clip&& other) = default;
        AnimationState_Clip& operator=(const AnimationState_Clip& other) = default;
        AnimationState_Clip& operator=(AnimationState_Clip&& other) = default;

        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params) override;
        void CleanState() override;
        std::unique_ptr<AnimationState> Clone() const override { return std::make_unique<AnimationState_Clip>(*this); }

        void EvaluatePose(Louron::AnimationPose& evaluated_pose, bool additive = false) override;
        
        void Serialise(YAML::Emitter& out) override;
        void Deserialise(const YAML::Node& data) override;

        AssetHandle AnimClipHandle = NULL_UUID;
        AssetHandle ReferenceClipHandle = NULL_UUID;    // When Layer is Additive - this is the animation clip that will be used as a reference. If this is not set, it will use frame 0 of the AnimClipHandle as the reference.
        uint32_t ReferencePoseFrame = 0;                // When Layer is Additive - this is the frame of the reference clip handle to use as the reference pose when solving delta
        
        float PlaybackSpeed = 1.0f;

        bool IsPlaying = false;
        bool IsLooping = false;
    };
    
    struct AnimationState_BlendTree : public AnimationState
    {
        StateType GetType() override { return StateType::BlendTree; }
        
        AnimationState_BlendTree() = default;
        ~AnimationState_BlendTree() = default;
        
        AnimationState_BlendTree(const AnimationState_BlendTree& other);
        AnimationState_BlendTree(AnimationState_BlendTree&& other) = default;

        AnimationState_BlendTree& operator=(const AnimationState_BlendTree& other);
        AnimationState_BlendTree& operator=(AnimationState_BlendTree&& other) = default;

        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params) override;
        void CleanState() override;
        std::unique_ptr<AnimationState> Clone() const override { return std::make_unique<AnimationState_BlendTree>(*this); }

        void EvaluatePose(Louron::AnimationPose& evaluated_pose, bool additive = false) override;
        
        void Serialise(YAML::Emitter& out) override;
        void Deserialise(const YAML::Node& data) override;

        BlendNode& GetBlendTree() { return AnimBlendTree; }

        BlendNode AnimBlendTree;
    };
}