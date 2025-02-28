#pragma once

// Louron Core Headers
#include "Component Base.h"

// C++ Standard Library Headers
#include <string>
#include <vector>
#include <utility>

// External Vendor Library Headers

namespace YAML
{
    class Node;
    class Emitter;
}

namespace Louron
{

    struct ScriptComponent : public ComponentBase 
    {
        // Name and Active State of Script
        std::vector<std::pair<std::string, bool>> Scripts;

        ScriptComponent() = default;
        ScriptComponent(const ScriptComponent&) = default;
        ScriptComponent(const std::vector<std::pair<std::string, bool>>& script_name) : Scripts(script_name) {}

        void Serialize(YAML::Emitter& out) const;
        bool Deserialize(const YAML::Node data, UUID entity_uuid);
    };

}