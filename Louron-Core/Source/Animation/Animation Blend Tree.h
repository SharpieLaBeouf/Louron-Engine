#pragma once

// Louron Core Headers
#include "Animation Base Types.h"

#include "../Asset/Asset.h"

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers

namespace Louron::Animation
{

    struct BlendTree;

    struct BlendNode
    {
        BlendNode() = default;
        ~BlendNode() = default;

        BlendNode(const BlendNode& other);
        BlendNode(BlendNode&& other) = default;
        BlendNode& operator=(const BlendNode& other);
        BlendNode& operator=(BlendNode&& other) = default;

        enum class TreeType : uint8_t
        {
            OneDimensional, 
            TwoDimensional
        } BlendType = TreeType::OneDimensional;

        struct MotionBase
        {
            virtual MotionType GetType() = 0;

            virtual ~MotionBase() = default;
        };

        struct MotionAnimation : public MotionBase
        {
            MotionAnimation() = default;
            ~MotionAnimation() = default;
    
            MotionAnimation(const MotionAnimation& other) = default;
            MotionAnimation(MotionAnimation&& other) = default;
            MotionAnimation& operator=(const MotionAnimation& other) = default;
            MotionAnimation& operator=(MotionAnimation&& other) = default;

			MotionType GetType() override { return MotionType::Clip; }

        };

        struct MotionBlendTree : public MotionBase
        {
            MotionBlendTree() = default;
            ~MotionBlendTree() = default;
    
            MotionBlendTree(const MotionBlendTree& other) = default;
            MotionBlendTree(MotionBlendTree&& other) = default;
            MotionBlendTree& operator=(const MotionBlendTree& other) = default;
            MotionBlendTree& operator=(MotionBlendTree&& other) = default;

			MotionType GetType() override { return MotionType::BlendTree; }
        };

        void EvaluatePose();

        std::vector<std::unique_ptr<MotionBase>> ChildNode;
    };

    struct BlendTree
    {
        BlendTree() = default;
        ~BlendTree() = default;
        
        BlendTree(const BlendTree& other) = default;
        BlendTree(BlendTree&& other) = default;
        BlendTree& operator=(const BlendTree& other) = default;
        BlendTree& operator=(BlendTree&& other) = default;

        void Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params);
        void CleanBlendTree();

        void EvaluatePose() { RootNode.EvaluatePose(); }

        BlendNode RootNode;
    };

}