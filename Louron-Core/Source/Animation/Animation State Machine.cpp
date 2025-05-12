#include "Animation State Machine.h"

#include "../Core/Logging.h"

#include <unordered_set>
#include <yaml-cpp/yaml.h>

namespace Louron::Animation
{

    StateMachine::StateMachine(const StateMachine& other)
    {
        States.clear();
        Transitions.clear();

        for(const auto& [string_hash, state] : other.States)
        {
            if (!state || string_hash == NULL_UUID)
                continue;

            States[string_hash] = state->Clone();
        }

        for(const auto& transition : other.Transitions)
        {
            if (!transition)
                continue;

            Transitions.emplace_back(std::make_unique<AnimationTransition>(*transition.get()));
        }

        DefaultState = other.DefaultState;
        CurrentState = other.CurrentState;
        PreviousState = NULL_UUID;
    }

    StateMachine &StateMachine::operator=(const StateMachine &other)
    {
        if (this == &other)
            return *this;

        States.clear();
        Transitions.clear();

        for(const auto& [string_hash, state] : other.States)
        {
            if (!state || string_hash == NULL_UUID)
                continue;
            
            States[string_hash] = state->Clone();
        }

        for(const auto& transition : other.Transitions)
        {
            if (!transition)
                continue;
            
            Transitions.emplace_back(std::make_unique<AnimationTransition>(*transition.get()));
        }

        DefaultState = other.DefaultState;
        CurrentState = other.CurrentState;
        PreviousState = NULL_UUID;

        return *this;
    }

    void StateMachine::UpdateStates(float ts)
    {
        if (DefaultState == m_DefaultEntryHash)
        {
            L_CORE_WARN("Animation State Machine: No State Linked to Entry!");
            return;
        }

        // First Update - Go From Entry To Default State
        if (CurrentState == m_DefaultEntryHash)
            SetCurrentState(DefaultState);
        
        // This will traverse transitions once we are in a state completely. We
        // will find a state to transition to, or not to transition at all, and
        // if we are we will start a transitioning timer and ensure the state
        // correctly transitions to that state prior to checking again for other
        // transitions from the next node.
        if (TargetState == NULL_UUID)
        {
            for (const auto& transition : Transitions)
            {
                if (!transition)
                    continue;
                
                if (transition->SourceStateHash == CurrentState)
                {
                    if (transition->CheckTransitionConditionsValid(AnimationParameters) && States.contains(transition->DestStateHash))
                    {
                        States[transition->DestStateHash]->CleanState(); // Ensure State is Cleaned Before Transitioning Into It

                        TargetState = transition->DestStateHash;

                        TransitionCompletion = (transition->TransitionDuration > 0.0f) ? 0.0f : 1.0f;

                        break;
                    }
                }
            }
        }
        
        // Check if new target state still invalid after checking for potential transitions, if it isn't we should start transitioning.
        if (TargetState != NULL_UUID)
        {
            auto transition = GetTransition(CurrentState, TargetState);
            if (transition)
            {
                if (transition->TransitionDuration > 0.0f)
                    TransitionCompletion = std::min(1.0f, TransitionCompletion + (ts / transition->TransitionDuration));
                else
                    TransitionCompletion = 1.0f;

                if(TransitionCompletion >= 1.0f)
                {
                    SetCurrentState(TargetState);
                }
            }
            else
            {
                TargetState = NULL_UUID;
                TransitionCompletion = 0.0f;
            }
        }

        if(States.contains(CurrentState) && States[CurrentState])
        {
            States[CurrentState]->Update(ts, AnimationParameters);
        }
        else
        {
            CurrentState = m_DefaultEntryHash;
            PreviousState = NULL_UUID;
            TargetState = NULL_UUID;
            TransitionCompletion = 0.0f;

            L_CORE_ERROR("Animation State Machine: We have been transitioned into a state that does not exist! Resetting State Machine...");
        }
    }

    void StateMachine::EvaluatePose(Louron::AnimationPose &evaluated_pose)
    {
        AnimationPose pose_a, pose_b;

        if(States.contains(CurrentState) && States[CurrentState])
        {
            States[CurrentState]->EvaluatePose(pose_a);
        }
        else
        {
            return;
        }
    
        auto target_state = (TargetState != NULL_UUID) ? States[TargetState].get() : nullptr;
        auto transition = (target_state) ? GetTransition(CurrentState, TargetState) : nullptr;
        if (target_state && transition && transition->TransitionDuration > 0.0f)
        {
            target_state->EvaluatePose(pose_b);
    
            float t = TransitionCompletion / transition->TransitionDuration;
            BlendPoses(pose_a, pose_b, t, evaluated_pose);
        }
        else
        {
            evaluated_pose = std::move(pose_a);
        }
    }

    void StateMachine::BlendPoses(const AnimationPose &a, const AnimationPose &b, float t, AnimationPose &result)
    {
        std::unordered_set<std::string> all_bones;
    
        for (const auto& [bone_name, transform] : a.Pose) all_bones.insert(bone_name);
        for (const auto& [bone_name, transform] : b.Pose) all_bones.insert(bone_name);
    
        for (const auto& bone_name : all_bones)
        {
            const auto& ta = a.Pose.contains(bone_name) ? a.Pose.at(bone_name) : AnimationPose::AnimationTransform{};
            const auto& tb = b.Pose.contains(bone_name) ? b.Pose.at(bone_name) : AnimationPose::AnimationTransform{};
    
            result.Pose[bone_name].Position    = glm::mix(ta.Position, tb.Position, t);
            result.Pose[bone_name].Orientation = glm::slerp(ta.Orientation, tb.Orientation, t);
            result.Pose[bone_name].Scale       = glm::mix(ta.Scale, tb.Scale, t);
        }
    }

    void StateMachine::SetCurrentState(const std::string &state_name) 
    { 
        auto state_hash = Louron::Utils::fnv1a_hash(state_name); 
        if (States.contains(state_hash)) 
        {  
            PreviousState = CurrentState;
            CurrentState = state_hash;
        } 
        else
        {
            L_CORE_WARN("Animation State Machine: State Name Does Not Exist in Animation State Machine: {}", state_name);
        }
    }

    void StateMachine::SetCurrentState(const StringHash &state_hash)
    {
        if (States.contains(state_hash)) 
        {  
            PreviousState = CurrentState;
            CurrentState = state_hash;
        } 
        else
        {
            L_CORE_WARN("Animation State Machine: State Hash Does Not Exist in Animation State Machine: {}", state_hash);
        }
        
        TargetState = NULL_UUID;
        TransitionCompletion = 0.0f;
    }

    StringHash StateMachine::CreateState(const std::string &state_name, StateType state_type)
    {
        StringHash state_hash = Louron::Utils::fnv1a_hash(state_name);
        if (States.size() == 0)
            DefaultState = state_hash;

        switch(state_type)
        {
            case StateType::Clip:
            {
                States[state_hash] = std::make_unique<AnimationState_Clip>();
                break;
            }
            case StateType::BlendTree:
            {
                States[state_hash] = std::make_unique<AnimationState_BlendTree>();
                break;
            }
        }

        if (DefaultState == state_hash)
        {
            CreateTransition(m_DefaultEntryHash, state_hash);
        }

        States[state_hash]->Name = state_name;

        return state_hash;
    }

    void StateMachine::RemoveState(const std::string &state_name)
    {
        RemoveState(Louron::Utils::fnv1a_hash(state_name));
    }

    void StateMachine::RemoveState(const StringHash &state_hash)
    {
        if (States.contains(state_hash))
        {
            RemoveAllTransitionTo(state_hash);
            RemoveAllTransitionFrom(state_hash);

            States.erase(state_hash);
        }
    }

    AnimationState* StateMachine::GetAnimationState(const StringHash &state_hash)
    {
        if (States.contains(state_hash))
            return States[state_hash].get();
        return nullptr;
    }

    AnimationTransition *StateMachine::CreateTransition(const std::string &state_name_from, const std::string &state_name_to)
    {
        return CreateTransition(Louron::Utils::fnv1a_hash(state_name_from), Louron::Utils::fnv1a_hash(state_name_to));
    }

    AnimationTransition* StateMachine::CreateTransition(const StringHash &state_hash_from, const StringHash &state_hash_to)
    {
        if (auto found_transition = GetTransition(state_hash_from, state_hash_to); found_transition)
            return found_transition;

        auto transition = std::make_unique<AnimationTransition>();
        transition->SourceStateHash = state_hash_from;
        transition->DestStateHash = state_hash_to;

        Transitions.emplace_back(std::move(transition));
        
        return Transitions.back().get();
    }

    void StateMachine::RemoveTransition(const std::string &state_name_from, const std::string &state_name_to)
    {
        RemoveTransition(Louron::Utils::fnv1a_hash(state_name_from), Louron::Utils::fnv1a_hash(state_name_to));
    }
    
    void StateMachine::RemoveTransition(const StringHash &state_hash_from, const StringHash &state_hash_to)
    {
        for(auto it = Transitions.begin(); it != Transitions.end(); )
        {
            if (!it->get())
            {
                ++it;
                continue;
            }

            if (it->get()->SourceStateHash == state_hash_from && it->get()->DestStateHash == state_hash_to)
                it = Transitions.erase(it);
            else
                ++it;
        }
    }

    void StateMachine::RemoveAllTransitionFrom(const std::string &state_name)
    {
        RemoveAllTransitionFrom(Louron::Utils::fnv1a_hash(state_name));
    }

    void StateMachine::RemoveAllTransitionFrom(const StringHash &state_hash)
    {
        for (auto it = Transitions.begin(); it != Transitions.end(); )
        {
            if (!it->get())
            {
                ++it;
                continue;
            }

            if (it->get()->SourceStateHash == state_hash)
                it = Transitions.erase(it);
            else
                ++it;
        }
    }

    void StateMachine::RemoveAllTransitionTo(const std::string &state_name)
    {
        RemoveAllTransitionTo(Louron::Utils::fnv1a_hash(state_name));
    }

    void StateMachine::RemoveAllTransitionTo(const StringHash &state_hash)
    {
        for (auto it = Transitions.begin(); it != Transitions.end(); )
        {
            if (!it->get())
            {
                ++it;
                continue;
            }

            if (it->get()->DestStateHash == state_hash)
                it = Transitions.erase(it);
            else
                ++it;
        }
    }

    AnimationTransition* StateMachine::GetTransition(const std::string &state_name_from, const std::string &state_name_to)
    {
        return GetTransition(Louron::Utils::fnv1a_hash(state_name_from), Louron::Utils::fnv1a_hash(state_name_to));
    }

    AnimationTransition* StateMachine::GetTransition(const StringHash &state_hash_from, const StringHash &state_hash_to)
    {
        for(const auto& transition : Transitions)
        {
            if (!transition)
                continue;
            
            if (transition->SourceStateHash == state_hash_from && transition->DestStateHash == state_hash_to)
                return transition.get(); // Found existing transition between two states
        }

        return nullptr;
    }

    StringHash StateMachine::AddParameter(const std::string &param_name, ParameterType param_type)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name);
        AnimationParameters[param_hash] = { param_name, 0.0f, param_type };
        return param_hash;
    }

    void StateMachine::SetBool(const std::string& param_name, bool value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::Bool)
        {
            AnimationParameters[param_hash].Value = ((value) ? 1.0f : 0.0f);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Bool. Not Initialised or Incorrect Type!");
        }
    }

    void StateMachine::SetBool(const StringHash &param_hash, bool value)
    {
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::Bool)
        {
            AnimationParameters[param_hash].Value = ((value) ? 1.0f : 0.0f);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Bool. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetFloat(const std::string& param_name, float value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::Float)
        {
            AnimationParameters[param_hash].Value = value;
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Float. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetFloat(const StringHash &param_hash, float value)
    {
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::Float)
        {
            AnimationParameters[param_hash].Value = value;
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Float. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetInt(const std::string& param_name, int32_t value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::Int)
        {
            AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Int. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetInt(const StringHash &param_hash, int32_t value)
    {
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::Int)
        {
            AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Int. Please use AddParameter prior to setting values.");
        }
    }
    
    void StateMachine::SetUInt(const std::string& param_name, uint32_t value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::UInt)
        {
            AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set UInt. Please use AddParameter prior to setting values.");
        }
    }
    
    void StateMachine::SetUInt(const StringHash &param_hash, uint32_t value)
    {
        if (AnimationParameters.contains(param_hash) && AnimationParameters[param_hash].Type == ParameterType::UInt)
        {
            AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set UInt. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::Serialise(YAML::Emitter &out)
    {
        std::string default_state_name;
        if(DefaultState == m_DefaultEntryHash)
        {
            default_state_name = "DEFAULT_ENTRY";
        }
        else
        {
            default_state_name = (States.contains(DefaultState) && States[DefaultState]) ? States[DefaultState]->Name : "DEFAULT_ENTRY";
        }

		out << YAML::Key << "Default State" << YAML::Value << default_state_name;

        out << YAML::Key << "Animation Parameters" << YAML::Value;
        {
            out << YAML::BeginSeq;

            for (const auto& [hash, param] : AnimationParameters)
            {
                out << YAML::BeginMap;

                out << YAML::Key << "Name" << YAML::Value << param.Name;
                out << YAML::Key << "Hash" << YAML::Value << hash;
                out << YAML::Key << "Type" << YAML::Value << Utils::ParamTypeToString(param.Type);
                out << YAML::Key << "Value" << YAML::Value << param.Value;

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
        }
        
        out << YAML::Key << "Animation States" << YAML::Value;
        {
            out << YAML::BeginSeq;

            for (const auto& [hash, state] : States)
            {
                if(!state) continue;

                out << YAML::BeginMap;

                out << YAML::Key << "Name" << YAML::Value << state->Name;
                out << YAML::Key << "Hash" << YAML::Value << hash;
                out << YAML::Key << "Type" << YAML::Value << Utils::StateTypeToString(state->GetType());

                state->Serialise(out);

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
        }
        
        out << YAML::Key << "Animation Transitions" << YAML::Value;
        {
            out << YAML::BeginSeq;

            for (const auto& transition : Transitions)
            {
                if (!transition || (!States.contains(transition->SourceStateHash) || !States.contains(transition->DestStateHash)) || (!States[transition->SourceStateHash] || !States[transition->DestStateHash]))
                    continue;

                out << YAML::BeginMap;

                out << YAML::Key << "Source Name" << YAML::Value << States[transition->SourceStateHash]->Name;
                out << YAML::Key << "Source Hash" << YAML::Value << transition->SourceStateHash;

                out << YAML::Key << "Destination Name" << YAML::Value << States[transition->DestStateHash]->Name;
                out << YAML::Key << "Destination Hash" << YAML::Value << transition->DestStateHash;

                out << YAML::Key << "Transition Duration"   << YAML::Value << transition->TransitionDuration;
                out << YAML::Key << "Has Exit Time"         << YAML::Value << transition->HasExitTime;
                out << YAML::Key << "Exit Time"             << YAML::Value << transition->ExitTime;

                out << YAML::Key << "Conditions" << YAML::Value;
                {
                    out << YAML::BeginSeq;

                    for (const auto& condition : transition->Conditions)
                    {
                        out << YAML::BeginMap;
    
                        out << YAML::Key << "Parameter Name"        << YAML::Value << condition.ParameterName;
                        out << YAML::Key << "Parameter Hash"        << YAML::Value << condition.ParameterHash;
                        out << YAML::Key << "Comparison Operation"  << YAML::Value << Utils::ComparisonTypeToString(condition.Operation);
                        out << YAML::Key << "Reference Threshold"   << YAML::Value << condition.ReferenceThreshold;
                        
                        out << YAML::EndMap;
                    }
                    
                    out << YAML::EndSeq;
                }

                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
        }
    }

    void StateMachine::Deserialise(const YAML::Node &data)
    {
        if (data["Default State"])
        {
            DefaultState = Louron::Utils::fnv1a_hash(data["Default State"].as<std::string>());
        }

        if (data["Animation Parameters"])
        {
            for(const auto& parameter : data["Animation Parameters"])
            {
                StringHash param_hash = NULL_UUID;
                AnimationParameter param = {};
                if(parameter["Name"])
                {
                    param.Name = parameter["Name"].as<std::string>();
                }
                if(parameter["Hash"])
                {
                    param_hash = parameter["Hash"].as<StringHash>();
                }
                if(parameter["Type"])
                {
                    param.Type = Utils::ParamTypeFromString(parameter["Type"].as<std::string>());
                }
                if(parameter["Value"])
                {
                    param.Value = parameter["Value"].as<float>();
                }
                AnimationParameters[param_hash] = std::move(param);
            }
        }

        if (data["Animation States"])
        {
            for(const auto& state : data["Animation States"])
            {
                StringHash state_hash = NULL_UUID;
                StateType type = StateType::Unknown;

                if(state["Type"])
                {
                    type = Utils::StateTypeFromString(state["Type"].as<std::string>());
                } else continue;

                if (state["Hash"])
                {
                    state_hash = state["Hash"].as<StringHash>();
                } else continue;

                switch (type)
                {
                    case StateType::Clip: States[state_hash] = std::make_unique<AnimationState_Clip>(); break; 
                    case StateType::BlendTree: States[state_hash] = std::make_unique<AnimationState_BlendTree>(); break; 
                    case StateType::Unknown: continue; 
                }
                
                States[state_hash]->Name = state["Name"].as<std::string>();
                States[state_hash]->Deserialise(state);
            }            
        }

        if (data["Animation Transitions"])
        {
            for(const auto& transition : data["Animation Transitions"])
            {
                StringHash source_hash = NULL_UUID;
                StringHash dest_hash = NULL_UUID;
                
                if(transition["Source Hash"])
                    source_hash = transition["Source Hash"].as<StringHash>();
                
                if(transition["Destination Hash"])
                    dest_hash = transition["Destination Hash"].as<StringHash>();

                if (source_hash == NULL_UUID || dest_hash == NULL_UUID)
                    continue;
                
                auto transition_ptr = CreateTransition(source_hash, dest_hash);
                if(!transition_ptr)
                    continue;

                if(transition["Transition Duration"])
                    transition_ptr->TransitionDuration = transition["Transition Duration"].as<float>();

                if(transition["Has Exit Time"])
                    transition_ptr->HasExitTime = transition["Has Exit Time"].as<bool>();

                if(transition["Exit Time"])
                    transition_ptr->ExitTime = transition["Exit Time"].as<float>();

                for(const auto& condition : transition["Conditions"])
                {
                    TransitionCondition cond{};

                    if(condition["Parameter Name"])
                        cond.ParameterName = condition["Parameter Name"].as<std::string>();

                    if(condition["Parameter Hash"])
                        cond.ParameterHash = condition["Parameter Hash"].as<StringHash>();

                    if(condition["Comparison Operation"])
                        cond.Operation = Utils::ComparisonTypeFromString(condition["Comparison Operation"].as<std::string>());

                    if(condition["Reference Threshold"])
                        cond.ReferenceThreshold = condition["Reference Threshold"].as<float>();

                    transition_ptr->Conditions.emplace_back(std::move(cond));
                }
            }
        }
    }
}