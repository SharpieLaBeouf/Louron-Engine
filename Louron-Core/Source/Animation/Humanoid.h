#pragma once

#include <bitset>
#include <string>
#include <unordered_map>

#include "../Asset/Asset.h"

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace Louron
{
    class Skeleton;

    enum class HumanoidBoneTypes : uint8_t
    {
        Hips,
        Spine,
        LowerChest,
        UpperChest,

        // Head

        Neck,
        Head,
        Jaw,
        LeftEye,
        RightEye,

        // Arms

        LeftShoulder,
        LeftUpperArm,
        LeftLowerArm,
        LeftHand,

        RightShoulder,
        RightUpperArm,
        RightLowerArm,
        RightHand,

        // Legs

        LeftUpperLeg,
        LeftLowerLeg,
        LeftFoot,
        LeftToe,
        
        RightUpperLeg,
        RightLowerLeg,
        RightFoot,
        RightToe,

        // Left Hand

        LeftThumb_Upper,
        LeftThumb_Middle,
        LeftThumb_End,
        
        LeftIndex_Upper,
        LeftIndex_Middle,
        LeftIndex_End,
        
        LeftMiddle_Upper,
        LeftMiddle_Middle,
        LeftMiddle_End,
        
        LeftRing_Upper,
        LeftRing_Middle,
        LeftRing_End,
        
        LeftPinky_Upper,
        LeftPinky_Middle,
        LeftPinky_End,
        
        // Right Hand

        RightThumb_Upper,
        RightThumb_Middle,
        RightThumb_End,
        
        RightIndex_Upper,
        RightIndex_Middle,
        RightIndex_End,
        
        RightMiddle_Upper,
        RightMiddle_Middle,
        RightMiddle_End,
        
        RightRing_Upper,
        RightRing_Middle,
        RightRing_End,
        
        RightPinky_Upper,
        RightPinky_Middle,
        RightPinky_End,

        // Count of All Bone Types

        Count
    };

    class Humanoid : public Asset
    {

    public:

        using HumanoidBoneMap = std::unordered_map<HumanoidBoneTypes, std::string>;

        // First = Child Bone
        // Second = Parent Bone (if bone is root / hips, parent will be HumanoidBoneTypes::Count)
        static std::unordered_map<HumanoidBoneTypes, HumanoidBoneTypes> s_ParentBoneMap;
        
		virtual AssetType GetType() const override { return AssetType::Humanoid; }

        void AutoConfigure();
        void SetSkeletonAsset(AssetHandle skeleton_handle) { m_SkeletonHandle = skeleton_handle; }
        void ValidateChildren(HumanoidBoneTypes bone_type);

        const HumanoidBoneMap& GetBoneMap() const { return m_BoneMapping; }
        void SetMappedBone(HumanoidBoneTypes bone_type, const std::string& bone_name);

        AssetHandle GetSkeletonHandle() const { return m_SkeletonHandle; }
        void SetSkeletonHandle(AssetHandle skeleton_handle); 

        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data);

    private:
    
        // Mapping of Skeleton Bone Names to Standardised Bone Types
        // Key = Humanoid Bone
        // Value = Skeleton Bone Name
        HumanoidBoneMap m_BoneMapping = {};

        // Asset handle for the skeleton this Humanoid has been created against.
        AssetHandle m_SkeletonHandle = NULL_UUID;
    };

    class HumanoidMask : public Asset
    {
    
    public:

		virtual AssetType GetType() const override { return AssetType::HumanoidMask; }

        /// <summary>
        /// Bitset Mask for Bones Affected By Mask
        /// </summary>
        std::bitset<static_cast<size_t>(HumanoidBoneTypes::Count)> BoneMask;
        
        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data);

    private:
        
        
    };

    namespace Utils
    {
        static inline HumanoidBoneTypes BoneTypeFromString(const std::string& bone_type_str)
        {
            static const std::unordered_map<std::string, HumanoidBoneTypes> string_to_bone_type = 
            {
                { "Hips", HumanoidBoneTypes::Hips },
                { "Spine", HumanoidBoneTypes::Spine },
                { "LowerChest", HumanoidBoneTypes::LowerChest },
                { "UpperChest", HumanoidBoneTypes::UpperChest },
                { "Neck", HumanoidBoneTypes::Neck },
                { "Head", HumanoidBoneTypes::Head },
                { "Jaw", HumanoidBoneTypes::Jaw },
                { "LeftEye", HumanoidBoneTypes::LeftEye },
                { "RightEye", HumanoidBoneTypes::RightEye },

                { "LeftShoulder", HumanoidBoneTypes::LeftShoulder },
                { "LeftUpperArm", HumanoidBoneTypes::LeftUpperArm },
                { "LeftLowerArm", HumanoidBoneTypes::LeftLowerArm },
                { "LeftHand", HumanoidBoneTypes::LeftHand },

                { "RightShoulder", HumanoidBoneTypes::RightShoulder },
                { "RightUpperArm", HumanoidBoneTypes::RightUpperArm },
                { "RightLowerArm", HumanoidBoneTypes::RightLowerArm },
                { "RightHand", HumanoidBoneTypes::RightHand },

                { "LeftUpperLeg", HumanoidBoneTypes::LeftUpperLeg },
                { "LeftLowerLeg", HumanoidBoneTypes::LeftLowerLeg },
                { "LeftFoot", HumanoidBoneTypes::LeftFoot },
                { "LeftToe", HumanoidBoneTypes::LeftToe },

                { "RightUpperLeg", HumanoidBoneTypes::RightUpperLeg },
                { "RightLowerLeg", HumanoidBoneTypes::RightLowerLeg },
                { "RightFoot", HumanoidBoneTypes::RightFoot },
                { "RightToe", HumanoidBoneTypes::RightToe },

                { "LeftThumb_Upper", HumanoidBoneTypes::LeftThumb_Upper },
                { "LeftThumb_Middle", HumanoidBoneTypes::LeftThumb_Middle },
                { "LeftThumb_End", HumanoidBoneTypes::LeftThumb_End },

                { "LeftIndex_Upper", HumanoidBoneTypes::LeftIndex_Upper },
                { "LeftIndex_Middle", HumanoidBoneTypes::LeftIndex_Middle },
                { "LeftIndex_End", HumanoidBoneTypes::LeftIndex_End },

                { "LeftMiddle_Upper", HumanoidBoneTypes::LeftMiddle_Upper },
                { "LeftMiddle_Middle", HumanoidBoneTypes::LeftMiddle_Middle },
                { "LeftMiddle_End", HumanoidBoneTypes::LeftMiddle_End },

                { "LeftRing_Upper", HumanoidBoneTypes::LeftRing_Upper },
                { "LeftRing_Middle", HumanoidBoneTypes::LeftRing_Middle },
                { "LeftRing_End", HumanoidBoneTypes::LeftRing_End },

                { "LeftPinky_Upper", HumanoidBoneTypes::LeftPinky_Upper },
                { "LeftPinky_Middle", HumanoidBoneTypes::LeftPinky_Middle },
                { "LeftPinky_End", HumanoidBoneTypes::LeftPinky_End },

                { "RightThumb_Upper", HumanoidBoneTypes::RightThumb_Upper },
                { "RightThumb_Middle", HumanoidBoneTypes::RightThumb_Middle },
                { "RightThumb_End", HumanoidBoneTypes::RightThumb_End },

                { "RightIndex_Upper", HumanoidBoneTypes::RightIndex_Upper },
                { "RightIndex_Middle", HumanoidBoneTypes::RightIndex_Middle },
                { "RightIndex_End", HumanoidBoneTypes::RightIndex_End },

                { "RightMiddle_Upper", HumanoidBoneTypes::RightMiddle_Upper },
                { "RightMiddle_Middle", HumanoidBoneTypes::RightMiddle_Middle },
                { "RightMiddle_End", HumanoidBoneTypes::RightMiddle_End },

                { "RightRing_Upper", HumanoidBoneTypes::RightRing_Upper },
                { "RightRing_Middle", HumanoidBoneTypes::RightRing_Middle },
                { "RightRing_End", HumanoidBoneTypes::RightRing_End },

                { "RightPinky_Upper", HumanoidBoneTypes::RightPinky_Upper },
                { "RightPinky_Middle", HumanoidBoneTypes::RightPinky_Middle },
                { "RightPinky_End", HumanoidBoneTypes::RightPinky_End },
            };

            auto it = string_to_bone_type.find(bone_type_str);
            if (it != string_to_bone_type.end())
                return it->second;

            return HumanoidBoneTypes::Count; // Invalid
        }

        static inline std::string BoneTypeToString(HumanoidBoneTypes bone_type)
        {
            switch (bone_type)
            {
                case HumanoidBoneTypes::Hips: return "Hips";
                case HumanoidBoneTypes::Spine: return "Spine";
                case HumanoidBoneTypes::LowerChest: return "LowerChest";
                case HumanoidBoneTypes::UpperChest: return "UpperChest";
                case HumanoidBoneTypes::Neck: return "Neck";
                case HumanoidBoneTypes::Head: return "Head";
                case HumanoidBoneTypes::Jaw: return "Jaw";
                case HumanoidBoneTypes::LeftEye: return "LeftEye";
                case HumanoidBoneTypes::RightEye: return "RightEye";

                case HumanoidBoneTypes::LeftShoulder: return "LeftShoulder";
                case HumanoidBoneTypes::LeftUpperArm: return "LeftUpperArm";
                case HumanoidBoneTypes::LeftLowerArm: return "LeftLowerArm";
                case HumanoidBoneTypes::LeftHand: return "LeftHand";

                case HumanoidBoneTypes::RightShoulder: return "RightShoulder";
                case HumanoidBoneTypes::RightUpperArm: return "RightUpperArm";
                case HumanoidBoneTypes::RightLowerArm: return "RightLowerArm";
                case HumanoidBoneTypes::RightHand: return "RightHand";

                case HumanoidBoneTypes::LeftUpperLeg: return "LeftUpperLeg";
                case HumanoidBoneTypes::LeftLowerLeg: return "LeftLowerLeg";
                case HumanoidBoneTypes::LeftFoot: return "LeftFoot";
                case HumanoidBoneTypes::LeftToe: return "LeftToe";

                case HumanoidBoneTypes::RightUpperLeg: return "RightUpperLeg";
                case HumanoidBoneTypes::RightLowerLeg: return "RightLowerLeg";
                case HumanoidBoneTypes::RightFoot: return "RightFoot";
                case HumanoidBoneTypes::RightToe: return "RightToe";

                case HumanoidBoneTypes::LeftThumb_Upper: return "LeftThumb_Upper";
                case HumanoidBoneTypes::LeftThumb_Middle: return "LeftThumb_Middle";
                case HumanoidBoneTypes::LeftThumb_End: return "LeftThumb_End";

                case HumanoidBoneTypes::LeftIndex_Upper: return "LeftIndex_Upper";
                case HumanoidBoneTypes::LeftIndex_Middle: return "LeftIndex_Middle";
                case HumanoidBoneTypes::LeftIndex_End: return "LeftIndex_End";

                case HumanoidBoneTypes::LeftMiddle_Upper: return "LeftMiddle_Upper";
                case HumanoidBoneTypes::LeftMiddle_Middle: return "LeftMiddle_Middle";
                case HumanoidBoneTypes::LeftMiddle_End: return "LeftMiddle_End";

                case HumanoidBoneTypes::LeftRing_Upper: return "LeftRing_Upper";
                case HumanoidBoneTypes::LeftRing_Middle: return "LeftRing_Middle";
                case HumanoidBoneTypes::LeftRing_End: return "LeftRing_End";

                case HumanoidBoneTypes::LeftPinky_Upper: return "LeftPinky_Upper";
                case HumanoidBoneTypes::LeftPinky_Middle: return "LeftPinky_Middle";
                case HumanoidBoneTypes::LeftPinky_End: return "LeftPinky_End";

                case HumanoidBoneTypes::RightThumb_Upper: return "RightThumb_Upper";
                case HumanoidBoneTypes::RightThumb_Middle: return "RightThumb_Middle";
                case HumanoidBoneTypes::RightThumb_End: return "RightThumb_End";

                case HumanoidBoneTypes::RightIndex_Upper: return "RightIndex_Upper";
                case HumanoidBoneTypes::RightIndex_Middle: return "RightIndex_Middle";
                case HumanoidBoneTypes::RightIndex_End: return "RightIndex_End";

                case HumanoidBoneTypes::RightMiddle_Upper: return "RightMiddle_Upper";
                case HumanoidBoneTypes::RightMiddle_Middle: return "RightMiddle_Middle";
                case HumanoidBoneTypes::RightMiddle_End: return "RightMiddle_End";

                case HumanoidBoneTypes::RightRing_Upper: return "RightRing_Upper";
                case HumanoidBoneTypes::RightRing_Middle: return "RightRing_Middle";
                case HumanoidBoneTypes::RightRing_End: return "RightRing_End";

                case HumanoidBoneTypes::RightPinky_Upper: return "RightPinky_Upper";
                case HumanoidBoneTypes::RightPinky_Middle: return "RightPinky_Middle";
                case HumanoidBoneTypes::RightPinky_End: return "RightPinky_End";

                default: return "Unknown";
            }
        }
    }

}