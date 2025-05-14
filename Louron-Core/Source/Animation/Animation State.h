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
        virtual void EvaluatePose(Louron::AnimationPose& evaluated_pose) = 0;

        virtual void Serialise(YAML::Emitter& out) = 0;
        virtual void Deserialise(const YAML::Node& data) = 0;

        std::string Name = "";
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

        void EvaluatePose(Louron::AnimationPose& evaluated_pose) override;
        
        void Serialise(YAML::Emitter& out) override;
        void Deserialise(const YAML::Node& data) override;

        AssetHandle AnimClipHandle = NULL_UUID;

        bool IsPlaying = false;
        bool IsLooping = false;

        float CurrentTime = 0.0f;
        float PlaybackSpeed = 1.0f;
    };
    
    // TODO: Finish Blend Tree Animation State
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

        void EvaluatePose(Louron::AnimationPose& evaluated_pose) override;
        
        void Serialise(YAML::Emitter& out) override;
        void Deserialise(const YAML::Node& data) override;

        BlendNode* GetBlendTree() const 
        {
            if(AnimBlendTree)
                return &AnimBlendTree->RootNode;
            return nullptr;
        }

        std::unique_ptr<BlendTree> AnimBlendTree = std::make_unique<BlendTree>();
    };
}