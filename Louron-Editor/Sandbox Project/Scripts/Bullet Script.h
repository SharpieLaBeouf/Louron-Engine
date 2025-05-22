#pragma once

#include "Script Core API/ScriptAPI.h"

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class BulletScript : public IScript
{

public:

    [[ExposedInEditor]] float timer = 1.0f;

    void OnCreate() override
    {
        
    }

    void OnUpdate() override
    {
        if (timer > 0.0f)
            timer -= Time::GetDeltaTime();
        
        if(timer <= 0.0f)
            Destroy(*this);
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