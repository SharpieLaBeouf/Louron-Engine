#pragma once

#include "Script Core API/ScriptAPI.h"

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class CollissionPush : public IScript
{

public:

    static inline bool pushed;

    float MaxPushForce = 20.0f;

    void OnCreate() override
    {
        pushed = false;
    }

    void OnUpdate() override
    {
        static float timer = 1.0f;
        if(GetTag() == "Player 1" && pushed)
        {
            timer -= Time::GetDeltaTime();
            if(timer < 0.0f)
            {
                pushed = false;
                timer = 1.0f;
            }
        }
    }

    // Collider Functions
    void OnCollideEnter(const Collider& other) override
    {        
        if (pushed)
            return;

        if ((GetTag() == "Player 1" && other.entity.GetTag() == "Player 2") || (GetTag() == "Player 2" && other.entity.GetTag() == "Player 1")) 
        {
            if (!HasComponent<RigidbodyComponent>() || !other.entity.HasComponent<RigidbodyComponent>())
                return;

            Vector3 self_velocity = GetComponent<RigidbodyComponent>().GetLinearVelocity();
            Vector3 other_velocity = other.entity.GetComponent<RigidbodyComponent>().GetLinearVelocity();

            if (self_velocity.Length() < other_velocity.Length()) 
            {
                // Calculate the direction from A to B
                Vector3 direction = GetPosition() - other.entity.GetPosition();
                direction.Normalise();

                // Calculate impulse magnitude based on other persons velocity
                float impulseMagnitude = LMath::Max(other_velocity.Length() * 100.0f, MaxPushForce); // length or length squared?

                GetComponent<RigidbodyComponent>().ApplyForce(direction * impulseMagnitude, RigidbodyComponent::ForceMode::Force);
                pushed = true;
            }
        }
    }

private:

};