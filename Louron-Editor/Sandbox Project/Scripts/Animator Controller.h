#pragma once

#include "Script Core API/ScriptAPI.h"

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class AnimatorController : public IScript
{

public:

    [[ExposedInEditor]] AnimatorComponent animator;

    [[ExposedInEditor]] Entity shoot_entity;
    [[ExposedInEditor]] Prefab bullet_prefab;

    Animation::StringHash vel_x_hash = Utils::fnv1a_hash("Velocity X");
    Animation::StringHash vel_z_hash = Utils::fnv1a_hash("Velocity Z"); 
    Animation::StringHash jump_hash = Utils::fnv1a_hash("IsJumping"); 
    Animation::StringHash dead_hash = Utils::fnv1a_hash("IsDead"); 

    bool has_jumped = false;
    bool is_dead = false;

    void OnCreate() override
    {
        animator = GetComponent<AnimatorComponent>();

        if (!shoot_entity)
        {
            auto children = GetChildren();
            for(const auto& child : children)
            {
                if (child.GetTag() == "Shoot")
                {
                    shoot_entity = child;
                    break;
                }
            }
        }
    } 

    float Approach(float current, float target, float delta)
    {
        if (current < target)
            return std::min(current + delta, target);
        else
            return std::max(current - delta, target);
    }

    Vector2 direction = { 0.0f, 0.0f };
    float current_speed = 0.0f;
    
    void OnUpdate() override
    {
        if(Input::GetKeyDown(KeyCode::R))
        {
            is_dead = false;
            animator.SetBool(dead_hash, is_dead);

            has_jumped = false;
            animator.SetBool(jump_hash, has_jumped);
            
            animator.ResetMachine();
        }

        if (is_dead)
            return;

        Shoot();
        Animate();
    }

    void Shoot()
    {
        if (!shoot_entity || !bullet_prefab)
        {
            Debug::Log("Shoot Entity or Bullet Prefab Not Set In Editor!", 2);
            return;
        }    

        if (Input::GetMouseButtonDown(MouseButtonCode::Mouse_Button_Left))
        {
            Entity bullet = Instantiate(bullet_prefab);
            bullet.SetPosition(shoot_entity.GetGlobalPosition());
            bullet.SetRotation(shoot_entity.GetGlobalRotation());
            bullet.GetComponent<RigidbodyComponent>().ApplyForce(shoot_entity.GetFront() * 75.0f, RigidbodyComponent::ForceMode::Impulse);
        }
    }

    void Animate()
    {
        constexpr float deadzone        = 0.0001f;
        constexpr float decay_speed     = 3.0f;
    
        constexpr float walk_speed      = 0.5f;
        constexpr float sprint_speed    = 1.0f;
    
        constexpr float walk_accel      = 0.333f; // 0.5 / 1.5s
        constexpr float sprint_accel    = 3.0f;
        constexpr float direction_accel = 4.0f;   // 1.0 / 0.25s
    
        // World-space movement multipliers (how fast we move per unit of speed)
        constexpr float walk_movement_speed   = 1.65f / 1.0f;  // 1.65 units/s
        constexpr float sprint_movement_speed = 2.85f / 0.7f;  // ≈4.07 units/s
    
        constexpr float mouse_sensitivity = 0.1f;
    
        if (has_jumped)
        {
            has_jumped = false;
            animator.SetBool(jump_hash, has_jumped);
        }

        if(Input::GetKeyDown(KeyCode::Space))
        {
            has_jumped = true;
            animator.SetBool(jump_hash, has_jumped);
        }

        if(Input::GetKeyDown(KeyCode::K))
        {
            is_dead = true;
            animator.SetBool(dead_hash, is_dead);
        }
    
        float dt = Time::GetDeltaTime();
        Vector2 input = { 0.0f, 0.0f };
    
        if (Input::GetKey(KeyCode::W)) input.y += 1.0f;
        if (Input::GetKey(KeyCode::S)) input.y -= 1.0f;
        if (Input::GetKey(KeyCode::D)) input.x += 1.0f;
        if (Input::GetKey(KeyCode::A)) input.x -= 1.0f;
    
        bool has_input = input.x != 0.0f || input.y != 0.0f;
    
        float target_speed = 0.0f;
        float accel = 0.0f;
        float move_speed = 0.0f;
    
        if (has_input)
        {
            Vector2 input_dir = input.Normalise();
    
            // Smooth direction change
            direction.x = Approach(direction.x, input_dir.x, direction_accel * dt);
            direction.y = Approach(direction.y, input_dir.y, direction_accel * dt);
    
            bool is_sprinting = Input::GetKey(KeyCode::LeftShift);
            target_speed = is_sprinting ? sprint_speed : walk_speed;
            accel        = is_sprinting ? sprint_accel : walk_accel;
            move_speed   = is_sprinting ? sprint_movement_speed : walk_movement_speed;
    
            current_speed = Approach(current_speed, target_speed, accel * dt);
        }
        else
        {
            current_speed = Approach(current_speed, 0.0f, decay_speed * dt);
            direction.x   = Approach(direction.x, 0.0f, decay_speed * dt);
            direction.y   = Approach(direction.y, 0.0f, decay_speed * dt);
        }
    
        Vector2 final_velocity = direction * current_speed;
    
        if (std::abs(final_velocity.x) < deadzone) final_velocity.x = 0.0f;
        if (std::abs(final_velocity.y) < deadzone) final_velocity.y = 0.0f;
    
        animator.SetFloat(vel_x_hash, final_velocity.x);
        animator.SetFloat(vel_z_hash, final_velocity.y);
    
        // -- Movement --
        Vector3 position = GetPosition();
        Vector3 rotation = GetRotation(); // Euler angles in degrees
    
        // Compute forward and right vectors from Y-rotation
        Vector3 forward = GetFront();
        Vector3 right   = { forward.z, 0.0f, -forward.x }; // 90° rotated right
    
        // Combine input with orientation to get world movement direction
        Vector3 movement =
            (right   * final_velocity.x +
             forward * -final_velocity.y) // move backward relative to forward
            * move_speed * dt;
    
        position = position + movement;
        SetPosition(position);
    
        // -- Rotation --
        static float last_mouse_x = -Input::GetMousePosition().x;
        float current_mouse_x = -Input::GetMousePosition().x;
        float delta_mouse_x = current_mouse_x - last_mouse_x;
        last_mouse_x = current_mouse_x;
    
        rotation.y += delta_mouse_x * mouse_sensitivity;
        SetRotation(rotation);

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

    // Collider Functions
    void OnCollideEnter(const Collider& other) override
    {

    }

    void OnCollideStay(const Collider& other) override
    {

    }

    void OnCollideLeave(const Collider& other) override
    {

    }

    // Collider Trigger Functions
    void OnTriggerEnter(const Collider& other) override
    {

    }

    void OnTriggerStay(const Collider& other) override
    {

    }

    void OnTriggerLeave(const Collider& other) override
    {

    }

private:

};