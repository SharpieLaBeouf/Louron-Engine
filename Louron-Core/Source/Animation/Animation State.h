#pragma once

// Louron Core Headers
#include "Animation Base Types.h"
#include "Animation Blend Tree.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers

namespace Louron::Animation
{

    enum class StateType : uint8_t
    {
        Empty,
        Animation,
        BlendTree
    };

    struct AnimationState
    {
        virtual StateType GetType() = 0;
        virtual void Evaluate() = 0;

        std::string Name;
    };

    struct AnimationState_Empty : public AnimationState
    {
        StateType GetType() override { return StateType::Empty; }        
        void Evaluate() override { }
    };

    struct AnimationState_Animation : public AnimationState
    {
        StateType GetType() override { return StateType::Animation; }        
        void Evaluate() override { /* TODO: Implement Simple Animation State */}

        AssetHandle AnimClipHandle = NULL_UUID;
        bool IsPlaying = false;
        bool IsLooping = false;

        float CurrentTime = 0.0f;
        float PlaybackSpeed = 1.0f;
    };
    
    struct AnimationState_BlendTree : public AnimationState
    {
        StateType GetType() override { return StateType::BlendTree; }
        void Evaluate() override { AnimBlendTree.Evaluate(); }

        BlendTree AnimBlendTree;
    };
}