#pragma once

// Louron Core Headers
#include "Animation Base Types.h"
#include "Animation State.h"

#include "../Asset/Asset.h"
#include "../Core/Utilities.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace Louron::Animation 
{

    class StateMachine
    {

    public:

        void Evaluate()
        {
            if (m_EntryState) m_EntryState->Evaluate();
            if (m_AnyState) m_EntryState->Evaluate();
            if (m_ExitState) m_EntryState->Evaluate();
        }

        void NewState(const std::string& state_name, StateType state_type);
        void RemoveState(const std::string& state_name);

		void Serialise(YAML::Emitter& out);
		bool Deserialise(const YAML::Node& data);

    private:

        std::unique_ptr<AnimationState> m_EntryState = std::make_unique<AnimationState_Empty>();
        std::unique_ptr<AnimationState> m_AnyState = std::make_unique<AnimationState_Empty>();
        std::unique_ptr<AnimationState> m_ExitState = std::make_unique<AnimationState_Empty>();

        StringHash m_DefaultState = Louron::Utils::fnv1a_hash("DEFAULT_ENTRY_STATE");
        StatesMap m_States = {};
        ParameterMap m_Parameters = {};

    };

}