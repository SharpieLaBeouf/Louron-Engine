#pragma once

#include "Louron.h"

#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;
struct AnimatorPanel 
{

private:

    static inline Louron::AssetHandle s_AssetContext = NULL_UUID;

public:

    static void SetAnimatorAssetContext(Louron::AssetHandle animator_asset) { s_AssetContext = animator_asset; }

    static void Draw(const Louron::Animation::StateMachine& machine)
    {
        using namespace Louron::Animation;

        ed::Begin("StateMachineGraph");

        // Draw each state as a node
        for (const auto& [hash, state_ptr] : machine.GetAllStates())
        {
            if (!state_ptr)
                continue;

            ed::BeginNode(hash);
            ImGui::Text("%s", state_ptr->Name.c_str());
            ed::EndNode();
        }

        ed::End();
    }
};