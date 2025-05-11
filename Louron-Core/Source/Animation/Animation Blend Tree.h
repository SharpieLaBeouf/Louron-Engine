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
        enum class TreeType : uint8_t
        {
            OneDimensional, 
            TwoDimensional
        } BlendType = TreeType::OneDimensional;

        struct MotionBase
        {
            enum class MotionType : uint8_t
            {
                AnimationClip, 
                BlendTree
            };
            virtual MotionType GetType() = 0;
        };

        struct MotionAnimation : public MotionBase
        {

        };

        struct MotionBlendTree : public MotionBase
        {

        };

        void Evaluate();

        std::vector<MotionBase> ChildNode;
    };

    struct BlendTree
    {
        void Evaluate() { RootNode.Evaluate(); }

        BlendNode RootNode;
    };

}