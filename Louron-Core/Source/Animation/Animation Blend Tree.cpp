#include "Animation Blend Tree.h"

#include "Animations.h"
#include "../Asset/Asset Manager API.h"

namespace Louron::Animation
{

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
		else
		{
			L_CORE_WARN("Animation State Machine: Missing State Param (1:{}) for BlendTree.", BlendParam[0]);
		}
		
		if (BlendType == TreeType::TwoDimensionalFreeForm)
		{
			if(state_params.contains(BlendParam[1]))
			{
				BlendState.y = state_params.at(BlendParam[1]).Value;
			}
			else
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
					float x = motion->BlendState.x;
			
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
					float distance = glm::distance(BlendState, motion->BlendState);
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

		L_CORE_TRACE("BlendTree BlendState: ({}, {})", BlendState.x, BlendState.y);
		for (const auto& motion : ChildNode)
			L_CORE_TRACE("  Motion Blend: ({}, {}), FinalWeight: {}", motion->BlendState.x, motion->BlendState.y, motion->FinalWeight);
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

#pragma endregion

}