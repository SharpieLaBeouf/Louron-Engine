#include "Animation Blend Tree.h"

namespace Louron::Animation
{

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

    void BlendTree::Update(float ts, const std::unordered_map<StringHash, AnimationParameter> state_params)
    {

    }

    void BlendTree::CleanBlendTree()
    {

    }
}