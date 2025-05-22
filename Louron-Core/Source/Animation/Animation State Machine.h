#pragma once

// Louron Core Headers
#include "Animation Base Types.h"
#include "Animation State.h"
#include "Animation Transition.h"
#include "Animations.h"

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

    class StateMachine : public Asset
    {

        const StringHash m_DefaultEntryHash = Louron::Utils::fnv1a_hash("DEFAULT_ENTRY");
        const StringHash m_DefaultAnyHash   = Louron::Utils::fnv1a_hash("DEFAULT_ANY");
        const StringHash m_DefaultExitHash  = Louron::Utils::fnv1a_hash("DEFAULT_EXIT");

    public:

        AssetType GetType() const override { return AssetType::AnimationStateMachine; }

    #pragma region Constructors and Operators
    
        StateMachine() = default;
        ~StateMachine() = default;

        StateMachine(const StateMachine& other);
        StateMachine(StateMachine&& other) = default;
        
        StateMachine& operator=(const StateMachine& other);
        StateMachine& operator=(StateMachine&& other) = default;
        
    #pragma endregion

        void UpdateStates(float ts);
        void EvaluatePose(Louron::AnimationPose& evaluated_pose);

        void ResetMachine();

        void SetCurrentState(const std::string& state_name);
        void SetCurrentState(const StringHash& state_hash);

        StringHash CreateState(const std::string& state_name, StateType state_type);
        void RenameState(const StringHash& state_hash, const std::string& state_new_name);
        void RemoveState(const std::string& state_name);
        void RemoveState(const StringHash& state_hash);

        AnimationState* GetAnimationState(const std::string& state_name) { return GetAnimationState(Louron::Utils::fnv1a_hash(state_name)); }
        AnimationState* GetAnimationState(const StringHash& state_hash);
        AnimationState* GetCurrentAnimationState() { return GetAnimationState(CurrentState); }
        AnimationState* GetPreviousAnimationState() { return GetAnimationState(PreviousState); }
        AnimationState* GetTargetAnimationState() { return GetAnimationState(TargetState); }

        const std::unordered_map<StringHash, std::unique_ptr<AnimationState>>& GetAllStates() const { return States; }

        AnimationTransition* CreateTransition(const std::string& state_name_from, const std::string& state_name_to);
        AnimationTransition* CreateTransition(const StringHash& state_hash_from, const StringHash& state_hash_to);
        void RemoveTransition(const std::string& state_name_from, const std::string& state_name_to);
        void RemoveTransition(const StringHash& state_hash_from, const StringHash& state_hash_to);
        
        void RemoveAllTransitionFrom(const std::string& state_name);
        void RemoveAllTransitionFrom(const StringHash& state_hash);

        void RemoveAllTransitionTo(const std::string& state_name);
        void RemoveAllTransitionTo(const StringHash& state_hash);

        AnimationTransition* GetTransition(const std::string& state_name_from, const std::string& state_name_to);
        AnimationTransition* GetTransition(const StringHash& state_hash_from, const StringHash& state_hash_to);

        const std::vector<std::unique_ptr<AnimationTransition>>& GetAllTransitions() const { return Transitions; }

        StringHash AddParameter(const std::string& param_name, ParameterType param_type);
        bool HasParameterNamed(const std::string& param_name);
        void RenameParameter(const StringHash& param_hash, const std::string& param_new_name);
        void RemoveParameter(const std::string& param_name);
        void RemoveParameter(const StringHash& param_hash);

        AnimationParameter* GetParameter(const StringHash& param_hash) { if (AnimationParameters.contains(param_hash)) { return &AnimationParameters[param_hash]; } return nullptr; }
        const std::unordered_map<StringHash, AnimationParameter>& GetParameters() const { return AnimationParameters; }

        void SetBool(const std::string& param_name, bool value);
        void SetBool(const StringHash& param_hash, bool value);
        
        void SetFloat(const std::string& param_name, float value);
        void SetFloat(const StringHash& param_hash, float value);
        
        void SetInt(const std::string& param_name, int32_t value);
        void SetInt(const StringHash& param_hash, int32_t value);
        
        void SetUInt(const std::string& param_name, uint32_t value);
        void SetUInt(const StringHash& param_hash, uint32_t value);

        StringHash GetDefaultState() const { return DefaultState; }
        void SetDefaultState(const StringHash& state_hash);

        StringHash GetEntryHash() const { return m_DefaultEntryHash; }
        StringHash GetAnyHash() const { return m_DefaultAnyHash; }
        StringHash GetExitHash() const { return m_DefaultExitHash; }

        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data);

    private:

        void BlendPoses(const AnimationPose& a, const AnimationPose& b, float t, AnimationPose& result);

        std::unordered_map<StringHash, std::unique_ptr<AnimationState>> States = {};
        std::vector<std::unique_ptr<AnimationTransition>> Transitions = {};

        StringHash DefaultState = m_DefaultEntryHash;
        
        StringHash CurrentState = m_DefaultEntryHash;
        StringHash PreviousState = NULL_UUID;
        StringHash TargetState = NULL_UUID;

        float TransitionCompletion = 0.0f;      // This will contral the influence of blending CurrentState to TargetState
        float ExitTimeCompletion = 0.0f;        // Completion of the exit time, this counts where we are at in the exit time when a transition has an exit time of 0.0-1.0+
        float PreviousNormalisedTime = 0.0f;    // Storing last normalised time as StateNormalisedTime wraps from 1.0 -> 0.0 when going above 1.0

        // StringHash == hash of parameter name, Params stored
        // as floats for ease of comparison operations
        std::unordered_map<StringHash, AnimationParameter> AnimationParameters;
        
        Louron::AnimationPose CurrentPose = {};
    };

    /*  State Machine Test - MANUAL
        
        using namespace Louron::Animation;
        StateMachine machine = {};
        StringHash idle_hash = machine.CreateState("Idle", StateType::Clip);
        StringHash walk_hash = machine.CreateState("Walk", StateType::Clip);
        StringHash run_hash = machine.CreateState("Run", StateType::Clip);

        auto idle_to_walk = machine.CreateTransition(idle_hash, walk_hash);
        idle_to_walk->Conditions.emplace_back("IsWalking", Louron::Utils::fnv1a_hash("IsWalking"), ComparisonType::Equal, 1.0f);
        idle_to_walk->TransitionDuration = 2.0f; // <-- Testing non-zero transition

        auto walk_to_idle = machine.CreateTransition(walk_hash, idle_hash);
        walk_to_idle->Conditions.emplace_back("IsWalking", Louron::Utils::fnv1a_hash("IsWalking"), ComparisonType::Equal, 0.0f);
        walk_to_idle->TransitionDuration = 0.0f;

        auto walk_to_run = machine.CreateTransition(walk_hash, run_hash);
        walk_to_run->Conditions.emplace_back("IsRunning", Louron::Utils::fnv1a_hash("IsRunning"), ComparisonType::Equal, 1.0f);
        walk_to_run->TransitionDuration = 0.0f;

        auto run_to_walk = machine.CreateTransition(run_hash, walk_hash);
        run_to_walk->Conditions.emplace_back("IsRunning", Louron::Utils::fnv1a_hash("IsRunning"), ComparisonType::Equal, 0.0f);
        run_to_walk->TransitionDuration = 0.0f;
        
        machine.AddParameter("IsWalking", ParameterType::Bool);
        machine.AddParameter("IsRunning", ParameterType::Bool);

        // Initial update
        machine.UpdateStates(1.0f);
        L_APP_INFO("Initial State: {}", machine.GetCurrentAnimationState()->Name);

        // Begin transition to "Walk"
        machine.SetBool(Louron::Utils::fnv1a_hash("IsWalking"), true);

        // Simulate transition over multiple updates
        for (float time = 0.0f; time <= 2.5f; time += 0.5f)
        {
            machine.UpdateStates(0.5f);
            L_APP_INFO("[t = {}s] Current State: {}", time + 0.5f, machine.GetCurrentAnimationState()->Name);
        }

        // Now trigger Run
        machine.SetBool(Louron::Utils::fnv1a_hash("IsRunning"), true);
        machine.UpdateStates(1.0f);
        L_APP_INFO("Current State: {}", machine.GetCurrentAnimationState()->Name);

        // Revert to Idle through Walk
        machine.SetBool(Louron::Utils::fnv1a_hash("IsRunning"), false);
        machine.SetBool(Louron::Utils::fnv1a_hash("IsWalking"), false);

        machine.UpdateStates(1.0f);
        L_APP_INFO("Current State: {}", machine.GetCurrentAnimationState()->Name);

        machine.UpdateStates(1.0f);
        L_APP_INFO("Final State: {}", machine.GetCurrentAnimationState()->Name);

    */

    /*  State Machine Test - SERIALISE/DESERIALISE

    
        // To Save and Serialise

            YAML::Emitter out;
            {
                out << YAML::BeginMap;

                machine.Serialise(out);

                out << YAML::EndMap;
            }

            std::ofstream fout("Test.lanimator"); // Create the file
            fout << out.c_str(); // Save
            fout.close();
                    
        // To Load and Deserialise
        
            using namespace Louron::Animation;

	        StateMachine machine = {};
            YAML::Node data;
            try 
            {
                data = YAML::LoadFile("Test.lanimator");
                machine.Deserialise(data);
            }
            catch (YAML::ParserException e) 
            {
                L_APP_ERROR("YAML-CPP Failed to Load Test Anim State Machine File: \'{}\', {}", std::filesystem::absolute("Test.lanimstates").string(), e.what());
            }
    
    */

}