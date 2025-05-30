#include "Animation Blend Tree.h"

#include "Animations.h"
#include "../Asset/Asset Manager API.h"

#include <yaml-cpp/yaml.h>

#include "../Core/Engine.h"

#include <numeric>

namespace Louron::Animation
{

#pragma region BlendNode

    BlendNode::BlendNode(const BlendNode& other)
    {
		BlendType = other.BlendType;

		BlendState = other.BlendState;
		BlendParam = other.BlendParam;

		Name = other.Name;

        ChildNode.clear();
        for (const auto& motion : other.ChildNode)
        {
            if(!motion)
				continue;

			switch (motion->GetType())
			{
			    case MotionType::Clip:
			    {
				    ChildNode.push_back(std::make_unique<MotionAnimation>(*static_cast<MotionAnimation*>(motion.get())));
				    break;
			    }
			    case MotionType::BlendTree:
			    {
				    ChildNode.push_back(std::make_unique<MotionBlendTree>(*static_cast<MotionBlendTree*>(motion.get())));
				    break;
			    }
			}
        }
    }

    BlendNode& BlendNode::operator=(const BlendNode &other)
    {
        if (this == &other)
            return *this;

		BlendType = other.BlendType;

		BlendState = other.BlendState;
		BlendParam = other.BlendParam;

		Name = other.Name;

		ChildNode.clear();
		for (const auto& motion : other.ChildNode)
		{
			if (!motion)
				continue;

			switch (motion->GetType())
			{
			case MotionType::Clip:
			{
				ChildNode.push_back(std::make_unique<MotionAnimation>(*static_cast<MotionAnimation*>(motion.get())));
				break;
			}
			case MotionType::BlendTree:
			{
				ChildNode.push_back(std::make_unique<MotionBlendTree>(*static_cast<MotionBlendTree*>(motion.get())));
				break;
			}
			}
		}

        return *this;
    }

	void BlendNode::Update(float ts, float& normalised_time, const std::unordered_map<StringHash, AnimationParameter>& state_params)
	{
		// 1. Update Blend Parameters
		if (state_params.contains(BlendParam[0]))
			BlendState.x = state_params.at(BlendParam[0]).Value;
		else if (BlendParam[0] != NULL_UUID)
			L_CORE_WARN("Animation State Machine: Missing State Param (1:{}) for BlendTree.", BlendParam[0]);
	
		if (BlendType == TreeType::TwoDimensionalFreeForm)
		{
			if (state_params.contains(BlendParam[1]))
				BlendState.y = state_params.at(BlendParam[1]).Value;
			else if (BlendParam[1] != NULL_UUID)
				L_CORE_WARN("Animation State Machine: Missing State Param (2:{}) for BlendTree.", BlendParam[1]);
		}
	
		// 2. Calculate Blend Contributions
		for (auto& motion : ChildNode)
			motion->FinalWeight = 0.0f;
	
		constexpr float constant_epsilon = 0.0001f;
	
		switch (BlendType)
		{
			case TreeType::OneDimensional:
			{
				// Find two nearest motions by BlendState.x
				MotionBase* lower = nullptr;
				MotionBase* upper = nullptr;
			
				float lower_val = -std::numeric_limits<float>::infinity();
				float upper_val = std::numeric_limits<float>::infinity();
			
				for (auto& motion : ChildNode)
				{
					float x = motion->BlendPosition.x;
			
					if (x <= BlendState.x && x > lower_val)
					{
						lower = motion.get();
						lower_val = x;
					}
			
					if (x >= BlendState.x && x < upper_val)
					{
						upper = motion.get();
						upper_val = x;
					}
				}
			
				if (lower && upper && lower != upper)
				{
					float range = upper_val - lower_val;
					if (range > constant_epsilon)
					{
						float t = (BlendState.x - lower_val) / range;
						lower->FinalWeight = 1.0f - t;
						upper->FinalWeight = t;
					}
					else
					{
						lower->FinalWeight = 0.5f;
						upper->FinalWeight = 0.5f;
					}
				}
				else if (lower)
				{
					lower->FinalWeight = 1.0f;
				}
				else if (upper)
				{
					upper->FinalWeight = 1.0f;
				}

				break;
			}
			case TreeType::TwoDimensionalFreeForm:
			{	
				float blend_mag = glm::length(BlendState);
				glm::vec2 blend_dir = (blend_mag > constant_epsilon) ? glm::normalize(BlendState) : glm::vec2(0.0f);
				float total_weight = 0.0f;
	
				for (auto& motion : ChildNode)
				{
					if (!motion) continue;
	
					float motion_mag = motion->Magnitude;
	
					float angle_weight = 1.0f;
					float mag_weight = 1.0f;
	
					if (motion_mag < constant_epsilon)
					{
						// Idle motion at origin
						mag_weight = glm::clamp(1.0f - blend_mag, 0.0f, 1.0f);
						mag_weight *= mag_weight * mag_weight; // cubic falloff
					}
					else
					{
						glm::vec2 motion_dir = glm::normalize(motion->BlendPosition);
						angle_weight = glm::clamp(glm::dot(blend_dir, motion_dir), 0.0f, 1.0f);
						angle_weight *= angle_weight;
	
						float mag_diff = std::abs(blend_mag - motion_mag);
						mag_weight = glm::clamp(1.0f - mag_diff, 0.0f, 1.0f);
						mag_weight *= mag_weight * mag_weight; // cubic falloff
					}
	
					float weight = angle_weight * mag_weight;
					motion->FinalWeight = weight;
					total_weight += weight;
				}
	
				// Normalize weights
				if (total_weight > constant_epsilon)
				{
					for (auto& motion : ChildNode)
						motion->FinalWeight /= total_weight;
				}
				else
				{
					// Fallback to Idle (origin motion)
					for (auto& motion : ChildNode)
					{
						if (motion->Magnitude < constant_epsilon)
						{
							motion->FinalWeight = 1.0f;
							break;
						}
					}
				}
	
				break;
			}
		}
	
		// 3. Step Animation Timers
		for (auto it = ChildNode.begin(); it != ChildNode.end(); )
		{
			MotionBase* motion = it->get();
			if (!motion)
			{
				it = ChildNode.erase(it);
				L_CORE_ERROR("Animation State Machine: Child Node Is Null!");
				continue;
			}
	
			// No Need to Update Animation Clips as the normalised time is
			// sampled from the parent blend trees normalised time, so motion
			// animation need not update any internal timings, only child blend trees
			if (motion->GetType() == MotionType::BlendTree)
				motion->Update(ts, state_params); // Update Child BLend Trees
	
			++it;
		}
	
		// 4. BlendTree Duration & Time Step	
		float weighted_duration = 0.0f;
		float total_weight = 0.0f;
	
		for (auto& motion : ChildNode)
		{
			if (!motion || motion->FinalWeight < 0.0001f)
				continue;
	
			float duration_seconds = 1.0f;
	
			if (motion->GetType() == MotionType::Clip)
			{
				auto* clip_motion = static_cast<MotionAnimation*>(motion.get());
				auto clip = AssetManager::GetAsset<AnimationClip>(clip_motion->AnimClipHandle);
				if (clip && clip->GetTicksPerSecond() > 0)
					duration_seconds = clip->GetDuration() / static_cast<float>(clip->GetTicksPerSecond());
			}
			else if (motion->GetType() == MotionType::BlendTree)
			{
				duration_seconds = 1.0f; // optional: support nested duration
			}
	
			weighted_duration += motion->FinalWeight * duration_seconds;
			total_weight += motion->FinalWeight;
		}
	
		if (total_weight > 0.0f)
			weighted_duration /= total_weight;
		else
			weighted_duration = 1.0f;
	
		float step = ts / weighted_duration;
		normalised_time = glm::mod<float>(normalised_time + step, 1.0f);
	}

    void BlendNode::CleanBlendNode()
	{
		for (auto it = ChildNode.begin(); it != ChildNode.end(); )
		{
			if (it->get())
			{
				if (it->get()->GetType() == MotionType::BlendTree)
				{
					auto motion_blend = reinterpret_cast<MotionBlendTree*>(it->get());
					motion_blend->NormalisedMotionTime = 0.0f;
					motion_blend->RootNode.CleanBlendNode();
				}
			}
			else
			{
				it = ChildNode.erase(it);
				continue;
			}
			++it;
		}
	}

	void BlendNode::EvaluatePose(Louron::AnimationPose& evaluated_pose, float normalised_time, bool additive)
	{
		constexpr float constant_epsilon = 0.0001f;

		std::vector<AnimationPose> child_poses(ChildNode.size());
	
		for (size_t i = 0; i < ChildNode.size(); ++i)
		{
			auto& motion = ChildNode[i];
			if (!motion || motion->FinalWeight < constant_epsilon)
				continue;
	
			if (motion->GetType() == MotionType::Clip)
				motion->EvaluatePose(child_poses[i], normalised_time, additive); // Use This Blend Node's Normalised Time
			else if (motion->GetType() == MotionType::BlendTree)
				motion->EvaluatePose(child_poses[i], reinterpret_cast<MotionBlendTree*>(motion.get())->NormalisedMotionTime, additive); // Use Child BLend Tree's Normalised Time
		}
		
		std::vector<size_t> sorted_indices(ChildNode.size());
		std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
		
		std::sort(sorted_indices.begin(), sorted_indices.end(),
			[&](size_t a, size_t b)
			{
				return (ChildNode[a] && ChildNode[b]) ?
					(ChildNode[a]->FinalWeight > ChildNode[b]->FinalWeight) : false;
			});

		// Blend child poses into out_pose
		for (size_t index : sorted_indices)
		{
			auto& motion = ChildNode[index];
			if (!motion) continue;
		
			float weight = motion->FinalWeight;
			if (weight <= constant_epsilon && !motion->UpdateWhenNoContribution)
				continue;
		
			for (auto& [bone, xf] : child_poses[index].Pose)
			{
				if (evaluated_pose.Pose.contains(bone))
				{
					auto& out = evaluated_pose.Pose[bone];
					out.Position += xf.Position * weight;
					out.Scale = glm::mix(out.Scale, xf.Scale, weight);
					out.Orientation = glm::slerp(out.Orientation, xf.Orientation, weight);
				}
				else
				{
					auto& out = evaluated_pose.Pose[bone];
					out.Position = xf.Position * weight;
					out.Scale = xf.Scale;
					out.Orientation = xf.Orientation;
				}
			}
		}
	}

	void BlendNode::Serialise(YAML::Emitter& out)
	{
		out << YAML::Key << "Blend Tree Name" << YAML::Value << Name;
		out << YAML::Key << "Blend Tree Type" << YAML::Value << Utils::TreeTypeToString(BlendType);

		out << YAML::Key << "Blend Param X" << YAML::Value << BlendParam[0];
		out << YAML::Key << "Blend Param Y" << YAML::Value << BlendParam[1];

		out << YAML::Key << "Motions" << YAML::Value;
		{
			out << YAML::BeginSeq;

			for (const auto& motion : ChildNode)
			{
				motion->Serialise(out);
			}

			out << YAML::EndSeq;
		}
	}

	void BlendNode::Deserialise(const YAML::Node& data)
	{
		if (data["Blend Tree Name"])
			Name = data["Blend Tree Name"].as<std::string>();
		
		if (data["Blend Tree Type"])
			BlendType = Utils::TreeTypeFromString(data["Blend Tree Type"].as<std::string>());
			
		if (data["Blend Param X"])
			BlendParam[0] = data["Blend Param X"].as<StringHash>();
		
		if (data["Blend Param Y"])
			BlendParam[1] = data["Blend Param Y"].as<StringHash>();
			
		if (auto motions_node = data["Motions"]; motions_node && motions_node.IsSequence())
		{
			for(const auto& motion : motions_node)
			{
				MotionType type = Utils::MotionTypeFromString(motion["Motion Type"].as<std::string>());

				switch (type)
				{
					case MotionType::Clip:
					{
						std::unique_ptr<MotionAnimation> new_motion = std::make_unique<MotionAnimation>();
						new_motion->Deserialise(motion);

						ChildNode.push_back(std::move(new_motion));
						break;
					}
					case MotionType::BlendTree:
					{
						std::unique_ptr<MotionBlendTree> new_motion = std::make_unique<MotionBlendTree>();
						new_motion->Deserialise(motion);

						ChildNode.push_back(std::move(new_motion));
						break;
					}
				}
			}

			for (auto& motion : ChildNode)
			{
				if(!motion)
					continue;
				
				motion->Magnitude = glm::length(motion->BlendPosition);
				motion->Angle = std::atan2f(motion->BlendPosition.y, motion->BlendPosition.x);
			}
		}
	}

    MotionBase *BlendNode::AddMotion(MotionType type)
	{
		switch(type)
		{
			case MotionType::Clip:
			{
				ChildNode.push_back(std::make_unique<MotionAnimation>());
				return ChildNode.back().get();
			}
			case MotionType::BlendTree:
			{
				ChildNode.push_back(std::make_unique<MotionBlendTree>());
				return ChildNode.back().get();
			}
		}
		return nullptr;
	}


#pragma endregion

#pragma region Motion Animation

	void MotionAnimation::EvaluatePose(Louron::AnimationPose& evaluated_pose, float normalised_time, bool additive)
	{
		auto animation_clip = AssetManager::GetAsset<AnimationClip>(AnimClipHandle);
		if (!animation_clip)
			return;
	
        AnimationPose current_pose;
		animation_clip->SamplePose(normalised_time * animation_clip->GetDuration(), current_pose);

        if (additive)
        {
            // TODO: CACHE THIS REFERENCE POSE INTO MotionAnimation
            AnimationPose reference_pose;

            auto ref_animation_clip = AssetManager::GetAsset<AnimationClip>(ReferenceClipHandle);
            if (!ref_animation_clip)
            {
                // Additive against current clip if no reference clip provided
                animation_clip->SamplePose(glm::clamp<float>(ReferencePoseFrame, 0.0f, animation_clip->GetDuration()), reference_pose);
                evaluated_pose = AnimationPose::ComputeDelta(current_pose, reference_pose);
            }
            else
            {
                // Additive against reference clip
                ref_animation_clip->SamplePose(glm::clamp<float>(ReferencePoseFrame, 0.0f, ref_animation_clip->GetDuration()), reference_pose);
                evaluated_pose = AnimationPose::ComputeDelta(current_pose, reference_pose);
            }
        }
        else
        {
            evaluated_pose = std::move(current_pose);
        }
	}

	void MotionAnimation::Serialise(YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Motion Type" << YAML::Value << Utils::MotionTypeToString(this->GetType());
			out << YAML::Key << "Motion Blend Position X" << YAML::Value << BlendPosition.x;
			out << YAML::Key << "Motion Blend Position Y" << YAML::Value << BlendPosition.y;

			out << YAML::Key << "Asset Handle" 			<< YAML::Value << AnimClipHandle;

			out << YAML::Key << "Reference Handle"      << YAML::Value << ReferenceClipHandle;
			out << YAML::Key << "Reference Pose Frame"  << YAML::Value << ReferencePoseFrame;

			out << YAML::Key << "Playback Speed" 		<< YAML::Value << PlaybackSpeed;
		}
		out << YAML::EndMap;
	}

	void MotionAnimation::Deserialise(const YAML::Node& data)
	{
		if (data["Motion Blend Position X"])
			BlendPosition.x = data["Motion Blend Position X"].as<float>();

		if (data["Motion Blend Position Y"])
			BlendPosition.y = data["Motion Blend Position Y"].as<float>();

		if (data["Asset Handle"])
			AnimClipHandle = data["Asset Handle"].as<uint32_t>();

        if(data["Reference Handle"])
            ReferenceClipHandle = data["Reference Handle"].as<uint32_t>();

        if(data["Reference Pose Frame"])
            ReferencePoseFrame = data["Reference Pose Frame"].as<uint32_t>();

		if (data["Playback Speed"])
			PlaybackSpeed = data["Playback Speed"].as<float>();
	}

#pragma endregion

#pragma region Motion Blend Tree

	void MotionBlendTree::Serialise(YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Motion Type" << YAML::Value << Utils::MotionTypeToString(this->GetType());
			out << YAML::Key << "Motion Blend Position X" << YAML::Value << BlendPosition.x;
			out << YAML::Key << "Motion Blend Position Y" << YAML::Value << BlendPosition.y;

			out << YAML::Key << "Motion Blend Tree" << YAML::BeginMap;
			{
				RootNode.Serialise(out);
			}
			out << YAML::EndMap;	
		}
		out << YAML::EndMap;		
	}

	void MotionBlendTree::Deserialise(const YAML::Node& data)
	{
		if (data["Motion Blend Position X"])
			BlendPosition.x = data["Motion Blend Position X"].as<float>();

		if (data["Motion Blend Position Y"])
			BlendPosition.y = data["Motion Blend Position Y"].as<float>();

		RootNode.Deserialise(data["Motion Blend Tree"]);
	}

#pragma endregion



}