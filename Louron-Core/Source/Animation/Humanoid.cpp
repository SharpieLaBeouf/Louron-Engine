#include "Humanoid.h"

#include "Skeleton.h"
#include "../Asset/Asset Manager API.h"

#include <yaml-cpp/yaml.h>

namespace Louron
{

#pragma region Humanoid

    std::unordered_map<HumanoidBoneTypes, HumanoidBoneTypes> Humanoid::s_ParentBoneMap =
    {
        { HumanoidBoneTypes::Hips,              HumanoidBoneTypes::Count },
        { HumanoidBoneTypes::Spine,             HumanoidBoneTypes::Hips },
        { HumanoidBoneTypes::LowerChest,        HumanoidBoneTypes::Spine },
        { HumanoidBoneTypes::UpperChest,        HumanoidBoneTypes::LowerChest },

        // Head
        { HumanoidBoneTypes::Neck,              HumanoidBoneTypes::UpperChest },
        { HumanoidBoneTypes::Head,              HumanoidBoneTypes::Neck },
        { HumanoidBoneTypes::Jaw,               HumanoidBoneTypes::Head },
        { HumanoidBoneTypes::LeftEye,           HumanoidBoneTypes::Head },
        { HumanoidBoneTypes::RightEye,          HumanoidBoneTypes::Head },

        // Arms
        { HumanoidBoneTypes::LeftShoulder,      HumanoidBoneTypes::UpperChest },
        { HumanoidBoneTypes::LeftUpperArm,      HumanoidBoneTypes::LeftShoulder },
        { HumanoidBoneTypes::LeftLowerArm,      HumanoidBoneTypes::LeftUpperArm },
        { HumanoidBoneTypes::LeftHand,          HumanoidBoneTypes::LeftLowerArm },

        { HumanoidBoneTypes::RightShoulder,     HumanoidBoneTypes::UpperChest },
        { HumanoidBoneTypes::RightUpperArm,     HumanoidBoneTypes::RightShoulder },
        { HumanoidBoneTypes::RightLowerArm,     HumanoidBoneTypes::RightUpperArm },
        { HumanoidBoneTypes::RightHand,         HumanoidBoneTypes::RightLowerArm },

        // Legs
        { HumanoidBoneTypes::LeftUpperLeg,      HumanoidBoneTypes::Hips },
        { HumanoidBoneTypes::LeftLowerLeg,      HumanoidBoneTypes::LeftUpperLeg },
        { HumanoidBoneTypes::LeftFoot,          HumanoidBoneTypes::LeftLowerLeg },
        { HumanoidBoneTypes::LeftToe,           HumanoidBoneTypes::LeftFoot },

        { HumanoidBoneTypes::RightUpperLeg,     HumanoidBoneTypes::Hips },
        { HumanoidBoneTypes::RightLowerLeg,     HumanoidBoneTypes::RightUpperLeg },
        { HumanoidBoneTypes::RightFoot,         HumanoidBoneTypes::RightLowerLeg },
        { HumanoidBoneTypes::RightToe,          HumanoidBoneTypes::RightFoot },

        // Left Hand Fingers
        { HumanoidBoneTypes::LeftThumb_Upper,   HumanoidBoneTypes::LeftHand },
        { HumanoidBoneTypes::LeftThumb_Middle,  HumanoidBoneTypes::LeftThumb_Upper },
        { HumanoidBoneTypes::LeftThumb_End,     HumanoidBoneTypes::LeftThumb_Middle },

        { HumanoidBoneTypes::LeftIndex_Upper,   HumanoidBoneTypes::LeftHand },
        { HumanoidBoneTypes::LeftIndex_Middle,  HumanoidBoneTypes::LeftIndex_Upper },
        { HumanoidBoneTypes::LeftIndex_End,     HumanoidBoneTypes::LeftIndex_Middle },

        { HumanoidBoneTypes::LeftMiddle_Upper,  HumanoidBoneTypes::LeftHand },
        { HumanoidBoneTypes::LeftMiddle_Middle, HumanoidBoneTypes::LeftMiddle_Upper },
        { HumanoidBoneTypes::LeftMiddle_End,    HumanoidBoneTypes::LeftMiddle_Middle },

        { HumanoidBoneTypes::LeftRing_Upper,    HumanoidBoneTypes::LeftHand },
        { HumanoidBoneTypes::LeftRing_Middle,   HumanoidBoneTypes::LeftRing_Upper },
        { HumanoidBoneTypes::LeftRing_End,      HumanoidBoneTypes::LeftRing_Middle },

        { HumanoidBoneTypes::LeftPinky_Upper,   HumanoidBoneTypes::LeftHand },
        { HumanoidBoneTypes::LeftPinky_Middle,  HumanoidBoneTypes::LeftPinky_Upper },
        { HumanoidBoneTypes::LeftPinky_End,     HumanoidBoneTypes::LeftPinky_Middle },

        // Right Hand Fingers
        { HumanoidBoneTypes::RightThumb_Upper,  HumanoidBoneTypes::RightHand },
        { HumanoidBoneTypes::RightThumb_Middle, HumanoidBoneTypes::RightThumb_Upper },
        { HumanoidBoneTypes::RightThumb_End,    HumanoidBoneTypes::RightThumb_Middle },

        { HumanoidBoneTypes::RightIndex_Upper,  HumanoidBoneTypes::RightHand },
        { HumanoidBoneTypes::RightIndex_Middle, HumanoidBoneTypes::RightIndex_Upper },
        { HumanoidBoneTypes::RightIndex_End,    HumanoidBoneTypes::RightIndex_Middle },

        { HumanoidBoneTypes::RightMiddle_Upper, HumanoidBoneTypes::RightHand },
        { HumanoidBoneTypes::RightMiddle_Middle,HumanoidBoneTypes::RightMiddle_Upper },
        { HumanoidBoneTypes::RightMiddle_End,   HumanoidBoneTypes::RightMiddle_Middle },

        { HumanoidBoneTypes::RightRing_Upper,   HumanoidBoneTypes::RightHand },
        { HumanoidBoneTypes::RightRing_Middle,  HumanoidBoneTypes::RightRing_Upper },
        { HumanoidBoneTypes::RightRing_End,     HumanoidBoneTypes::RightRing_Middle },

        { HumanoidBoneTypes::RightPinky_Upper,  HumanoidBoneTypes::RightHand },
        { HumanoidBoneTypes::RightPinky_Middle, HumanoidBoneTypes::RightPinky_Upper },
        { HumanoidBoneTypes::RightPinky_End,    HumanoidBoneTypes::RightPinky_Middle },
    };

    void Humanoid::AutoConfigure()
    {

    }

    void Humanoid::SetMappedBone(HumanoidBoneTypes bone_type, const std::string &bone_name)
    {
        if (!AssetManager::IsAssetHandleValid(m_SkeletonHandle))
        {
            L_CORE_WARN("Humanoid::SetMappedBone - Could Not Set Bone, Skeleton Handle Invalid.");
            return;
        }

        auto skeleton = AssetManager::GetAsset<Skeleton>(m_SkeletonHandle);
        if (!skeleton)
        {
            L_CORE_ERROR("Humanoid::SetMappedBone - Could Not Set Bone, Could Not Get Skeleton.");
            return;
        }

        auto bone_ptr = skeleton->SkeletonLayout.find(bone_name);
        if (!bone_ptr)
        {
            L_CORE_WARN("Humanoid::SetMappedBone - Could Not Set Bone, Skeleton Does Not Contain '{}' Bone Name.", bone_name);
            return;
        }

        // Handle Root Bone Separately
        if (s_ParentBoneMap[bone_type] == HumanoidBoneTypes::Count)
        {
            m_BoneMapping[bone_type] = bone_name;
            ValidateChildren(bone_type);
            return;
        }

        HumanoidBoneTypes current = bone_type;
        HumanoidBoneTypes mapped_ancestor = HumanoidBoneTypes::Count;
        std::string ancestor_bone_name;

        while (s_ParentBoneMap.contains(current) && s_ParentBoneMap[current] != HumanoidBoneTypes::Count)
        {
            current = s_ParentBoneMap[current];

            auto it = m_BoneMapping.find(current);
            if (it != m_BoneMapping.end())
            {
                mapped_ancestor = current;
                ancestor_bone_name = it->second;
                break;
            }
        }

        if (mapped_ancestor == HumanoidBoneTypes::Count)
        {
            L_CORE_WARN("Humanoid::SetMappedBone - Cannot map '{}' yet. No parent chain root is set.",
                        Utils::BoneTypeToString(bone_type));
            return;
        }

        BoneLayout* ancestor_ptr = skeleton->SkeletonLayout.find(ancestor_bone_name);
        if (!ancestor_ptr)
        {
            L_CORE_WARN("Humanoid::SetMappedBone - Mapped ancestor '{}' not found in skeleton layout.", ancestor_bone_name);
            return;
        }

        if (!ancestor_ptr->find(bone_name))
        {
            L_CORE_WARN("Humanoid::SetMappedBone - '{}' must be a descendant of '{}', but is not.",
                        bone_name, ancestor_bone_name);
            return;
        }

        // Set the bone
        m_BoneMapping[bone_type] = bone_name;
        ValidateChildren(bone_type);
    }

    void Humanoid::ValidateChildren(HumanoidBoneTypes bone_type)
    {
        if (!AssetManager::IsAssetHandleValid(m_SkeletonHandle))
        {
            L_CORE_WARN("Humanoid::ValidateChildren - Skeleton handle invalid.");
            return;
        }

        auto skeleton = AssetManager::GetAsset<Skeleton>(m_SkeletonHandle);
        if (!skeleton)
        {
            L_CORE_ERROR("Humanoid::ValidateChildren - Could not get skeleton asset.");
            return;
        }

        auto root_it = m_BoneMapping.find(bone_type);
        if (root_it == m_BoneMapping.end())
            return;

        const std::string& root_bone_name = root_it->second;
        BoneLayout* root_bone_ptr = skeleton->SkeletonLayout.find(root_bone_name);
        if (!root_bone_ptr)
        {
            L_CORE_WARN("Humanoid::ValidateChildren - Root bone '{}' not found in skeleton layout.", root_bone_name);
            return;
        }

        std::bitset<static_cast<size_t>(HumanoidBoneTypes::Count)> bones_to_prune;

        std::function<void(HumanoidBoneTypes, BoneLayout*)> gather_invalid_bones;
        gather_invalid_bones = [&](HumanoidBoneTypes current_bone, BoneLayout* current_layout_node)
        {
            for (const auto& [child_bone, parent_bone] : s_ParentBoneMap)
            {
                if (parent_bone != current_bone)
                    continue;

                auto it = m_BoneMapping.find(child_bone);
                if (it == m_BoneMapping.end())
                    continue;

                const std::string& child_bone_name = it->second;

                if (!current_layout_node || !current_layout_node->find(child_bone_name))
                {
                    bones_to_prune.set(static_cast<size_t>(child_bone));
                    gather_invalid_bones(child_bone, nullptr); // recurse children with nullptr to auto include for pruning, if parent is invalid, all children then are invalid
                }
                else
                {
                    gather_invalid_bones(child_bone, current_layout_node);
                }
            }
        };

        gather_invalid_bones(bone_type, root_bone_ptr);

        for (size_t i = 0; i < bones_to_prune.size(); ++i)
        {
            if (bones_to_prune[i])
            {
                HumanoidBoneTypes bone = static_cast<HumanoidBoneTypes>(i);
                const std::string& name = m_BoneMapping[bone];
                L_CORE_WARN("Humanoid::ValidateChildren - Pruning '{}' mapped to '{}'.", Utils::BoneTypeToString(bone), name);
                m_BoneMapping.erase(bone);
            }
        }
    }

    void Humanoid::SetSkeletonHandle(AssetHandle skeleton_handle)
    {
        if (!AssetManager::IsAssetHandleValid(skeleton_handle))
        {
            L_CORE_WARN("Humanoid::SetSkeletonHandle - Invalid Skeleton Handle.");
            return;
        }

        if (m_SkeletonHandle != skeleton_handle)
            m_BoneMapping.clear();

        m_SkeletonHandle = skeleton_handle;        
    }

    void Humanoid::Serialise(YAML::Emitter &out)
    {
        ValidateChildren(HumanoidBoneTypes::Hips);

        out << YAML::Key << "Skeleton Handle" << YAML::Value << m_SkeletonHandle;
        out << YAML::Key << "Humanoid Bone Map" << YAML::Value;
        {
            out << YAML::BeginSeq;

            for (const auto& [bone_type, bone_name] : m_BoneMapping)
            {
                out << YAML::BeginMap;

                out << YAML::Key << "BoneType" << YAML::Value << Utils::BoneTypeToString(bone_type);
                out << YAML::Key << "BoneName" << YAML::Value << bone_name;

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
        }
    }

    void Humanoid::Deserialise(const YAML::Node& data)
    {
        if (data["Skeleton Handle"])
            m_SkeletonHandle = data["Skeleton Handle"].as<uint32_t>();

        if (!data["Humanoid Bone Map"])
            return;

        const auto& bone_array = data["Humanoid Bone Map"];
        for (const auto& node : bone_array)
        {
            if (!node["BoneType"] || !node["BoneName"])
                continue;

            auto bone_type = Utils::BoneTypeFromString(node["BoneType"].as<std::string>());
            auto bone_name = node["BoneName"].as<std::string>();

            if (bone_type != HumanoidBoneTypes::Count)
                m_BoneMapping[bone_type] = bone_name;
        }

        ValidateChildren(HumanoidBoneTypes::Hips);
    }

#pragma endregion

#pragma region Humanoid Mask

    void HumanoidMask::Serialise(YAML::Emitter& out)
    {
        out << YAML::Key << "HumanoidMask" << YAML::Value;
        {
            out << YAML::BeginSeq;

            for (size_t i = 0; i < BoneMask.size(); ++i)
            {
                if (BoneMask.test(i))
                {
                    out << Utils::BoneTypeToString(static_cast<HumanoidBoneTypes>(i));
                }
            }

            out << YAML::EndSeq;
        }
    }

    void HumanoidMask::Deserialise(const YAML::Node& data)
    {
        if (!data["HumanoidMask"])
            return;

        BoneMask.reset();

        for (const auto& node : data["HumanoidMask"])
        {
            HumanoidBoneTypes bone_type = Utils::BoneTypeFromString(node.as<std::string>());
            if (bone_type != HumanoidBoneTypes::Count)
            {
                BoneMask.set(static_cast<size_t>(bone_type));
            }
        }
    }
    
#pragma endregion

}