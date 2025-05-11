#pragma once

// Louron Core Headers
#include "../Asset/Asset.h"

// C++ Standard Library Headers
#include <unordered_map>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace Louron::Animation
{

    struct AnimationState;

    using StringHash = uint32_t;
    using ParameterMap = std::unordered_map<StringHash, AnimationParam>;
    using StatesMap = std::unordered_map<StringHash, std::unique_ptr<AnimationState>>;

    union AnimationParam
    {
        // 1D Values
        float value_f;
        bool value_b;
        int32_t value_i;
    };
}