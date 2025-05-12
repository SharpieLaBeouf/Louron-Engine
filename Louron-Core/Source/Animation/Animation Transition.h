#pragma once

// Louron Core Headers
#include "Animation Base Types.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron::Animation
{
    struct TransitionCondition
    {
        std::string ParameterName;
        StringHash ParameterHash = NULL_UUID;
        ComparisonType Operation;
        float ReferenceThreshold; // Casted to uint32_t and bool
    };

    struct AnimationTransition
    {
        StringHash SourceStateHash = NULL_UUID;
        StringHash DestStateHash = NULL_UUID;

        float TransitionDuration = 0.3f;
        bool HasExitTime = false;
        float ExitTime = 0.0f;
        
        std::vector<TransitionCondition> Conditions;
        
        bool CheckTransitionConditionsValid(const std::unordered_map<StringHash, AnimationParameter> state_params)
        {
            for (auto& condition : Conditions)
            {
                if(condition.ParameterHash == NULL_UUID)
                    condition.ParameterHash = ::Louron::Utils::fnv1a_hash(condition.ParameterName);
                
                auto it = state_params.find(condition.ParameterHash);
                if (it == state_params.end())
                    return false; // Missing parameter

                float param_value = it->second.Value;
                float threshold = condition.ReferenceThreshold;

                // Flip Operations and return false if they do not pass, if all
                // flipped conditions are false, that means all conditions did
                // infact pass and should transition to the next state
                switch(condition.Operation)
                {
                    case ComparisonType::Equal:
                    {
                        if (param_value != threshold) return false;
                        break;
                    }
                    case ComparisonType::NotEqual:
                    {
                        if (param_value == threshold) return false;
                        break;
                    }
                    case ComparisonType::GreaterThan:
                    {
                        if (param_value <= threshold) return false;
                        break;
                    }
                    case ComparisonType::LessThan:
                    {
                        if (param_value >= threshold) return false;
                        break;
                    }
                    case ComparisonType::GreaterThanOrEqual:
                    {
                        if (param_value < threshold) return false;
                        break;
                    }
                    case ComparisonType::LessThanOrEqual:
                    {
                        if (param_value > threshold) return false;
                        break;
                    }
                }
            }
            return true; // All conditions passed == all flipped conditions failed
        }
    };
    
}