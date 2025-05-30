#pragma once

// Louron Core Headers
#include "../Core/UUID.h"
#include "../Asset/Asset.h"

#include "../Core/Logging.h"
#include "../Core/Engine.h"

// C++ Standard Library Headers
#include <string>
#include <vector>
#include <unordered_map>

// External Vendor Library Headers
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

namespace Louron
{
    struct AnimationPose
    {
        AnimationPose() = default;
        ~AnimationPose() = default;

        AnimationPose(const AnimationPose& other) = default;
        AnimationPose(AnimationPose&& other) = default;

        AnimationPose& operator=(const AnimationPose& other) = default;
        AnimationPose& operator=(AnimationPose&& other) = default;

        struct AnimationTransform
        {
            glm::vec3 Position;
            glm::quat Orientation;
            glm::vec3 Scale;
        };
        
        // Key = Bone Name
        // Value = Local Transform
        std::unordered_map<std::string, AnimationTransform> Pose = {};

        static AnimationPose ComputeDelta(const AnimationPose& current, const AnimationPose& reference)
        {
            AnimationPose delta;

            for (const auto& [bone_name, current_transform] : current.Pose)
            {
                const auto ref_iter = reference.Pose.find(bone_name);
                if (ref_iter == reference.Pose.end())
                    continue;

                const auto& reference_transform = ref_iter->second;

                AnimationTransform delta_transform;
                delta_transform.Position    = current_transform.Position - reference_transform.Position;
                delta_transform.Orientation = glm::inverse(reference_transform.Orientation) * current_transform.Orientation;
                delta_transform.Scale       = current_transform.Scale - reference_transform.Scale;

                delta.Pose[bone_name] = delta_transform;
            }

            return delta;
        }

    };

    struct Keyframe_Position
    {
        float Time;
        glm::vec3 Position;
    };

    struct Keyframe_Rotation 
    {
        float Time;
        glm::quat Rotation;
    };

    struct Keyframe_Scale 
    {
        float Time;
        glm::vec3 Scale;
    };

    struct BoneKeyframes 
    {
        std::vector<Keyframe_Position>  PositionKeyframes;
        std::vector<Keyframe_Rotation>  RotationKeyframes;
        std::vector<Keyframe_Scale>     ScaleKeyframes;
    };

    using BoneKeyFrameMap = std::unordered_map<std::string, BoneKeyframes>;

    class AnimationClip : public Asset
    {

    public:

        // --- Constructors & Assignment Operators ---

        virtual AssetType GetType() const override { return AssetType::AnimationClip; }

        AnimationClip() = default;
        AnimationClip(const AnimationClip& other) = default;
        AnimationClip(AnimationClip&& other) noexcept = default;

        AnimationClip& operator=(const AnimationClip& other) = default;
        AnimationClip& operator=(AnimationClip&& other) noexcept = default;

        // --- Animation Clip Functions ---

        void AddBoneKeyframes(const std::string& bone_name, const BoneKeyframes& keyframes)
        {
            m_BoneKeyFrames[bone_name] = keyframes;
        }

        void AddBonePositionKeyFrame(const std::string& bone_name, const Keyframe_Position& keyframe) 
        {
            m_BoneKeyFrames[bone_name].PositionKeyframes.push_back(keyframe);
            if (keyframe.Time > m_AnimationDuration) 
            {
                m_AnimationDuration = keyframe.Time;
            }
        }

        void AddBoneRotationKeyFrame(const std::string& bone_name, const Keyframe_Rotation& keyframe)
        {
            m_BoneKeyFrames[bone_name].RotationKeyframes.push_back(keyframe);
            if (keyframe.Time > m_AnimationDuration)
            {
                m_AnimationDuration = keyframe.Time;
            }
        }

        void AddBoneScaleKeyFrame(const std::string& bone_name, const Keyframe_Scale& keyframe)
        {
            m_BoneKeyFrames[bone_name].ScaleKeyframes.push_back(keyframe);
            if (keyframe.Time > m_AnimationDuration)
            {
                m_AnimationDuration = keyframe.Time;
            }
        }

        const std::vector<Keyframe_Position>& GetBonePositionKeyframes(const std::string& bone_name) const
        {
            static const std::vector<Keyframe_Position> emptyKeyframes;
            auto it = m_BoneKeyFrames.find(bone_name);
            return (it != m_BoneKeyFrames.end()) ? it->second.PositionKeyframes : emptyKeyframes;
        }

        const std::vector<Keyframe_Rotation>& GetBoneRotationKeyframes(const std::string& bone_name) const
        {
            static const std::vector<Keyframe_Rotation> emptyKeyframes;
            auto it = m_BoneKeyFrames.find(bone_name);
            return (it != m_BoneKeyFrames.end()) ? it->second.RotationKeyframes : emptyKeyframes;
        }

        const std::vector<Keyframe_Scale>& GetBoneScaleKeyframes(const std::string& bone_name) const
        {
            static const std::vector<Keyframe_Scale> emptyKeyframes;
            auto it = m_BoneKeyFrames.find(bone_name);
            return (it != m_BoneKeyFrames.end()) ? it->second.ScaleKeyframes : emptyKeyframes;
        }

        float GetDuration() const { return m_AnimationDuration; }
        void SetDuration(float duration) { m_AnimationDuration = duration; }

        uint32_t GetTicksPerSecond() const { return m_TicksPerSecond; }
        void SetTicksPerSecond(uint32_t ticks_per_second) { m_TicksPerSecond = ticks_per_second; }

        const BoneKeyFrameMap& GetAllBoneKeyFrames() const { return m_BoneKeyFrames; }
        void SetAllBoneKeyFrames(const BoneKeyFrameMap& bone_key_frames) { m_BoneKeyFrames = bone_key_frames; }

        void SamplePose(float time, AnimationPose& out_pose) const
        {
            for (const auto& [bone_name, keyframes] : m_BoneKeyFrames)
            {
                AnimationPose::AnimationTransform transform{};

                // --- Position ---
                if (!keyframes.PositionKeyframes.empty())
                {
                    transform.Position = InterpolatePosition(keyframes.PositionKeyframes, time);
                }

                // --- Rotation ---
                if (!keyframes.RotationKeyframes.empty())
                {
                    transform.Orientation = InterpolateRotation(keyframes.RotationKeyframes, time);
                }

                // --- Scale ---
                if (!keyframes.ScaleKeyframes.empty())
                {
                    transform.Scale = InterpolateScale(keyframes.ScaleKeyframes, time);
                }

                out_pose.Pose[bone_name] = transform;
            }
        }
        
        glm::vec3 InterpolatePosition(const std::vector<Keyframe_Position>& keys, float time) const
        {
            if (keys.size() == 1)
                return keys.front().Position;

            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                if (time < keys[i + 1].Time)
                {
                    float t = (time - keys[i].Time) / (keys[i + 1].Time - keys[i].Time);
                    return glm::mix(keys[i].Position, keys[i + 1].Position, t);
                }
            }

            return keys.back().Position;
        }

        glm::quat InterpolateRotation(const std::vector<Keyframe_Rotation>& keys, float time) const
        {
            if (keys.size() == 1)
                return keys.front().Rotation;

            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                if (time < keys[i + 1].Time)
                {
                    float t = (time - keys[i].Time) / (keys[i + 1].Time - keys[i].Time);
                    return glm::slerp(keys[i].Rotation, keys[i + 1].Rotation, t);
                }
            }

            return keys.back().Rotation;
        }

        glm::vec3 InterpolateScale(const std::vector<Keyframe_Scale>& keys, float time) const
        {
            if (keys.size() == 1)
                return keys.front().Scale;

            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                if (time < keys[i + 1].Time)
                {
                    float t = (time - keys[i].Time) / (keys[i + 1].Time - keys[i].Time);
                    return glm::mix(keys[i].Scale, keys[i + 1].Scale, t);
                }
            }

            return keys.back().Scale;
        }

    private:

        // --- Data of AnimationClip Class ---

        float m_AnimationDuration = 0.0f;
        uint32_t m_TicksPerSecond = 0;
        BoneKeyFrameMap m_BoneKeyFrames;

    };

}