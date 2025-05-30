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

    public:

        struct Layer;

        using StateMap = std::unordered_map<StringHash, std::unique_ptr<AnimationState>>;

        AssetType GetType() const override { return AssetType::AnimationStateMachine; }

        static StringHash DefaultEntryHash;
        static StringHash DefaultAnyHash;
        static StringHash DefaultExitHash;

    #pragma region Constructors and Operators
    
        StateMachine() = default;
        ~StateMachine() = default;

        StateMachine(const StateMachine& other);
        StateMachine(StateMachine&& other) = default;
        
        StateMachine& operator=(const StateMachine& other);
        StateMachine& operator=(StateMachine&& other) = default;
        
    #pragma endregion

    #pragma region General Functionality
    
        void UpdateStates(float ts);
        void EvaluatePose(Louron::AnimationPose& evaluated_pose);
        
        void ResetMachine();

        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data);
    
    #pragma endregion
    
    #pragma region --State-- Getter and Setters

        void AddLayer(const std::string& layer_name);

        void RemoveLayer(size_t layer_index);

        Layer* GetLayer(size_t layer_index);

        std::vector<Layer>& GetLayers();
        
    #pragma endregion
    
    #pragma region --State-- Getter and Setters
    
        void SetCurrentState(size_t layer_index, const std::string& state_name);
        void SetCurrentState(size_t layer_index, const StringHash& state_hash);
        
        StringHash CreateState(size_t layer_index, const std::string& state_name, StateType state_type);
        void RenameState(size_t layer_index, const StringHash& state_hash, const std::string& state_new_name);
        void RemoveState(size_t layer_index, const std::string& state_name);
        void RemoveState(size_t layer_index, const StringHash& state_hash);
        
        AnimationState* GetAnimationState(size_t layer_index, const std::string& state_name);
        AnimationState* GetAnimationState(size_t layer_index, const StringHash& state_hash);
        AnimationState* GetCurrentAnimationState(size_t layer_index);
        AnimationState* GetPreviousAnimationState(size_t layer_index);
        AnimationState* GetTargetAnimationState(size_t layer_index);

        StringHash GetDefaultStateHash(size_t layer_index) const;
        void SetDefaultStateHash(size_t layer_index, const StringHash& state_hash);

        const StateMap& GetAllStates(size_t layer_index) const;

    #pragma endregion
    
    #pragma region --Transition-- Getter and Setters

        AnimationTransition* CreateTransition(size_t layer_index, const std::string& state_name_from, const std::string& state_name_to);
        AnimationTransition* CreateTransition(size_t layer_index, const StringHash& state_hash_from, const StringHash& state_hash_to);
        void RemoveTransition(size_t layer_index, const std::string& state_name_from, const std::string& state_name_to);
        void RemoveTransition(size_t layer_index, const StringHash& state_hash_from, const StringHash& state_hash_to);
        
        void RemoveAllTransitionFrom(size_t layer_index, const std::string& state_name);
        void RemoveAllTransitionFrom(size_t layer_index, const StringHash& state_hash);

        void RemoveAllTransitionTo(size_t layer_index, const std::string& state_name);
        void RemoveAllTransitionTo(size_t layer_index, const StringHash& state_hash);

        AnimationTransition* GetTransition(size_t layer_index, const std::string& state_name_from, const std::string& state_name_to);
        AnimationTransition* GetTransition(size_t layer_index, const StringHash& state_hash_from, const StringHash& state_hash_to);

        std::vector<std::unique_ptr<AnimationTransition>>* GetAllTransitions(size_t layer_index);

    #pragma endregion
    
    #pragma region --Paramater-- Getter and Setters

        StringHash AddParameter(const std::string& param_name, ParameterType param_type);
        bool HasParameterNamed(const std::string& param_name);
        void RenameParameter(const StringHash& param_hash, const std::string& param_new_name);
        void RemoveParameter(const std::string& param_name);
        void RemoveParameter(const StringHash& param_hash);

        AnimationParameter* GetParameter(const StringHash& param_hash);
        const std::unordered_map<StringHash, AnimationParameter>& GetParameters() const;

        void SetBool(const std::string& param_name, bool value);
        void SetBool(const StringHash& param_hash, bool value);
        
        void SetFloat(const std::string& param_name, float value);
        void SetFloat(const StringHash& param_hash, float value);
        
        void SetInt(const std::string& param_name, int32_t value);
        void SetInt(const StringHash& param_hash, int32_t value);
        
        void SetUInt(const std::string& param_name, uint32_t value);
        void SetUInt(const StringHash& param_hash, uint32_t value);

    #pragma endregion

        struct Layer
        {
            Layer() = default;
            ~Layer() = default;

            Layer(const Layer& other);
            Layer(Layer&& other) = default;
            
            Layer& operator=(const Layer& other);
            Layer& operator=(Layer&& other) = default;

            void EvaluatePose(AnimationPose& evaluated_pose);

            StateMap States  = {};
            std::vector<std::unique_ptr<AnimationTransition>> Transitions = {};

            std::string LayerName = "New State";

            StringHash DefaultState     = StateMachine::DefaultEntryHash;
            StringHash CurrentState     = StateMachine::DefaultEntryHash;
            StringHash PreviousState    = NULL_UUID;
            StringHash TargetState      = NULL_UUID;

            float TransitionCompletion      = 0.0f;      // This will contral the influence of blending CurrentState to TargetState
            float ExitTimeCompletion        = 0.0f;        // Completion of the exit time, this counts where we are at in the exit time when a transition has an exit time of 0.0-1.0+
            float PreviousNormalisedTime    = 0.0f;    // Storing last normalised time as StateNormalisedTime wraps from 1.0 -> 0.0 when going above 1.0
        
            float LayerWeight = 0.0f;

            // TODO: Humanoid HumanoidMask = {};

            enum class LayerBlendType : uint8_t
            {
                Override,   // Will override any layers lower in the vector queue from back to front
                Additive    // Will add ontop of any layers lower in the vector queue form back to front
            } BlendType = LayerBlendType::Override;

            // Is this layer merely a reference to a another layer with different
            // animations to be applied? E.g., injured animations instead of
            // normal walking animations, but same state machine states and
            // transitions as the layer syncing to 
            bool SyncLayer = false; // TODO

            // When syncing layer, the index to the layer to sync against
            size_t LayerIndex = -1; // TODO
            
            // When true this will play animations at their own sample rate
            // using its own normalised time, when false, it will squish or
            // stretch the animation in the Synced Layer to the normalised time
            // and sample rate of the layer being synced to 
            bool UseOwnLayerTiming = false; // TODO

            // Will this layer have IK interactions?
            bool UsingIK = false; // TODO
        };

    private:
    
        bool ValidLayer(size_t layer_index) const;
        static void BlendPoses(const AnimationPose& a, const AnimationPose& b, float t, AnimationPose& result);
        static void AdditiveBlend(const AnimationPose& a, const AnimationPose& b, float weight, AnimationPose& result);

        // Vector of Layers
        std::vector<Layer> m_Layers = {};

        // StringHash == hash of parameter name, Params stored
        // as floats for ease of comparison operations
        std::unordered_map<StringHash, AnimationParameter> m_AnimationParameters;

        friend struct Layer;
    };

}