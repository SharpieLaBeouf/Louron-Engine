#pragma once

// Louron Core Headers
#include "../Asset/Asset.h"

#include "../Core/Utilities.h"

// C++ Standard Library Headers
#include <unordered_map>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Louron
{
    struct AnimationPose;
}

namespace Louron::Animation
{
    using StringHash = uint32_t;

    enum class ComparisonType : uint8_t
    {
        Unknown,
        Equal,
        NotEqual,
        GreaterThan,
        LessThan,
        GreaterThanOrEqual,
        LessThanOrEqual
    };

    enum class StateType : uint8_t 
    {
        Unknown,
        Clip,
        BlendTree
    };
    
    enum class MotionType : uint8_t
    {
        Unknown,
        Clip, 
        BlendTree
    };
    
    enum class TreeType : uint8_t
    {
        Unknown,
        OneDimensional,
        TwoDimensionalFreeForm
    };

    enum class ParameterType : uint8_t
    {
        Unknown,
        Bool,
        Float,
        Int,
        UInt
    };

    struct AnimationParameter
    {
        std::string Name;
        float Value;
        ParameterType Type;
    };

    namespace Utils
    {
        static inline std::string ComparisonTypeToString(ComparisonType type)
        {
            switch(type)
            {
                case ComparisonType::Equal:                 return "Equal";
                case ComparisonType::NotEqual:              return "NotEqual";
                case ComparisonType::GreaterThan:           return "GreaterThan";
                case ComparisonType::LessThan:              return "LessThan";
                case ComparisonType::GreaterThanOrEqual:    return "GreaterThanOrEqual";
                case ComparisonType::LessThanOrEqual:       return "LessThanOrEqual";
            }
            return "Unknown";
        }

        static inline ComparisonType ComparisonTypeFromString(const std::string& type_str)
        {
            if(type_str == "Equal")                 return ComparisonType::Equal;
            if(type_str == "NotEqual")              return ComparisonType::NotEqual;
            if(type_str == "GreaterThan")           return ComparisonType::GreaterThan;
            if(type_str == "LessThan")              return ComparisonType::LessThan;
            if(type_str == "GreaterThanOrEqual")    return ComparisonType::GreaterThanOrEqual;
            if(type_str == "LessThanOrEqual")       return ComparisonType::LessThanOrEqual;
            return ComparisonType::Unknown;
        }

        static inline std::string StateTypeToString(StateType type)
        {
            switch(type)
            {
                case StateType::Clip:      return "Clip";
                case StateType::BlendTree: return "BlendTree";
            }
            return "Unknown";
        }

        static inline StateType StateTypeFromString(const std::string& type_str)
        {
            if(type_str == "Clip")      return StateType::Clip;
            if(type_str == "BlendTree") return StateType::BlendTree;
            return StateType::Unknown;
        }

        static inline std::string MotionTypeToString(MotionType type)
        {
            switch(type)
            {
                case MotionType::Clip:      return "Clip";
                case MotionType::BlendTree: return "BlendTree";
            }
            return "Unknown";
        }

        static inline MotionType MotionTypeFromString(const std::string& type_str)
        {
            if(type_str == "Clip")      return MotionType::Clip;
            if(type_str == "BlendTree")     return MotionType::BlendTree;
            return MotionType::Unknown;
        }
        
        static inline std::string TreeTypeToString(TreeType type)
        {
            switch(type)
            {
                case TreeType::OneDimensional:          return "OneDimensional";
                case TreeType::TwoDimensionalFreeForm:  return "TwoDimensionalFreeForm";
            }
            return "Unknown";
        }

        static inline TreeType TreeTypeFromString(const std::string& type_str)
        {
            if(type_str == "OneDimensional")            return TreeType::OneDimensional;
            if(type_str == "TwoDimensionalFreeForm")    return TreeType::TwoDimensionalFreeForm;
            return TreeType::Unknown;
        }

        static inline std::string ParamTypeToString(ParameterType type)
        {
            switch(type)
            {
                case ParameterType::Bool:       return "Bool";
                case ParameterType::Float:      return "Float";
                case ParameterType::Int:        return "Int";
                case ParameterType::UInt:       return "UInt";
            }
            return "Unknown";
        }

        static inline ParameterType ParamTypeFromString(const std::string& type_str)
        {
            if(type_str == "Bool")      return ParameterType::Bool;
            if(type_str == "Float")     return ParameterType::Float;
            if(type_str == "Int")       return ParameterType::Int;
            if(type_str == "UInt")      return ParameterType::UInt;
            return ParameterType::Unknown;
        }
    }

}