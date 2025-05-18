#pragma once

// Louron Core Headers
#include "Animation Base Types.h"

#include "../Asset/Asset.h"

// C++ Standard Library Headers
#include <array>
#include <vector>

// External Vendor Library Headers

namespace YAML
{
    class Emitter;
    class Node;
}

namespace Louron::Animation
{

    struct BlendTree;

    /// -------- MOTION --------
    
    struct MotionBase
    {
        // Constructor
        virtual ~MotionBase() = default;
        
        // Functional
        virtual MotionType GetType() = 0;
        virtual void Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params) = 0;
        virtual void CleanMotion() = 0;
        virtual void EvaluatePose(Louron::AnimationPose& evaluated_pose) = 0;
        
        virtual void Serialise(YAML::Emitter& out) = 0;
        virtual void Deserialise(const YAML::Node& data) = 0;

        // Contribution to Final Blend
        float FinalWeight = 0.0f;

        // X: Used for 1D & 2D
        // Y: Used for 2D ONLY
        glm::vec2 BlendPosition = { 0.0f, 0.0f };

        // Use when you want to step the animation 
        // timer even if there is no contribution
        bool UpdateWhenNoContribution = false;
    };
    
    /// -------- BLEND NODE --------

    struct BlendNode
    {
        // Constructors
        BlendNode() = default;
        ~BlendNode() = default;

        BlendNode(const BlendNode& other);
        BlendNode(BlendNode&& other) = default;
        BlendNode& operator=(const BlendNode& other);
        BlendNode& operator=(BlendNode&& other) = default;
        
        // Functional
        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params);
        void CleanBlendNode();

        void EvaluatePose(Louron::AnimationPose& evaluated_pose);
        
        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data);

        MotionBase* AddMotion(MotionType type);
        // Data
        TreeType BlendType = TreeType::OneDimensional;
        std::vector<std::unique_ptr<MotionBase>> ChildNode;

        // X: Used for 1D & 2D
        // Y: Used for 2D ONLY
        glm::vec2 BlendState = { 0.0f, 0.0f };
        std::array<StringHash, 2> BlendParam = { NULL_UUID, NULL_UUID };

        std::string Name = "Blend Tree Node";
    };

    /// -------- MOTION --------

    // Animation On BlendTree
    struct MotionAnimation : public MotionBase
    {
        // Constructors
        MotionAnimation() = default;
        ~MotionAnimation() = default;

        MotionAnimation(const MotionAnimation& other) = default;
        MotionAnimation(MotionAnimation&& other) = default;
        MotionAnimation& operator=(const MotionAnimation& other) = default;
        MotionAnimation& operator=(MotionAnimation&& other) = default;

        // Functional
        MotionType GetType() override { return MotionType::Clip; }
        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params) override;
        void CleanMotion() override;

        void EvaluatePose(Louron::AnimationPose& evaluated_pose) override;

        void Serialise(YAML::Emitter& out) override;
        void Deserialise(const YAML::Node& data) override;

        // Data
        AssetHandle AnimClipHandle = NULL_UUID;

        bool IsPlaying = false;
        bool IsLooping = false;

        float CurrentTime = 0.0f;
        float PlaybackSpeed = 1.0f;
    };

    // Recursive Blend Tree on BlendTree
    struct MotionBlendTree : public MotionBase
    {
        // Constructors
        MotionBlendTree() = default;
        ~MotionBlendTree() = default;

        MotionBlendTree(const MotionBlendTree& other) = default;
        MotionBlendTree(MotionBlendTree&& other) = default;
        MotionBlendTree& operator=(const MotionBlendTree& other) = default;
        MotionBlendTree& operator=(MotionBlendTree&& other) = default;

        // Functional
        MotionType GetType() override { return MotionType::BlendTree; }
        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params) override { RootNode.Update(ts, state_params); }
        void CleanMotion() override { RootNode.CleanBlendNode(); }

        void EvaluatePose(Louron::AnimationPose& evaluated_pose) override { RootNode.EvaluatePose(evaluated_pose); }

        void Serialise(YAML::Emitter& out) override;
        void Deserialise(const YAML::Node& data) override;
        
        // Data
        BlendNode RootNode;
    };
    
    /// -------- BLEND TREE --------

    struct BlendTree
    {
        // Constructors
        BlendTree() = default;
        ~BlendTree() = default;
        
        BlendTree(const BlendTree& other) = default;
        BlendTree(BlendTree&& other) = default;
        BlendTree& operator=(const BlendTree& other) = default;
        BlendTree& operator=(BlendTree&& other) = default;
        
        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data);

        // Functional
        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params) { RootNode.Update(ts, state_params); }
        void CleanBlendTree() { RootNode.CleanBlendNode(); }
        void EvaluatePose(Louron::AnimationPose& evaluated_pose) { RootNode.EvaluatePose(evaluated_pose); }

        // Data
        BlendNode RootNode;
    };

}