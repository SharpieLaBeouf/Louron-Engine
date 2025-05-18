#include "Animation Blend Tree.h"

#include "Animations.h"
#include "../Asset/Asset Manager API.h"

#include <yaml-cpp/yaml.h>

namespace Louron::Animation
{

#pragma region Blend Tree

	void BlendTree::Serialise(YAML::Emitter& out)
	{
		RootNode.Serialise(out);
	}

	void BlendTree::Deserialise(const YAML::Node& data)
	{
		RootNode.Deserialise(data);
	}

#pragma endregion

#pragma region BlendNode

    BlendNode::BlendNode(const BlendNode& other)
    {
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

	void BlendNode::Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params)
	{
		// TODO: Find the weight to be stored in ChildNode->Second based on the params

		// 1. Update Blend State's
		if(state_params.contains(BlendParam[0]))
		{
			BlendState.x = state_params.at(BlendParam[0]).Value;
		}
		else if (BlendParam[0] != NULL_UUID)
		{
			L_CORE_WARN("Animation State Machine: Missing State Param (1:{}) for BlendTree.", BlendParam[0]);
		}
		
		if (BlendType == TreeType::TwoDimensionalFreeForm)
		{
			if(state_params.contains(BlendParam[1]))
			{
				BlendState.y = state_params.at(BlendParam[1]).Value;
			}
			else if (BlendParam[1] != NULL_UUID)
			{
				L_CORE_WARN("Animation State Machine: Missing State Param (2:{}) for BlendTree.", BlendParam[1]);
			}
		}

		// 2. Calculate Blend Contribution
		for (auto& motion : ChildNode)
			motion->FinalWeight = 0.0f;
		
		constexpr float constant_epsilon = 0.0001f;

		switch(BlendType)
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
				float total_weight = 0.0f;
			
				for (auto& motion : ChildNode)
				{
					float distance = glm::distance(BlendState, motion->BlendPosition);
					float weight = 1.0f / (distance + constant_epsilon); // Ensure No Division by Zero
					motion->FinalWeight = weight;
					total_weight += weight;
				}
			
				// Normalize
				if (total_weight > 0.0f)
				{
					for (auto& motion : ChildNode)
					{
						motion->FinalWeight /= total_weight;
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
		
			// Only update if contributing
			if (motion->FinalWeight > constant_epsilon || motion->UpdateWhenNoContribution)
			{
				motion->Update(ts, state_params);
			}
		
			++it;
		}

		// L_CORE_TRACE("BlendTree BlendState: ({}, {})", BlendState.x, BlendState.y);
		// for (const auto& motion : ChildNode)
		// 	L_CORE_TRACE("  Motion Blend: ({}, {}), FinalWeight: {}", motion->BlendPosition.x, motion->BlendPosition.y, motion->FinalWeight);
	}

    void BlendNode::CleanBlendNode()
	{
		for (auto it = ChildNode.begin(); it != ChildNode.end(); )
		{
			if (it->get())
			{
				it->get()->CleanMotion();
			}
			else
			{
				it = ChildNode.erase(it);
				continue;
			}
			++it;
		}
	}

	void BlendNode::EvaluatePose(Louron::AnimationPose& evaluated_pose)
	{
		constexpr float constant_epsilon = 0.0001f;

		std::vector<AnimationPose> child_poses(ChildNode.size());
	
		for (size_t i = 0; i < ChildNode.size(); ++i)
		{
			auto& motion = ChildNode[i];
			if (!motion || motion->FinalWeight < constant_epsilon)
				continue;
	
			motion->EvaluatePose(child_poses[i]);
		}
	
		// Blend child poses into out_pose
		for (size_t i = 0; i < ChildNode.size(); ++i)
		{
			auto& motion = ChildNode[i];
			if(!motion) 
				continue;

			float weight = motion ? motion->FinalWeight : 0.0f;
			if (weight <= constant_epsilon && !motion->UpdateWhenNoContribution)
				continue;
	
			for (const auto& [bone_name, transform] : child_poses[i].Pose)
			{
				auto& final_transform = evaluated_pose.Pose[bone_name];
	
				final_transform.Position   	+= transform.Position * weight;
				final_transform.Orientation  = glm::slerp(final_transform.Orientation, transform.Orientation, weight);
				final_transform.Scale      	+= transform.Scale * weight;
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

    void MotionAnimation::Update(float ts, const std::unordered_map<StringHash, AnimationParameter>& state_params)
	{
		auto animation_clip = AssetManager::GetAsset<AnimationClip>(AnimClipHandle);
		if (!animation_clip)
			return;

		if (IsPlaying)
		{
			if (CurrentTime >= animation_clip->GetDuration())
			{
				if (IsLooping)
				{
					CurrentTime = 0.0f;
				}
				else
				{
					IsPlaying = false;
					CurrentTime = 0.0f;
				}
			}
			else
			{    
				CurrentTime += ts * PlaybackSpeed;
			}
		}            
	}

    void MotionAnimation::CleanMotion()
	{
		IsPlaying = false;
		CurrentTime = 0.0f;
	}

	void MotionAnimation::EvaluatePose(Louron::AnimationPose& evaluated_pose)
	{
		auto clip = AssetManager::GetAsset<AnimationClip>(AnimClipHandle);
		if (!clip || !IsPlaying)
			return;
	
		clip->SamplePose(CurrentTime, evaluated_pose);
	}

	void MotionAnimation::Serialise(YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Motion Type" << YAML::Value << Utils::MotionTypeToString(this->GetType());
			out << YAML::Key << "Motion Blend Position X" << YAML::Value << BlendPosition.x;
			out << YAML::Key << "Motion Blend Position Y" << YAML::Value << BlendPosition.y;

			out << YAML::Key << "Asset Handle" << YAML::Value << AnimClipHandle;
			out << YAML::Key << "Should Loop" << YAML::Value << IsLooping;
			out << YAML::Key << "Playback Speed" << YAML::Value << PlaybackSpeed;
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

		if (data["Should Loop"])
			IsLooping = data["Should Loop"].as<bool>();

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