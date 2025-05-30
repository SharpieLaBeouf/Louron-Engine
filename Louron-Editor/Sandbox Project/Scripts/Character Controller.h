#pragma once

#include "Script Core API/ScriptAPI.h"

#include <format>

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class CharacterController : public Louron::IScript
{

public:

    [[ExposedInEditor]] float speed = 5.0f;
    [[ExposedInEditor]] float rotation_speed = 90.0f;
    [[ExposedInEditor]] float rotation_smoothing = 1.0f;

    BasicAnimationComponent animator;

    enum class MovementState : uint8_t
    {
        Idle = 0,
        Walk,
        Run,
        Jump
    } movement_state = MovementState::Idle;

    std::array<std::string, 4> animation_clip_names = 
    {
        "Locomotion_Idle_Game Ready",
        "Locomotion_Walking_Game Ready",
        "Locomotion_Running_Game Ready",
        "Locomotion_Jump_Game Ready"
    };

    void OnCreate() override
    {
        animator = GetComponent<BasicAnimationComponent>();
    }

    void OnUpdate() override
    {
        IVector2 direction = { 0, 0 };
        MovementState frame_start_state = movement_state;

        { // Check General Input
            if(Input::GetKey(KeyCode::W))
            {
                direction.x++;
            }
            if(Input::GetKey(KeyCode::S))
            {
                direction.x--;
            }
            if(Input::GetKey(KeyCode::A))
            {
                direction.y++;
            }
            if(Input::GetKey(KeyCode::D))
            {
                direction.y--;
            }
        }

        // Input Handling
        if(movement_state != MovementState::Jump)
        {
            // Check Idle, Walk, Run
            if (direction == IVector2::Zero())
            {
                movement_state = MovementState::Idle;
            }
            else if (Input::GetKey(KeyCode::LeftShift))
            {
                movement_state = MovementState::Run;
            }
            else
            {
                movement_state = MovementState::Walk;
            }

            if(Input::GetKeyDown(KeyCode::Space))
            {
                movement_state = MovementState::Jump;
            }
        }
        else
        {
            // Jump Animation Finished
            if (animation_clip_names[(uint8_t)MovementState::Jump] == animator.GetCurrentClipName()  && !animator.IsPlaying())
            {
                // Check Idle, Walk, Run
                if (direction == IVector2::Zero())
                {
                    movement_state = MovementState::Idle;
                }
                else if (Input::GetKey(KeyCode::LeftShift))
                {
                    movement_state = MovementState::Run;
                }
                else
                {
                    movement_state = MovementState::Walk;
                }
            }
        }
        
        // Check State Changes
        if(movement_state != frame_start_state)
        {
            // Change and Start Playing Another Animation
            animator.PlayAnimation(animation_clip_names[(uint8_t)movement_state].c_str(), movement_state != MovementState::Jump);
            Debug::Log(std::format("Changing Animation From \"{}\" to \"{}\"", 
                animation_clip_names[(uint8_t)frame_start_state], 
                animation_clip_names[(uint8_t)movement_state]));
        }
        
        if (direction != IVector2::Zero())
        {
            // -- Movement Translation --
            Vector3 position = GetPosition();

            Quaternion rotation = GetRotation();
            Vector3 forward = rotation.Rotate(Vector3(0.0f, 0.0f, -1.0f)); // local -Z

            float movement_multiplier = (movement_state == MovementState::Run) ? 2.0f : 1.0f;
            float delta_time = Time::GetDeltaTime();

            position = position + (-forward * direction.x * speed * movement_multiplier * delta_time);
            SetPosition(position);

            // -- Yaw Rotation (Euler manipulation) --
            Vector3 euler = rotation.ToEuler();
            float current_yaw = euler.y;
            float target_yaw = current_yaw + direction.y * rotation_speed * delta_time;

            // Smooth interpolation
            float smoothed_yaw = LMath::Lerp(rotation_smoothing * delta_time, current_yaw, target_yaw);
            euler = Vector3(0.0f, smoothed_yaw, 0.0f);

            SetRotation(Quaternion::Euler(euler));
        }
    }

    void OnLateUpdate() override
    {

    }

    void OnFixedUpdate() override
    {

    }

    void OnDestroy() override
    {

    }

private:

};