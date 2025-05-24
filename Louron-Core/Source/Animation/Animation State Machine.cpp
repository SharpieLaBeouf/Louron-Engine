#include "Animation State Machine.h"

#include "../Core/Logging.h"

#include <unordered_set>
#include <yaml-cpp/yaml.h>

namespace Louron::Animation
{

    StringHash StateMachine::DefaultEntryHash = Louron::Utils::fnv1a_hash("DEFAULT_ENTRY");
    StringHash StateMachine::DefaultAnyHash   = Louron::Utils::fnv1a_hash("DEFAULT_ANY");
    StringHash StateMachine::DefaultExitHash  = Louron::Utils::fnv1a_hash("DEFAULT_EXIT");

    StateMachine::Layer::Layer(const Layer &other)
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

        TargetState = other.TargetState;
        TransitionCompletion = other.TransitionCompletion;
        ExitTimeCompletion = other.ExitTimeCompletion;
        PreviousNormalisedTime = other.PreviousNormalisedTime;

        LayerName = other.LayerName;
        LayerWeight = other.LayerWeight;
        BlendType = other.BlendType;
        SyncLayer = other.SyncLayer;
        LayerIndex = other.LayerIndex;
        UseOwnLayerTiming = other.UseOwnLayerTiming;
        UsingIK = other.UsingIK;
    }

    StateMachine::Layer& StateMachine::Layer::operator=(const Layer &other)
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

        TargetState = other.TargetState;
        TransitionCompletion = other.TransitionCompletion;
        ExitTimeCompletion = other.ExitTimeCompletion;
        PreviousNormalisedTime = other.PreviousNormalisedTime;

        LayerName = other.LayerName;
        LayerWeight = other.LayerWeight;
        BlendType = other.BlendType;
        SyncLayer = other.SyncLayer;
        LayerIndex = other.LayerIndex;
        UseOwnLayerTiming = other.UseOwnLayerTiming;
        UsingIK = other.UsingIK;

        return *this;
    }

#pragma region Constructors and Operators

    StateMachine::StateMachine(const StateMachine &other)
    {
        m_Layers.clear();
        m_Layers.reserve(other.m_Layers.size());

        m_Layers = other.m_Layers;

        m_AnimationParameters.clear();
        m_AnimationParameters = other.m_AnimationParameters;
    }

    StateMachine& StateMachine::operator=(const StateMachine &other)
    {
        if (this == &other)
            return *this;
       
            m_Layers.clear();
        m_Layers.reserve(other.m_Layers.size());

        m_Layers = other.m_Layers;

        m_AnimationParameters.clear();
        m_AnimationParameters = other.m_AnimationParameters;

        return *this;
    }

#pragma endregion

#pragma region General Functionality

    void StateMachine::UpdateStates(float ts)
    {
        if (m_Layers.empty())
            return;
            
        for (int layer_index = 0; layer_index < m_Layers.size(); ++layer_index)
        {
            auto& layer = m_Layers[layer_index];

            if (layer.CurrentState == StateMachine::DefaultExitHash)
                return; // No Updates - StateMachine Exited! To restart, use StateMachine::SetCurrentState(StateMachine::GetEntryHash());

            if (layer.DefaultState == StateMachine::DefaultEntryHash)
            {
                L_CORE_WARN("Animation State Machine: No State Linked to Entry!");
                return;
            }

            if (layer.CurrentState == StateMachine::DefaultEntryHash)
                SetCurrentState(layer_index, layer.DefaultState);

            bool is_transition_blending = false;

            const float curr_norm = layer.States[layer.CurrentState]->NormalisedStateTime;
            float delta_norm = curr_norm - layer.PreviousNormalisedTime;
            if (delta_norm < 0.0f)
                delta_norm += 1.0f;

            layer.PreviousNormalisedTime = curr_norm;

            // Check for transition
            if (layer.TargetState == NULL_UUID)
            {
                for (const auto& transition : layer.Transitions)
                {
                    if (!transition || transition->SourceStateHash != layer.CurrentState)
                        continue;

                    if (!transition->CheckTransitionConditionsValid(m_AnimationParameters))
                        continue;

                    layer.TargetState = transition->DestStateHash;
                    layer.TransitionCompletion = 0.0f;

                    if (transition->HasExitTime)
                    {
                        // Compute effective exitTime for looping vs non-looping
                        float exit_time_value = transition->ExitTime;
                        auto state_type = layer.States[layer.CurrentState]->GetType();
                        bool looping_state = (state_type == StateType::Clip && reinterpret_cast<AnimationState_Clip*>(layer.States[layer.CurrentState].get())->IsLooping) || (state_type == StateType::BlendTree);

                        if (!looping_state && exit_time_value > 1.0f)
                            exit_time_value = fmod(exit_time_value, 1.0f);

                        // Initialize the counter to align with current playhead
                        if (curr_norm < exit_time_value)
                            layer.ExitTimeCompletion = curr_norm;
                        else if (exit_time_value == curr_norm)
                            layer.ExitTimeCompletion = exit_time_value;
                        else
                            layer.ExitTimeCompletion = curr_norm - 1.0f;
                    }
                    else
                    {
                        layer.ExitTimeCompletion = 0.0f;
                    }

                    break; // only arm one transition
                }
            }

            // If a transition is armed, see if we can start blending
            if (layer.TargetState != NULL_UUID)
            {
                auto transition = GetTransition(layer_index, layer.CurrentState, layer.TargetState);
                if (transition)
                {
                    bool can_start_blend = false;

                    if (!transition->HasExitTime)
                    {
                        can_start_blend = true;
                    }
                    else
                    {
                        // Recompute effective exitTimeValue
                        float exit_time_value = transition->ExitTime;
                        auto state_type = layer.States[layer.CurrentState]->GetType();
                        bool looping_state = (state_type == StateType::Clip && reinterpret_cast<AnimationState_Clip*>(layer.States[layer.CurrentState].get())->IsLooping) || (state_type == StateType::BlendTree);

                        if (!looping_state && exit_time_value > 1.0f)
                            exit_time_value = fmod(exit_time_value, 1.0f);

                        // Accumulate wrapped delta
                        layer.ExitTimeCompletion += delta_norm;
                        if (layer.ExitTimeCompletion >= exit_time_value)
                            can_start_blend = true;
                    }

                    if (can_start_blend)
                    {
                        // Clean target state once at start of blend
                        if (layer.TransitionCompletion == 0.0f && layer.States[layer.TargetState])
                            layer.States[layer.TargetState]->CleanState();

                        // Advance blend
                        if (transition->TransitionDuration > 0.0f)
                        {
                            layer.TransitionCompletion = std::min(1.0f, layer.TransitionCompletion + (ts / transition->TransitionDuration));
                            is_transition_blending = true;
                        }
                        else
                        {
                            layer.TransitionCompletion = 1.0f;
                        }

                        // Complete or continue blend
                        if (layer.TransitionCompletion >= 1.0f)
                        {
                            SetCurrentState(layer_index, layer.TargetState);
                        }
                        else if (layer.States[layer.TargetState])
                        {
                            layer.States[layer.TargetState]->Update(ts, m_AnimationParameters);
                        }
                    }
                }
                else
                {
                    // Invalid transition, reset
                    layer.TargetState = NULL_UUID;
                    layer.TransitionCompletion = 0.0f;
                    layer.ExitTimeCompletion = 0.0f;
                }
            }

            // Always update current state
            if (layer.States.contains(layer.CurrentState) && layer.States[layer.CurrentState])
            {
                layer.States[layer.CurrentState]->Update(ts, m_AnimationParameters);
            }
            else if (layer.CurrentState != StateMachine::DefaultExitHash)
            {
                // Fallback for invalid current state
                ResetMachine();
                L_CORE_ERROR("Animation State Machine: Transitioned into a state that does not exist! Resetting State Machine...");
            }

        }
    }

    void StateMachine::EvaluatePose(Louron::AnimationPose &evaluated_pose)
    {
        if (m_Layers.empty())
            return;
        
        using Contribution = std::tuple<Layer*, float>;
        std::vector<Contribution> layer_contributions;

        // -------------------------------
        // STEP 1: Determine contributions
        // -------------------------------
        float remaining_budget = 1.0f;

        for (int i = static_cast<int>(m_Layers.size()) - 1; i >= 0; --i)
        {
            Layer& layer = m_Layers[i];
            
            if (i == 0) 
                layer.LayerWeight = 1.0f; // Always enforce full strength on base layer.

            if (layer.BlendType == Layer::LayerBlendType::Override)
            {
                float contribution = remaining_budget * layer.LayerWeight;

                if (contribution > 0.0f)
                    layer_contributions.emplace_back(&layer, contribution);

                // If layer wants more than what's left, give it the rest and stop
                if (contribution >= remaining_budget)
                {
                    remaining_budget = 0.0f;
                    break;
                }

                // Otherwise, continue
                remaining_budget -= contribution;
            }
            else if (layer.LayerWeight > 0.0f)
            {
                // TODO: layer_contributions.emplace_back(&layer, layer.LayerWeight);
            }
        }

        // -------------------------------
        // STEP 2: Blend poses front-to-back
        // -------------------------------
        Louron::AnimationPose blended_pose;
        float blend_total_weight = 0.0f;

        for (int i = static_cast<int>(layer_contributions.size()) - 1; i >= 0; --i)
        {
            Layer* layer = std::get<0>(layer_contributions[i]);
            float weight = std::get<1>(layer_contributions[i]);

            Louron::AnimationPose layer_pose;
            layer->EvaluatePose(layer_pose);

            if (layer->BlendType == Layer::LayerBlendType::Override)
            {
                float new_total_weight = blend_total_weight + weight;

                if (blend_total_weight == 0.0f)
                {
                    // First pose sets the base pose to full strength
                    blended_pose = layer_pose;
                }
                else
                {
                    float t = weight / new_total_weight; // Proportion of new pose in the updated blend
                    AnimationPose out_pose;
                    StateMachine::BlendPoses(blended_pose, layer_pose, t, out_pose);
                    blended_pose = std::move(out_pose);
                }

                blend_total_weight = new_total_weight;
            }
            else
            {
                // TODO: Additive Blending, Need to Add Additive Reference Poses
            }
        }

        evaluated_pose = blended_pose;
    }

    void StateMachine::Layer::EvaluatePose(AnimationPose& evaluated_pose)
    {
        AnimationPose pose_a, pose_b;

        if(States.contains(CurrentState) && States[CurrentState])
        {
            States[CurrentState]->EvaluatePose(pose_a);
        }
        else
        {
            if (CurrentState == StateMachine::DefaultExitHash && States.contains(PreviousState) && States[PreviousState])
                States[PreviousState]->EvaluatePose(pose_a);

            return;
        }
    
        auto target_state = (TargetState != NULL_UUID && States.contains(TargetState)) ? States[TargetState].get() : nullptr;
        AnimationTransition* transition = nullptr;

        for(const auto& t : Transitions)
        {
            if (!t)
                continue;
            
            if (t->SourceStateHash == CurrentState && t->DestStateHash == TargetState)
                transition =  t.get(); // Found existing transition between two states
        }

        if (target_state && transition && transition->TransitionDuration > 0.0f)
        {
            target_state->EvaluatePose(pose_b);
    
            float t = TransitionCompletion / transition->TransitionDuration;
            StateMachine::BlendPoses(pose_a, pose_b, t, evaluated_pose);
        }
        else
        {
            evaluated_pose = std::move(pose_a);
        }
    }

    void StateMachine::ResetMachine()
    {
        for (auto& layer : m_Layers)
        {
            layer.CurrentState              = StateMachine::DefaultEntryHash;
            layer.PreviousState             = NULL_UUID;
            layer.TargetState               = NULL_UUID;
            
            layer.TransitionCompletion      = 0.0f;
            layer.ExitTimeCompletion        = 0.0f;
            layer.PreviousNormalisedTime    = 0.0f;

            for (auto& [hash, state] : layer.States)
            {
                if (state)
                {
                    state->CleanState();
                }
            }
        }
    }
    
    bool StateMachine::ValidLayer(size_t layer_index) const 
    {
        bool result = (layer_index >= 0 && layer_index < m_Layers.size());
        if(!result)
            L_CORE_WARN("Animation State Machine: Invalid Layer Index. Index Passed: {} - Total Layer Stack Size: {}", std::to_string(layer_index), std::to_string(m_Layers.size()));
        return result;
    }

    void StateMachine::BlendPoses(const AnimationPose& a, const AnimationPose& b, float t, AnimationPose& result)
    {
        result.Pose.clear();

        std::unordered_set<std::string> all_bones;

        for (const auto& [bone_name, _] : a.Pose) all_bones.insert(bone_name);
        for (const auto& [bone_name, _] : b.Pose) all_bones.insert(bone_name);

        for (const auto& bone_name : all_bones)
        {
            const bool has_a = a.Pose.contains(bone_name);
            const bool has_b = b.Pose.contains(bone_name);

            AnimationPose::AnimationTransform out;

            if (has_a && has_b)
            {
                const auto& ta = a.Pose.at(bone_name);
                const auto& tb = b.Pose.at(bone_name);

                out.Position    = glm::mix(ta.Position, tb.Position, t);
                out.Orientation = glm::slerp(ta.Orientation, tb.Orientation, t);
                out.Scale       = glm::mix(ta.Scale, tb.Scale, t);
            }
            else if (has_a)
            {
                out = a.Pose.at(bone_name);
            }
            else if (has_b)
            {
                out = b.Pose.at(bone_name);
            }

            result.Pose[bone_name] = out;
        }
    }
    
#pragma endregion

#pragma region --State-- Getter and Setters

    void StateMachine::SetCurrentState(size_t layer_index, const std::string& state_name) 
    {
        if (!ValidLayer(layer_index))
            return;

        auto state_hash = Louron::Utils::fnv1a_hash(state_name); 
        if (m_Layers[layer_index].States.contains(state_hash)) 
        {  
            m_Layers[layer_index].PreviousState = m_Layers[layer_index].CurrentState;
            m_Layers[layer_index].CurrentState = state_hash;
        } 
        else
        {
            L_CORE_WARN("Animation State Machine: State Name Does Not Exist in Animation State Machine: {}", state_name);
        }
    }

    void StateMachine::SetCurrentState(size_t layer_index, const StringHash& state_hash)
    {
        if (!ValidLayer(layer_index))
            return;
            
        auto& layer = m_Layers[layer_index];

        if (layer.States.contains(state_hash) || state_hash == StateMachine::DefaultExitHash) 
        {  
            layer.PreviousState = layer.CurrentState;
            layer.CurrentState = state_hash;
        } 
        else
        {
            L_CORE_WARN("Animation State Machine: State Hash Does Not Exist in Animation State Machine: {}", state_hash);
        }
        
        layer.TargetState = NULL_UUID;
        layer.TransitionCompletion = 0.0f;
    }

    StringHash StateMachine::CreateState(size_t layer_index, const std::string& state_name, StateType state_type)
    {
        if (!ValidLayer(layer_index))
            return NULL_UUID;
            
        auto& layer = m_Layers[layer_index];

        StringHash state_hash = Louron::Utils::fnv1a_hash(state_name);
        if (layer.States.size() == 0)
            layer.DefaultState = state_hash;

        switch(state_type)
        {
            case StateType::Clip:
            {
                layer.States[state_hash] = std::make_unique<AnimationState_Clip>();
                break;
            }
            case StateType::BlendTree:
            {
                layer.States[state_hash] = std::make_unique<AnimationState_BlendTree>();
                break;
            }
        }

        if (layer.DefaultState == state_hash)
        {
            CreateTransition(layer_index, StateMachine::DefaultEntryHash, state_hash);
        }

        layer.States[state_hash]->Name = state_name;

        return state_hash;
    }

    void StateMachine::RenameState(size_t layer_index, const StringHash& state_hash, const std::string& state_new_name)
    {
        if (!ValidLayer(layer_index))
            return;

        auto& layer = m_Layers[layer_index];

        if (!layer.States.contains(state_hash)) // Current State Does Not Exist
            return;

        StringHash state_new_hash = Louron::Utils::fnv1a_hash(state_new_name);
        if (layer.States.contains(state_new_hash)) // Already Exists!
            return;

        layer.States[state_new_hash] = std::move(layer.States[state_hash]);
        layer.States[state_new_hash]->Name = state_new_name;
        layer.States.erase(state_hash);

        if (layer.DefaultState == state_hash)
            layer.DefaultState = state_new_hash;

        if (layer.PreviousState == state_hash)
            layer.PreviousState = state_new_hash;

        if (layer.TargetState == state_hash)
            layer.TargetState = state_new_hash;

        for (auto& transition : layer.Transitions)
        {
            if  (!transition)
                continue;
            
            if(transition->SourceStateHash == state_hash)
                transition->SourceStateHash = state_new_hash;
                
            if(transition->DestStateHash == state_hash)
                transition->DestStateHash = state_new_hash;
        }
    }

    void StateMachine::RemoveState(size_t layer_index, const std::string& state_name)
    {
        RemoveState(layer_index, Louron::Utils::fnv1a_hash(state_name));
    }

    void StateMachine::RemoveState(size_t layer_index, const StringHash& state_hash)
    {
        if (!ValidLayer(layer_index))
            return;

        auto& layer = m_Layers[layer_index];

        if (layer.States.contains(state_hash))
        {
            RemoveAllTransitionTo(layer_index, state_hash);
            RemoveAllTransitionFrom(layer_index, state_hash);

            layer.States.erase(state_hash);
        }
    }

    StringHash StateMachine::GetDefaultStateHash(size_t layer_index) const
    {
        return ValidLayer(layer_index) ? m_Layers[layer_index].DefaultState : NULL_UUID;
    }

    void StateMachine::SetDefaultStateHash(size_t layer_index, const StringHash &state_hash)
    {
        if (!ValidLayer(layer_index))
            return;

        auto& layer = m_Layers[layer_index];

        if(!layer.States.contains(state_hash))
            return;
        
        layer.DefaultState = state_hash;
    }

    const StateMachine::StateMap& StateMachine::GetAllStates(size_t layer_index) const
    {
        if (!ValidLayer(layer_index))
        {
            static StateMap s_NullStateMap = {};
            return s_NullStateMap;
        }

        return m_Layers[layer_index].States;
    }

    AnimationState* StateMachine::GetAnimationState(size_t layer_index, const std::string &state_name)
    { 
        return GetAnimationState(layer_index, Louron::Utils::fnv1a_hash(state_name)); 
    }

    AnimationState* StateMachine::GetAnimationState(size_t layer_index, const StringHash &state_hash)
    {
        if (!ValidLayer(layer_index))
            return nullptr;

        auto& layer = m_Layers[layer_index];

        if (!layer.States.contains(state_hash) || state_hash == NULL_UUID)
            return nullptr;

        return layer.States[state_hash].get();
    }

    AnimationState* StateMachine::GetCurrentAnimationState(size_t layer_index)
    {
        if (!ValidLayer(layer_index))
            return nullptr;

        auto& layer = m_Layers[layer_index];

        if (!layer.States.contains(layer.CurrentState))
            return nullptr;

        return layer.States[layer.CurrentState].get();
    }

    AnimationState* StateMachine::GetPreviousAnimationState(size_t layer_index)
    { 
        if (!ValidLayer(layer_index))
            return nullptr;

        auto& layer = m_Layers[layer_index];

        if (!layer.States.contains(layer.PreviousState))
            return nullptr;

        return layer.States[layer.PreviousState].get();
    }

    AnimationState* StateMachine::GetTargetAnimationState(size_t layer_index)
    {
        if (!ValidLayer(layer_index))
            return nullptr;

        auto& layer = m_Layers[layer_index];

        if (!layer.States.contains(layer.TargetState))
            return nullptr;

        return layer.States[layer.TargetState].get();
    }

#pragma endregion

#pragma region --Transition-- Getter and Setters

    AnimationTransition* StateMachine::CreateTransition(size_t layer_index, const std::string& state_name_from, const std::string& state_name_to)
    {
        return CreateTransition(layer_index, Louron::Utils::fnv1a_hash(state_name_from), Louron::Utils::fnv1a_hash(state_name_to));
    }

    AnimationTransition* StateMachine::CreateTransition(size_t layer_index, const StringHash& state_hash_from, const StringHash& state_hash_to)
    {
        if (!ValidLayer(layer_index))
            return nullptr;

        auto& layer = m_Layers[layer_index];

        if (auto found_transition = GetTransition(layer_index, state_hash_from, state_hash_to); found_transition)
            return found_transition;

        auto transition = std::make_unique<AnimationTransition>();
        transition->SourceStateHash = state_hash_from;
        transition->DestStateHash = state_hash_to;

        layer.Transitions.emplace_back(std::move(transition));
        
        return layer.Transitions.back().get();
    }

    void StateMachine::RemoveTransition(size_t layer_index, const std::string& state_name_from, const std::string& state_name_to)
    {
        RemoveTransition(layer_index, Louron::Utils::fnv1a_hash(state_name_from), Louron::Utils::fnv1a_hash(state_name_to));
    }
    
    void StateMachine::RemoveTransition(size_t layer_index, const StringHash& state_hash_from, const StringHash& state_hash_to)
    {
        if (!ValidLayer(layer_index))
            return;

        auto& layer = m_Layers[layer_index];

        for(auto it = layer.Transitions.begin(); it != layer.Transitions.end(); )
        {
            if (!it->get())
            {
                ++it;
                continue;
            }

            if (it->get()->SourceStateHash == state_hash_from && it->get()->DestStateHash == state_hash_to)
                it = layer.Transitions.erase(it);
            else
                ++it;
        }
    }

    void StateMachine::RemoveAllTransitionFrom(size_t layer_index, const std::string& state_name)
    {
        RemoveAllTransitionFrom(layer_index, Louron::Utils::fnv1a_hash(state_name));
    }

    void StateMachine::RemoveAllTransitionFrom(size_t layer_index, const StringHash& state_hash)
    {
        if (!ValidLayer(layer_index))
            return;

        auto& layer = m_Layers[layer_index];

        for (auto it = layer.Transitions.begin(); it != layer.Transitions.end(); )
        {
            if (!it->get())
            {
                ++it;
                continue;
            }

            if (it->get()->SourceStateHash == state_hash)
                it = layer.Transitions.erase(it);
            else
                ++it;
        }
    }

    void StateMachine::RemoveAllTransitionTo(size_t layer_index, const std::string& state_name)
    {
        RemoveAllTransitionTo(layer_index, Louron::Utils::fnv1a_hash(state_name));
    }

    void StateMachine::RemoveAllTransitionTo(size_t layer_index, const StringHash& state_hash)
    {
        if (!ValidLayer(layer_index))
            return;

        auto& layer = m_Layers[layer_index];

        for (auto it = layer.Transitions.begin(); it != layer.Transitions.end(); )
        {
            if (!it->get())
            {
                ++it;
                continue;
            }

            if (it->get()->DestStateHash == state_hash)
                it = layer.Transitions.erase(it);
            else
                ++it;
        }
    }

    AnimationTransition* StateMachine::GetTransition(size_t layer_index, const std::string& state_name_from, const std::string& state_name_to)
    {
        return GetTransition(layer_index, Louron::Utils::fnv1a_hash(state_name_from), Louron::Utils::fnv1a_hash(state_name_to));
    }

    AnimationTransition* StateMachine::GetTransition(size_t layer_index, const StringHash& state_hash_from, const StringHash& state_hash_to)
    {
        if (!ValidLayer(layer_index))
            return nullptr;

        auto& layer = m_Layers[layer_index];

        for(const auto& transition : layer.Transitions)
        {
            if (!transition)
                continue;
            
            if (transition->SourceStateHash == state_hash_from && transition->DestStateHash == state_hash_to)
                return transition.get(); // Found existing transition between two states
        }

        return nullptr;
    }

    std::vector<std::unique_ptr<AnimationTransition>>* StateMachine::GetAllTransitions(size_t layer_index)
    {
        if (!ValidLayer(layer_index))
            return nullptr;

        return &m_Layers[layer_index].Transitions;
    }

#pragma endregion

#pragma region --Transition-- Getter and Setters

    StringHash StateMachine::AddParameter(const std::string &param_name, ParameterType param_type)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name);

        if (m_AnimationParameters.contains(param_hash)) 
            return param_hash;

        m_AnimationParameters[param_hash] = { param_name, 0.0f, param_type };
        return param_hash;
    }

    bool StateMachine::HasParameterNamed(const std::string &param_name)
    {
        return m_AnimationParameters.contains(Louron::Utils::fnv1a_hash(param_name));
    }

    void StateMachine::RenameParameter(const StringHash &param_hash, const std::string &param_new_name)
    {
        if (!m_AnimationParameters.contains(param_hash)) // Current Param Does Not Exist
            return;

        StringHash param_new_hash = Louron::Utils::fnv1a_hash(param_new_name);
        if (m_AnimationParameters.contains(param_new_hash)) // Already Exists!
            return;

        m_AnimationParameters[param_new_hash] = m_AnimationParameters[param_hash];
        m_AnimationParameters[param_new_hash].Name = param_new_name;
        m_AnimationParameters.erase(param_hash);

        for (auto& layer : m_Layers)
        {
            // Traverse the State Machine and any recursive blend trees to update the param references!
            for (auto& [hash, state] : layer.States)
            {
                if (!state)
                    continue;

                switch (state->GetType())
                {
                    case StateType::Clip:
                    {
                        // Do nothing
                        break;
                    }
                    case StateType::BlendTree:
                    {
                        std::function<void(Louron::Animation::BlendNode&)> rename_params_recursive = [&](Louron::Animation::BlendNode& blend_node) -> void
                        {
                            if (blend_node.BlendParam[0] == param_hash)
                            {
                                blend_node.BlendParam[0] = param_new_hash;
                            }

                            if (blend_node.BlendParam[1] == param_hash)
                            {
                                blend_node.BlendParam[1] = param_new_hash;
                            }

                            for(auto& motion : blend_node.ChildNode)
                            {
                                if (!motion)
                                    continue;
                                
                                switch (motion->GetType())
                                {
                                    case MotionType::Clip:
                                    {
                                        // Do nothing
                                        break;
                                    }
                                    case MotionType::BlendTree:
                                    {
                                        auto motion_blend_tree = reinterpret_cast<MotionBlendTree*>(motion.get());
                                        rename_params_recursive(motion_blend_tree->RootNode);
                                        break;
                                    }
                                }
                            }
                        };
                        
                        auto state_blend_tree = reinterpret_cast<AnimationState_BlendTree*>(state.get());
                        rename_params_recursive(state_blend_tree->AnimBlendTree);
                        break;
                    }
                }
            }

            // Update Transitions & Conditions
            for (auto& transition : layer.Transitions)
            {
                if (!transition)
                    continue;

                for (auto& condition : transition->Conditions)
                {
                    if (condition.ParameterHash == param_hash)
                    {
                        condition.ParameterHash = param_new_hash;
                        condition.ParameterName = param_new_name;
                    }
                }
            }
        }
    }

    void StateMachine::RemoveParameter(const std::string &param_name)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name);
        RemoveParameter(param_hash);
    }

    void StateMachine::RemoveParameter(const StringHash &param_hash)
    {
        if (m_AnimationParameters.contains(param_hash))
        {
            m_AnimationParameters.erase(param_hash);
        }
    }

    AnimationParameter *StateMachine::GetParameter(const StringHash &param_hash)
    {
        if (m_AnimationParameters.contains(param_hash)) 
        { 
            return &m_AnimationParameters[param_hash]; 
        } 
        return nullptr;
    }

    const std::unordered_map<StringHash, AnimationParameter> &StateMachine::GetParameters() const
    {
        return m_AnimationParameters;
    }

    void StateMachine::SetBool(const std::string& param_name, bool value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::Bool)
        {
            m_AnimationParameters[param_hash].Value = ((value) ? 1.0f : 0.0f);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Bool. Not Initialised or Incorrect Type!");
        }
    }

    void StateMachine::SetBool(const StringHash &param_hash, bool value)
    {
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::Bool)
        {
            m_AnimationParameters[param_hash].Value = ((value) ? 1.0f : 0.0f);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Bool. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetFloat(const std::string& param_name, float value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::Float)
        {
            m_AnimationParameters[param_hash].Value = value;
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Float. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetFloat(const StringHash &param_hash, float value)
    {
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::Float)
        {
            m_AnimationParameters[param_hash].Value = value;
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Float. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetInt(const std::string& param_name, int32_t value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::Int)
        {
            m_AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Int. Please use AddParameter prior to setting values.");
        }
    }

    void StateMachine::SetInt(const StringHash &param_hash, int32_t value)
    {
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::Int)
        {
            m_AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set Int. Please use AddParameter prior to setting values.");
        }
    }
    
    void StateMachine::SetUInt(const std::string& param_name, uint32_t value)
    {
        StringHash param_hash = Louron::Utils::fnv1a_hash(param_name); 
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::UInt)
        {
            m_AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set UInt. Please use AddParameter prior to setting values.");
        }
    }
    
    void StateMachine::SetUInt(const StringHash &param_hash, uint32_t value)
    {
        if (m_AnimationParameters.contains(param_hash) && m_AnimationParameters[param_hash].Type == ParameterType::UInt)
        {
            m_AnimationParameters[param_hash].Value = static_cast<float>(value);
        }
        else
        {
            L_CORE_WARN("Animation State Machine: Could Not Set UInt. Please use AddParameter prior to setting values.");
        }
    }

#pragma endregion

    void StateMachine::Serialise(YAML::Emitter &out)
    {
        out << YAML::Key << "Animation Parameters" << YAML::Value;
        {
            out << YAML::BeginSeq;

            for (const auto& [hash, param] : m_AnimationParameters)
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

        out << YAML::Key << "State Machine Layers" << YAML::Value;
        out << YAML::BeginSeq;

        for(size_t i = 0; i < m_Layers.size(); ++i)
        {
            Layer& layer = m_Layers[i];

            out << YAML::BeginMap;

            out << YAML::Key << "Layer Name" << YAML::Value << layer.LayerName; 
            out << YAML::Key << "Layer Weight" << YAML::Value << layer.LayerWeight;
            out << YAML::Key << "Layer Blend Type" << YAML::Value << (layer.BlendType == Layer::LayerBlendType::Override ? "Override" : "Additive");
            
            out << YAML::Key << "Layer Sync Layer" << YAML::Value << layer.SyncLayer;
            out << YAML::Key << "Layer Sync Index" << YAML::Value << layer.LayerIndex;
            out << YAML::Key << "Layer Use Own Timing" << YAML::Value << layer.UseOwnLayerTiming;
            out << YAML::Key << "Layer Using IK" << YAML::Value << layer.UsingIK;

            {
                std::string default_state_name;
                if(layer.DefaultState == StateMachine::DefaultEntryHash)
                {
                    default_state_name = "DEFAULT_ENTRY";
                }
                else
                {
                    default_state_name = (layer.States.contains(layer.DefaultState) && layer.States[layer.DefaultState]) ? layer.States[layer.DefaultState]->Name : "DEFAULT_ENTRY";
                }

                out << YAML::Key << "Layer Default State" << YAML::Value << default_state_name;
                
                out << YAML::Key << "Layer Animation States" << YAML::Value;
                {
                    out << YAML::BeginSeq;

                    for (const auto& [hash, state] : layer.States)
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
                
                out << YAML::Key << "Layer Animation Transitions" << YAML::Value;
                {
                    out << YAML::BeginSeq;

                    for (const auto& transition : layer.Transitions)
                    {
                        if (!transition)
                            continue;

                        StringHash source_hash = transition->SourceStateHash;
                        StringHash dest_hash = transition->DestStateHash;
                
                        bool source_valid = (source_hash == StateMachine::DefaultEntryHash) || (layer.States.contains(source_hash) && layer.States[source_hash]);
                        bool dest_valid = (dest_hash == StateMachine::DefaultExitHash) || (layer.States.contains(dest_hash) && layer.States[dest_hash]);
                
                        if (!source_valid || !dest_valid)
                            continue;
                                
                        out << YAML::BeginMap;

                        out << YAML::Key << "Source Name" << YAML::Value << ((source_hash == StateMachine::DefaultEntryHash) ? "Entry State" : layer.States[transition->SourceStateHash]->Name);
                        out << YAML::Key << "Source Hash" << YAML::Value << transition->SourceStateHash;

                        out << YAML::Key << "Destination Name" << YAML::Value << ((dest_hash == StateMachine::DefaultExitHash) ? "Exit State" : layer.States[transition->DestStateHash]->Name);
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
            out << YAML::EndMap;
        }
    
        out << YAML::EndSeq;
    }

    // TODO: Fix this to work with new Serialise
    void StateMachine::Deserialise(const YAML::Node &data)
    {
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
                m_AnimationParameters[param_hash] = std::move(param);
            }
        }

        for (const auto& layer_node : data["State Machine Layers"])
        {
            m_Layers.push_back({});
            Layer& layer = m_Layers.back();

            if (layer_node["Layer Name"])
            {
                layer.LayerName = layer_node["Layer Name"].as<std::string>();
            }

            if (layer_node["Layer Weight"])
            {
                layer.LayerWeight = layer_node["Layer Weight"].as<float>();
            }

            if (layer_node["Layer Blend Type"])
            {
                layer.BlendType = (layer_node["Layer Blend Type"].as<std::string>() == "Additive" ? Layer::LayerBlendType::Additive : Layer::LayerBlendType::Override);
            }

            if (layer_node["Layer Sync Layer"])
            {
                layer.SyncLayer = layer_node["Layer Sync Layer"].as<bool>();
            }

            if (layer_node["Layer Sync Index"])
            {
                layer.LayerIndex = layer_node["Layer Sync Index"].as<size_t>();
            }

            if (layer_node["Layer Use Own Timing"])
            {
                layer.UseOwnLayerTiming = layer_node["Layer Use Own Timing"].as<bool>();
            }

            if (layer_node["Layer Using IK"])
            {
                layer.UsingIK = layer_node["Layer Using IK"].as<bool>();
            }

            if (layer_node["Layer Default State"])
            {
                layer.DefaultState = Louron::Utils::fnv1a_hash(layer_node["Layer Default State"].as<std::string>());
            }

            if (layer_node["Layer Animation States"])
            {
                for(const auto& state : layer_node["Layer Animation States"])
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
                        case StateType::Clip: layer.States[state_hash] = std::make_unique<AnimationState_Clip>(); break; 
                        case StateType::BlendTree: layer.States[state_hash] = std::make_unique<AnimationState_BlendTree>(); break; 
                        case StateType::Unknown: continue; 
                    }
                    
                    layer.States[state_hash]->Name = state["Name"].as<std::string>();
                    layer.States[state_hash]->Deserialise(state);

                    if (state_hash == layer.DefaultState)
                    {
                        CreateTransition(m_Layers.size() - 1, StateMachine::DefaultEntryHash, state_hash);
                    }
                }            
            }

            if (layer_node["Layer Animation Transitions"])
            {
                for(const auto& transition : layer_node["Layer Animation Transitions"])
                {
                    StringHash source_hash = NULL_UUID;
                    StringHash dest_hash = NULL_UUID;
                    
                    if(transition["Source Hash"])
                        source_hash = transition["Source Hash"].as<StringHash>();
                    
                    if(transition["Destination Hash"])
                        dest_hash = transition["Destination Hash"].as<StringHash>();

                    if (source_hash == NULL_UUID || dest_hash == NULL_UUID)
                        continue;
                    
                    auto transition_ptr = CreateTransition(m_Layers.size() - 1, source_hash, dest_hash);
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
    
    #pragma region --State-- Getter and Setters

        void StateMachine::AddLayer(const std::string& layer_name)
        {
            m_Layers.push_back({});
            m_Layers.back().LayerName = layer_name;
        }

        void StateMachine::RemoveLayer(size_t layer_index)
        {
            if (!ValidLayer(layer_index))
                return;
            
            m_Layers.erase(m_Layers.begin() + layer_index);
        }

        StateMachine::Layer* StateMachine::GetLayer(size_t layer_index)
        {
            if (!ValidLayer(layer_index))
                return nullptr;

            return &m_Layers[layer_index];
        }

        std::vector<StateMachine::Layer> &StateMachine::GetLayers()
        {
            return m_Layers;
        }

#pragma endregion

}