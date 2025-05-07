#pragma once

#include "Script Core API/ScriptAPI.h"

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class <<SCRIPT_NAME>> : public IScript
{

public:

    [[ExposedInEditor]] const char* my_string = "Hello World!";
    [[ExposedInEditor]] bool my_bool = false;

    void OnCreate() override
    {
            
    }

    void OnUpdate() override
    {
            
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