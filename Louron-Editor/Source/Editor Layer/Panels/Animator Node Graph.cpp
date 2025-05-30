#include "Animator Node Graph.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

#include <imgui.h>
#include <imgui_internal.h>

using namespace Louron;

static bool s_Edited = false;

void AnimatorPanel::Draw(bool& show_window)
{
    using namespace Louron::Animation;

    #pragma region Setup Node Editor Context 

    static ax::NodeEditor::EditorContext* context = nullptr;
    static std::string settings_path_string = "";

    if(!context)
    {
        ax::NodeEditor::Config config;

        if(!std::filesystem::exists(Project::GetActiveProject()->GetProjectDirectory() / "Editor/GUI/"))
            std::filesystem::create_directories(Project::GetActiveProject()->GetProjectDirectory() / "Editor/GUI/");

        std::filesystem::path settings_path = Project::GetActiveProject()->GetProjectDirectory() / "Editor/GUI/Animator.json";
        settings_path_string = settings_path.generic_string();
        config.SettingsFile = settings_path_string.c_str();
        context = ax::NodeEditor::CreateEditor(&config);
    }

    ax::NodeEditor::SetCurrentEditor(context);

    #pragma endregion

    auto machine_asset = AssetManager::GetAsset<StateMachine>(s_AssetContext);

    if (!machine_asset)
        return;

    if (!machine_asset->GetLayer(s_SelectedLayer))
    {
        s_NodeMap.clear();
        s_TransitionMap.clear();
        s_SelectedLayer = 0; // Reset to Base Layer
        return;
    }

    ImGuiWindowFlags window_flags = (s_Edited) ? ImGuiWindowFlags_UnsavedDocument : 0;
    if (ImGui::Begin("AnimatorStateMachine", &show_window, window_flags)) 
    {
        #pragma region Navigation

        // Example Top Bar Following Same Design as Context Browser Panel
        ImGui::Text("Navigation:");
        ImGui::SameLine();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

        if (ImGui::Button("State Machine Base")) 
        {
            s_StateBlendTreeNode = NULL_UUID;
            s_BlendTreeChildLevel.clear();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        if (auto blend_state = machine_asset->GetAnimationState(s_SelectedLayer, s_StateBlendTreeNode); s_StateBlendTreeNode != NULL_UUID && blend_state)
        {
            ImGui::SameLine();
            ImGui::Text("/");
            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            if (ImGui::Button(blend_state->Name.c_str()))
            {
                s_BlendTreeChildLevel.clear();
            }
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();

            BlendNode* blend_node = &reinterpret_cast<AnimationState_BlendTree*>(blend_state)->AnimBlendTree;

            for (int i = 0; i < s_BlendTreeChildLevel.size(); i++)
            {
                if (!blend_node)
                    break;

                const size_t& index = s_BlendTreeChildLevel[i];
                if (!blend_node->ChildNode.empty() && index >= 0 && index < blend_node->ChildNode.size())
                {
                    if (blend_node->ChildNode[index] && blend_node->ChildNode[index]->GetType() == MotionType::BlendTree)
                    {
                        blend_node = &reinterpret_cast<MotionBlendTree*>(blend_node->ChildNode[index].get())->RootNode;
                    }
                    else
                    {
                        s_BlendTreeChildLevel.erase(s_BlendTreeChildLevel.begin() + i, s_BlendTreeChildLevel.end());
                        break;
                    }
                }

                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();
    
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    
                if (ImGui::Button(blend_node->Name.c_str())) 
                {
                    if (i < s_BlendTreeChildLevel.size() - 1)
                    {
                        s_BlendTreeChildLevel.erase(s_BlendTreeChildLevel.begin() + i + 1, s_BlendTreeChildLevel.end());
                    }
                    
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar();
                    break;
                }
    
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar();
            }
        }

        #pragma endregion

        ImGuiTableFlags table_flags = 
            ImGuiTableFlags_BordersInnerV   |
            ImGuiTableFlags_Resizable        |
            ImGuiTableFlags_NoPadOuterX;
        if (ImGui::BeginTable("##GraphLayout", 3, table_flags))
        {
            ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthFixed,  0.10f);
            ImGui::TableSetupColumn("Graph"     , ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Details"   , ImGuiTableColumnFlags_WidthFixed,  0.20f);

        #pragma region Left Panel - Parameters and Layers

        ImGui::TableNextColumn();
        ImGui::BeginChild("##LeftPane", ImVec2(0,0), true);

        // Setup Tabs
        if (ImGui::BeginTabBar("##TabBar", ImGuiTabBarFlags_DrawSelectedOverline))
        {
            static bool open_settings_popup = false;
            static int popup_layer_index = -1;
            
            static bool renaming_layer = false;
            static int renaming_layer_index = -1;

            if (ImGui::BeginTabItem("Parameters"))
            {
                #pragma region Parameters

                static int new_param_counter = 1;
                static ParameterType new_param_type = ParameterType::Float;
                static std::vector<Louron::Animation::StringHash> parameters_to_remove;

                ImGui::TableNextColumn();
                ImGui::BeginChild("##LeftPane_Params", ImVec2(0, 0), true);

                static bool renaming = false;
                static char renaming_buf[256];
                static StringHash renaming_hash = NULL_UUID;

                if (static bool once = true; once)
                {
                    memset(renaming_buf, 0, sizeof(renaming_buf));
                    once = false;
                }

                if (!renaming && renaming_hash != NULL_UUID)
                {
                    if (renaming_buf[0] != '\0')
                    {
                        std::string base_name = renaming_buf;
                        std::string new_param_name = base_name;
                        int i = 1;
                        while (machine_asset->HasParameterNamed(new_param_name))
                        {
                            new_param_name = base_name + " " + std::to_string(i++);
                        }

                        machine_asset->RenameParameter(renaming_hash, new_param_name);
                    }

                    renaming_hash = NULL_UUID;
                    memset(renaming_buf, 0, sizeof(renaming_buf));
                }

                const auto& parameters = machine_asset->GetParameters();
                for (const auto& [hash, parameter] : parameters)
                {
                    ImGui::PushID(static_cast<int>(hash));

                    ImGui::Dummy({0.0f, 1.5f});
                    ImGui::BeginGroup();

                    if (renaming && hash == renaming_hash)
                    {
                        if (ImGui::InputText("##ParamName", renaming_buf, sizeof(renaming_buf), ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            renaming = false;
                        }

                        if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                        {
                            renaming = false;
                        }
                    }
                    else
                    {
                        std::string label = parameter.Name + " (" + Louron::Animation::Utils::ParamTypeToString(parameter.Type) + "):";
                        ImGui::TextUnformatted(label.c_str());

                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            renaming = true;
                            renaming_hash = hash;

                        #if defined(L_PLATFORM_WINDOWS)
                            strncpy_s(renaming_buf, parameter.Name.c_str(), sizeof(renaming_buf));
                        #else
                            strncpy(renaming_buf, parameter.Name.c_str(), sizeof(renaming_buf));
                        #endif
                        }
                    }

                    ImGui::SameLine();

                    const float button_width = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                    const float spacing = ImGui::GetStyle().ItemSpacing.x;

                    float total_available_width = ImGui::GetContentRegionAvail().x;
                    float input_width = total_available_width - button_width - spacing;

                    switch (parameter.Type)
                    {
                        case ParameterType::Bool:
                        {
                            bool value = static_cast<bool>(parameter.Value);
                            if (ImGui::Checkbox("##Value", &value))
                            {
                                machine_asset->SetBool(hash, value);
                                s_Edited = true;
                            }
                            break;
                        }
                        case ParameterType::Int:
                        {
                            int32_t value = static_cast<int32_t>(parameter.Value);
                            ImGui::SetNextItemWidth(input_width);
                            if (ImGui::InputScalar("##Value", ImGuiDataType_S32, &value))
                            {
                                machine_asset->SetInt(hash, value);
                                s_Edited = true;
                            }
                            break;
                        }
                        case ParameterType::UInt:
                        {
                            uint32_t value = static_cast<uint32_t>(parameter.Value);
                            ImGui::SetNextItemWidth(input_width);
                            if (ImGui::InputScalar("##Value", ImGuiDataType_U32, &value))
                            {
                                machine_asset->SetUInt(hash, value);
                                s_Edited = true;
                            }
                            break;
                        }
                        case ParameterType::Float:
                        {
                            float value = static_cast<float>(parameter.Value);
                            ImGui::SetNextItemWidth(input_width);
                            if (ImGui::InputFloat("##Value", &value))
                            {
                                machine_asset->SetFloat(hash, value);
                                s_Edited = true;
                            }
                            break;
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("X"))
                    {
                        parameters_to_remove.push_back(hash);
                    }

                    ImGui::EndGroup();
                    ImGui::PopID();
                }

                ImGui::Dummy({0.0f, 5.0f});
                ImGui::Separator();
                ImGui::Dummy({0.0f, 2.0f});

                ImGui::Text("Add New Parameter");

                if (ImGui::BeginCombo("##NewType", Louron::Animation::Utils::ParamTypeToString(new_param_type).c_str()))
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        auto type = static_cast<ParameterType>(i);
                        bool selected = (type == new_param_type);
                        if (ImGui::Selectable(Louron::Animation::Utils::ParamTypeToString(type).c_str(), selected))
                            new_param_type = type;
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                static char buf[256] = {};

                ImGui::Text("Parameter Name");
                ImGui::InputText("##ParamName", buf, sizeof(buf));

                if (ImGui::Button("Add"))
                {
                    std::string base_name = buf;
                    std::string new_name = base_name;

                    while(machine_asset->HasParameterNamed(new_name))
                    {
                        new_name = base_name + " " + std::to_string(new_param_counter++);
                    }

                    machine_asset->AddParameter(new_name, new_param_type);
                    s_Edited = true;

                    memset(buf, 0, sizeof(buf));
                }

                ImGui::EndChild();

                for (const auto& hash : parameters_to_remove)
                {
                    machine_asset->RemoveParameter(hash);
                    s_Edited = true;
                }
                parameters_to_remove.clear();

                #pragma endregion

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Layers"))
            {
                ImGui::TableNextColumn();
                ImGui::BeginChild("##LeftPane_Layers", ImVec2(0, 0), true);

                auto& layers = machine_asset->GetLayers();
                for (int i = 0; i < layers.size(); ++i)
                {
                    ImGui::PushID(i);

                    ImGui::BeginGroup();
                    ImVec2 content_region = ImGui::GetContentRegionAvail();
                    float line_height = 30.0f;
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

                    bool selected = (s_SelectedLayer == i);
                    if (ImGui::Selectable("##LayerSelectable", selected, 0, ImVec2(content_region.x, line_height)))
                    {
                        s_SelectedLayer = i;
                        s_NodeMap.clear();
                        s_TransitionMap.clear();
                    }

                    static int context_popup_layer = -1;
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        ImGui::OpenPopup("LayerContextPopup");
                        context_popup_layer = i;
                    }

                    if (ImGui::BeginPopup("LayerContextPopup"))
                    {
                        if (ImGui::MenuItem("Rename Layer"))
                        {
                            // TODO: IMPLEMENT RENAMING LAYERS
                            renaming_layer = true;
                            renaming_layer_index = context_popup_layer;

                            context_popup_layer = -1;
                        }

                        if (context_popup_layer == 0)
                            ImGui::BeginDisabled();

                        if (ImGui::MenuItem("Delete Layer"))
                        {
                            machine_asset->RemoveLayer(static_cast<size_t>(context_popup_layer));
                            --i;
						    ImGui::EndPopup();
                            context_popup_layer = -1;
                            ImGui::EndGroup();
                            ImGui::PopID();
                            ImGui::PopStyleVar();
                            s_Edited = true;
                            continue;
                        }
                        
                        if (context_popup_layer == 0)
                            ImGui::EndDisabled();

						ImGui::EndPopup();
                    }

                    ImGui::PopStyleVar();

                    ImGui::SameLine(10);

                    ImGui::TextUnformatted(layers[i].LayerName.c_str());

                    ImGui::SameLine(content_region.x - 30 - ImGui::CalcTextSize("Options").x);

                    // Highlight if hovered
                    static std::unordered_map<int, bool> options_hovered;

                    if (options_hovered[i])
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                    else
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));

                    ImGui::Button(("Options##" + std::to_string(i)).c_str());

                    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) // Bloody ImGui so difficult to work with sometimes...
                        options_hovered[i] = ImGui::IsItemHovered();

                    bool first_click_options_button = false;
                    if (options_hovered[i] && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
                    {
                        first_click_options_button = true;
                        open_settings_popup = true;
                        popup_layer_index = i;
                    } 

                    ImGui::PopStyleColor();

                    if (open_settings_popup && popup_layer_index == i)
                    {
                        ImGui::OpenPopup("LayerSettingsPopup");

                        ImGui::SetNextWindowSize({ 400.0f, 200.0f });
                    }

                    bool is_window_opened = false;
                    bool is_window_hovered = false;
                    if (ImGui::BeginPopup("LayerSettingsPopup"))
                    {
                        is_window_opened = true;
                        is_window_hovered = ImGui::IsWindowHovered();

                        ImGui::Columns(2, "LayerOptionsCols", false);
		                ImGui::SetColumnWidth(-1, ImGui::CalcTextSize("Use Own Layer Timing:").x + 10.0f);

                        if (popup_layer_index == 0)
                            ImGui::BeginDisabled();

                        ImGui::Text("Weight:");

                        ImGui::NextColumn();
                        
                        ImGui::SetNextItemWidth(-1.0f);
                        if (ImGui::SliderFloat(("##LayerWeightSlider" + std::to_string(popup_layer_index)).c_str(), &layers[i].LayerWeight, 0.0f, 1.0f, "%.2f"))
                            s_Edited = true;

                        if (ImGui::IsItemHovered())
                            is_window_hovered = true;

                        ImGui::NextColumn();
                        
                        std::array<const char*, 2> layer_blend_types = { "Override", "Additive" };
                        uint8_t item_current = static_cast<uint8_t>(layers[i].BlendType);
                        ImGui::Text("Blend Type:");
                        
                        ImGui::NextColumn();
                        ImGui::SetNextItemWidth(-1.0f);
                        
                        if (ImGui::BeginCombo("##LayerBlendTypeCombo", layer_blend_types[item_current])) {

                            if (ImGui::IsItemHovered())
                                is_window_hovered = true;

                            for (int n = 0; n < layer_blend_types.size(); n++)
                            {
                                const bool is_selected = (item_current == n);
                                if (ImGui::Selectable(layer_blend_types[n], is_selected))
                                {
                                    s_Edited = true;
                                    item_current = n;
                                    layers[i].BlendType = static_cast<StateMachine::Layer::LayerBlendType>(item_current);
                                }

                                if (ImGui::IsItemHovered())
                                    is_window_hovered = true;

                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }

                            ImGui::EndCombo();
                        }
                        
                        ImGui::NextColumn();

                        ImGui::Text("Sync:");

                        ImGui::NextColumn();
                        ImGui::SetNextItemWidth(-1.0f);

                        if (ImGui::Checkbox("##SyncLayerCheck", &layers[i].SyncLayer) && layers[i].SyncLayer)
                        {
                            s_Edited = true;
                            layers[i].LayerIndex = 0;
                        }

                        if (ImGui::IsItemHovered())
                            is_window_hovered = true;

                        ImGui::NextColumn();

                        if (layers[i].SyncLayer)
                        {                            
                            std::vector<std::string> layer_names;
                            layer_names.reserve(layers.size());

                            for (int j = 0; j < layers.size(); j++)
                            {
                                layer_names.push_back(layers[j].LayerName);
                            }

                            uint8_t item_current = layers[i].LayerIndex;
                            ImGui::Text("State to Sync:");
                            
                            ImGui::NextColumn();
                            ImGui::SetNextItemWidth(-1.0f);
                            
                            if (ImGui::BeginCombo("##StateToSyncCombo", ((item_current >= 0 && item_current < layer_names.size()) ? layer_names[item_current].c_str() : "None"))) 
                            {
                                if (ImGui::IsItemHovered())
                                    is_window_hovered = true;

                                for (int n = 0; n < layer_names.size(); n++)
                                {
                                    const bool is_selected = (item_current == n);
                                    if (ImGui::Selectable(layer_names[n].c_str(), is_selected))
                                    {
                                        s_Edited = true;
                                        item_current = n;
                                        layers[i].LayerIndex = item_current;
                                    }

                                    if (ImGui::IsItemHovered())
                                        is_window_hovered = true;

                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();
                                }

                                ImGui::EndCombo();
                            }

                            ImGui::NextColumn();

                            ImGui::Text("Use Own Layer Timing:");
                            
                            ImGui::NextColumn();
                            ImGui::SetNextItemWidth(-1.0f);

                            if (ImGui::Checkbox("##UseOwnTimingButton", &layers[i].UseOwnLayerTiming))
                                s_Edited = true;
                        
                            if (ImGui::IsItemHovered())
                                is_window_hovered = true;

                            ImGui::NextColumn();

                        }
                        else
                        {
                            layers[i].LayerIndex = -1;
                        }

                        ImGui::Text("Using IK:");
                            
                        ImGui::NextColumn();
                        ImGui::SetNextItemWidth(-1.0f);

                        if (ImGui::Checkbox("##UsingIKButton", &layers[i].UsingIK))
                            s_Edited = true;

                        if (ImGui::IsItemHovered())
                            is_window_hovered = true;

                        ImGui::NextColumn();
                        
                        if (popup_layer_index == 0)
                            ImGui::EndDisabled();

                        ImGui::Columns(1);

                        ImGui::EndPopup();
                    }
                    
                    if (is_window_opened && !is_window_hovered && !first_click_options_button && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        open_settings_popup = false;
                        popup_layer_index = -1;
                    }

                    ImGui::EndGroup();
                    ImGui::PopID();
                }

                ImGui::Dummy({0.0f, 1.5f});
                ImGui::Separator();
                ImGui::Dummy({0.0f, 1.5f});

                // HERE: Implement Add Layer Button, Centered

                ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x * 0.5f) - (ImGui::CalcTextSize("Add Layer").x + ImGui::GetStyle().FramePadding.x * 2.0f) * 0.5f);
                if(ImGui::Button("Add Layer"))
                {
                    machine_asset->AddLayer("New Layer");

                    layers = machine_asset->GetLayers();
                    size_t layer_index = layers.size() - 1;

                    layers[layer_index].LayerWeight = 1.0f;
                    machine_asset->CreateState(layer_index, "Default Animation State", Louron::Animation::StateType::Clip);
                    machine_asset->CreateState(layer_index, "Default Blend Tree State", Louron::Animation::StateType::BlendTree);

                    s_SelectedLayer = layer_index;
                    s_NodeMap.clear();
                    s_TransitionMap.clear();

                    s_Edited = true;
                }

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndChild();

        #pragma endregion

        #pragma region Node Editor
        
            ImGui::TableNextColumn();
            ImGuiWindowFlags centerFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            ImGui::BeginChild("##GraphPane", ImVec2(0,0), false, centerFlags);

            if (auto blend_state = machine_asset->GetAnimationState(s_SelectedLayer, s_StateBlendTreeNode); s_StateBlendTreeNode != NULL_UUID && blend_state && blend_state->GetType() == StateType::BlendTree)
            {
                BlendNode* node = &reinterpret_cast<AnimationState_BlendTree*>(blend_state)->AnimBlendTree;

                for (int i = 0; i < s_BlendTreeChildLevel.size(); i++)
                {
                    const size_t& index = s_BlendTreeChildLevel[i];
                    if (!node->ChildNode.empty() && index >= 0 && index < node->ChildNode.size())
                    {
                        if (node->ChildNode[index] && node->ChildNode[index]->GetType() == MotionType::BlendTree)
                        {
                            node = &reinterpret_cast<MotionBlendTree*>(node->ChildNode[index].get())->RootNode;
                            continue;
                        }
                        else
                        {
                            s_BlendTreeChildLevel.erase(s_BlendTreeChildLevel.begin() + i, s_BlendTreeChildLevel.end());
                            break;
                        }
                    }
                }

                DrawBlendTree(*node);
            }
            else
            {
                if (s_StateBlendTreeNode != NULL_UUID)
                    s_StateBlendTreeNode = NULL_UUID;

                if (!s_BlendTreeChildLevel.empty())
                    s_BlendTreeChildLevel.clear();

                AnimatorPanel::DrawGraph(*machine_asset.get());
            }


            ImGui::EndChild();

        #pragma endregion

        #pragma region Inspector

            ImGui::TableNextColumn();
            ImGui::BeginChild("##RightPane", ImVec2(0,0), true);
            ImGui::Text("Inspector");
            
            ImGui::Dummy({0.0f, 1.5f});
            ImGui::Separator();
            ImGui::Dummy({0.0f, 1.5f});

            auto draw_base_inspector = [&]() -> void
            {
                if (s_StateBlendTreeNode != NULL_UUID)
                    s_StateBlendTreeNode = NULL_UUID;

                if (!s_BlendTreeChildLevel.empty())
                    s_BlendTreeChildLevel.clear();

                if (auto state = machine_asset->GetAnimationState(s_SelectedLayer, s_SelectedNode); s_SelectedNode != NULL_UUID && state)
                {
                    ImGui::Text("State Name:");
    
                    ImGui::SameLine();
    
                    static StringHash previous_node = NULL_UUID;
                    static char state_name_buf[256] = {};
                    if(static bool once = true; once)
                    {
                        once = false;
                        memset(state_name_buf, 0, sizeof(state_name_buf));
                    }
    
                    if (previous_node != s_SelectedNode && s_SelectedNode != NULL_UUID)
                    {
    
                    #if defined(L_PLATFORM_WINDOWS)
                        strncpy_s(state_name_buf, state->Name.c_str(), sizeof(state_name_buf));
                    #else
                        strncpy(state_name_buf, state->Name.c_str(), sizeof(state_name_buf));
                    #endif
    
                    }
                    previous_node = s_SelectedNode;
    
                    if (ImGui::InputText("##StateNameInput", state_name_buf, sizeof(state_name_buf), ImGuiInputTextFlags_EnterReturnsTrue) || (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
                    {
                        machine_asset->RenameState(s_SelectedLayer, s_SelectedNode, state_name_buf);
    
                        s_SelectedNode = Louron::Utils::fnv1a_hash(state_name_buf);
                        previous_node = s_SelectedNode;
    
                        state = machine_asset->GetAnimationState(s_SelectedLayer, state_name_buf);
    
                        s_NodeMap.clear();
                        s_TransitionMap.clear();
                    }
    
                    ImGui::Text("Default State:");
                    ImGui::SameLine();
    
                    bool is_current_default = (machine_asset->GetDefaultStateHash(s_SelectedLayer) == s_SelectedNode);
                    bool make_default = is_current_default;
                    
                    if (!is_current_default)
                    {
                        if (ImGui::Checkbox("##DefaultStateCheckbox", &make_default) && make_default)
                        {
                            machine_asset->SetDefaultStateHash(s_SelectedLayer, s_SelectedNode);
    
                            s_NodeMap.clear();
                            s_TransitionMap.clear();
                        }
                    }
                    else
                    {
                        ImGui::BeginDisabled();
                        bool dummy = true;
                        ImGui::Checkbox("##DefaultStateCheckbox", &dummy);
                        ImGui::EndDisabled();
                    }
        
                    switch (state->GetType())
                    {
                        case StateType::Clip:
                        {
                            ImGui::Text("State Type: Animation Clip");
        
                            AnimationState_Clip* state_clip = reinterpret_cast<AnimationState_Clip*>(state);
        
                            ImGui::Text("Is Playing: ");
                            ImGui::SameLine();
                            ImGui::BeginDisabled();
                            ImGui::Checkbox("##IsPlaying", &state_clip->IsPlaying);
                            ImGui::EndDisabled();
                            
                            ImGui::Text("Is Looping: ");
                            ImGui::SameLine();
                            if (ImGui::Checkbox("##IsLooping", &state_clip->IsLooping)) 
                                s_Edited = true;
                            
                            ImGui::Text("Current Normalised State Time: ");
                            ImGui::SameLine();
        
                            ImGui::BeginDisabled();
                            ImGui::SliderFloat("##CurrentTime", &state_clip->NormalisedStateTime, 0.0f, 1.0f, "%.2f");
                            ImGui::EndDisabled();

                            ImGui::Text("Playback Speed: ");
                            ImGui::SameLine();
                            if (ImGui::DragFloat("##PlaybackSpeed", &state_clip->PlaybackSpeed, 0.01f, 0.1f, 10.0f, "%.2f")) 
                                s_Edited = true;
        
                            ImGui::Text("Animation Clip: ");
                            ImGui::SameLine();
        
                            char buf[256];
                            buf[0] = '\0';
        
                            if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(state_clip->AnimClipHandle))
                            {
                                auto& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(state_clip->AnimClipHandle);
            
                                #if defined(L_PLATFORM_WINDOWS)
                                    strncpy_s(buf, meta_data.AssetName.c_str(), sizeof(buf));
                                #else
                                    strncpy(buf, meta_data.AssetName.c_str(), sizeof(buf));
                                #endif
                            }
                            else
                            {
                                #if defined(L_PLATFORM_WINDOWS)
                                    strncpy_s(buf, "None", sizeof(buf));
                                #else
                                    strncpy(buf, "None", sizeof(buf));
                                #endif
                            }
        
                            ImGui::InputText("##AnimClip_Handle", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
        
                            if (ImGui::BeginDragDropTarget()) 
                            {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
                                    AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;
        
                                    if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
                                        state_clip->AnimClipHandle = dropped_asset_handle;
                                        s_Edited = true;
                                    }
                                    else {
                                        L_APP_WARN("Invalid Asset Type Dropped on Animation Clip Target.");
                                    }
                                }
        
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
        
                                    std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
                                    std::filesystem::path dropped_asset_path = dropped_asset_path_string;
        
                                    if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {
        
                                        AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());
        
                                        if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
                                            state_clip->AnimClipHandle = dropped_asset_handle;
                                            s_Edited = true;
                                        }
                                        else {
                                            L_APP_WARN("Invalid Asset Type Dropped on Animation Clip Target.");
                                        }
                                    }
                                    else {
                                        L_APP_WARN("Invalid File Path Dropped on Animation Clip Target.");
                                    }
                                }
                                ImGui::EndDragDropTarget();
                            }
                            
                            ImGui::SameLine();
                            if (ImGui::SmallButton("x##AnimClipHandle"))
                            {
                                state_clip->AnimClipHandle = NULL_UUID;
                            }

                            if (auto layer = machine_asset->GetLayer(s_SelectedLayer); layer && layer->BlendType == StateMachine::Layer::LayerBlendType::Additive)
                            {
                                
                                ImGui::Text("Reference Clip: ");
                                ImGui::SameLine();
            
                                char ref_buf[256];
                                ref_buf[0] = '\0';
            
                                if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(state_clip->ReferenceClipHandle))
                                {
                                    auto& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(state_clip->ReferenceClipHandle);
                
                                    #if defined(L_PLATFORM_WINDOWS)
                                        strncpy_s(ref_buf, meta_data.AssetName.c_str(), sizeof(ref_buf));
                                    #else
                                        strncpy(ref_buf, meta_data.AssetName.c_str(), sizeof(ref_buf));
                                    #endif
                                }
                                else
                                {
                                    #if defined(L_PLATFORM_WINDOWS)
                                        strncpy_s(ref_buf, "None (Referencing Current Anim)", sizeof(ref_buf));
                                    #else
                                        strncpy(ref_buf, "None (Referencing Current Anim)", sizeof(ref_buf));
                                    #endif
                                }
            
                                ImGui::InputText("##RefAnimClip_Handle", ref_buf, sizeof(ref_buf), ImGuiInputTextFlags_ReadOnly);
            
                                if (ImGui::BeginDragDropTarget()) 
                                {
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
                                        AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;
            
                                        if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) 
                                        {
                                            state_clip->ReferenceClipHandle = dropped_asset_handle;
                                            s_Edited = true;
                                        }
                                        else {
                                            L_APP_WARN("Invalid Asset Type Dropped on Animation Clip Target.");
                                        }
                                    }
            
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
            
                                        std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
                                        std::filesystem::path dropped_asset_path = dropped_asset_path_string;
            
                                        if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {
            
                                            AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());
            
                                            if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
                                                state_clip->ReferenceClipHandle = dropped_asset_handle;
                                                s_Edited = true;
                                            }
                                            else {
                                                L_APP_WARN("Invalid Asset Type Dropped on Reference Animation Clip Target.");
                                            }
                                        }
                                        else {
                                            L_APP_WARN("Invalid File Path Dropped on Reference Animation Clip Target.");
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                                                                
                                ImGui::SameLine();
                                if (ImGui::SmallButton("x##ReferenceClipHandle"))
                                {
                                    state_clip->ReferenceClipHandle = NULL_UUID;
                                }

                                ImGui::Text("Frame Reference: ");
                                ImGui::SameLine();

                                uint32_t min_frame = 0;
                                uint32_t max_frame = 0;

                                if (AssetManager::IsAssetHandleValid(state_clip->ReferenceClipHandle))
                                {
                                    max_frame = AssetManager::GetAsset<AnimationClip>(state_clip->ReferenceClipHandle)->GetDuration();
                                }
                                else if (AssetManager::IsAssetHandleValid(state_clip->AnimClipHandle)) // If No Reference Pose, we refer to AnimClipHandle
                                {
                                    max_frame = AssetManager::GetAsset<AnimationClip>(state_clip->AnimClipHandle)->GetDuration();
                                }

                                // ensure valid memory pointers
                                if (max_frame > 0)
                                {
                                    ImGui::SliderScalar("##ReferenceFrameSlider", ImGuiDataType_U32, &state_clip->ReferencePoseFrame, &min_frame, &max_frame);
                                }
                                else
                                {
                                    if (state_clip->ReferencePoseFrame != 0)
                                        state_clip->ReferencePoseFrame = 0;

                                    max_frame = 100; // ensure some padding in dummy slider
                                    
                                    ImGui::BeginDisabled();
                                    
                                    ImGui::SliderScalar("##ReferenceFrameSlider", ImGuiDataType_U32, &min_frame, &min_frame, &max_frame);
                                    
                                    ImGui::EndDisabled();
                                }
                            }
                            
                            break;
                        }
                        case StateType::BlendTree:
                        {
                            ImGui::Text("State Type: Blend Tree");
                            
                            ImGui::Text("Current Normalised State Time: ");
                            ImGui::SameLine();
        
                            ImGui::BeginDisabled();
                            ImGui::SliderFloat("##CurrentTime", &state->NormalisedStateTime, 0.0f, 1.0f, "%.2f");
                            ImGui::EndDisabled();

                            break;
                        }
                    }
                }
        
                if (s_SelectedTransition.IsValid())
                {
                    auto transition = machine_asset->GetTransition(s_SelectedLayer, s_SelectedTransition.source_node.Get(), s_SelectedTransition.dest_node.Get());
                    if(transition)
                    {
                        auto source_state = machine_asset->GetAnimationState(s_SelectedLayer, transition->SourceStateHash);
                        auto dest_state = machine_asset->GetAnimationState(s_SelectedLayer, transition->DestStateHash);
    
                        const std::string& source_name = (transition->SourceStateHash == StateMachine::DefaultEntryHash) ? "Entry State" : (source_state) ? source_state->Name : "Null State";
                        const std::string& dest_name = (transition->DestStateHash == StateMachine::DefaultExitHash) ? "Exit State" : (dest_state) ? dest_state->Name : "Null State";
        
                        ImGui::Text("Source State: %s", source_name.c_str());
                        ImGui::Text("Dest State: %s", dest_name.c_str());
    
                        ImGui::Dummy({0.0f, 1.5f});
                        ImGui::Separator();
                        ImGui::Dummy({0.0f, 1.5f});
    
                        ImGui::Text("Transition Duration:");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##Transition Duration", &transition->TransitionDuration, 1.0f, 0.0f, FLT_MAX, "%.2f")) 
                            s_Edited = true;
    
                        ImGui::Text("Has Exit Time:");
                        ImGui::SameLine();
                        if (ImGui::Checkbox("##HasExitTime", &transition->HasExitTime)) 
                            s_Edited = true;
                        
                        ImGui::Text("Exit Time:");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##Exit Time", &transition->ExitTime, 0.001f, 0.0f, 1.0f, "%.2f")) 
                            s_Edited = true;
    
                        ImGui::Dummy({0.0f, 1.5f});
                        ImGui::Separator();
                        ImGui::Dummy({0.0f, 1.5f});
    
                        // Track index so we can erase by index
                        for (int i = 0; i < (int)transition->Conditions.size(); ++i)
                        {
                            auto& condition = transition->Conditions[i];
                            ImGui::PushID(&condition); // unique ID per condition
    
                            // --- Parameter combo ---
                            StringHash selected_param = condition.ParameterHash;
                            const char* selected_param_name = "";
                            int current_index = -1;
    
                            const auto& all_params = machine_asset->GetParameters();
                            std::vector<const char*> param_names;
                            param_names.reserve(all_params.size());
    
                            int idx = 0;
                            for (const auto& [param_hash, param] : all_params)
                            {
                                param_names.push_back(param.Name.c_str());
                                if (param_hash == selected_param)
                                {
                                    current_index = idx;
                                    selected_param_name = param.Name.c_str();
                                }
                                idx++;
                            }
    
                            ImGui::Text("Parameter:");
                            ImGui::SameLine();
                            if (ImGui::BeginCombo("##Parameter", selected_param_name))
                            {
                                for (int i = 0; i < (int)param_names.size(); ++i)
                                {
                                    bool is_selected = (i == current_index);
                                    if (ImGui::Selectable(param_names[i], is_selected))
                                    {
                                        int j = 0;
                                        for (const auto& [param_hash, param] : all_params)
                                        {
                                            if (j == i)
                                            {
                                                if (condition.ParameterHash != param_hash) // Only mark edited if it actually changed
                                                {
                                                    condition.ParameterHash = param_hash;
                                                    s_Edited = true;
                                                }
                                                break;
                                            }
                                            j++;
                                        }
                                    }
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
    
                            // --- Comparison ---
                            static const char* comp_names[] = {
                                "Equal", "NotEqual", "GreaterThan", "LessThan",
                                "GreaterThanOrEqual", "LessThanOrEqual"
                            };
                            int comp_index = static_cast<int>(condition.Operation);
                            ImGui::Text("Comparison:");
                            ImGui::SameLine();
                            if (ImGui::BeginCombo("##Comparison", comp_names[comp_index]))
                            {
                                for (int i = 0; i < IM_ARRAYSIZE(comp_names); ++i)
                                {
                                    bool selected = (comp_index == i);
                                    if (ImGui::Selectable(comp_names[i], selected))
                                    {
                                        if (condition.Operation != static_cast<ComparisonType>(i))
                                        {
                                            condition.Operation = static_cast<ComparisonType>(i);
                                            s_Edited = true;
                                        }
                                    }
                                    if (selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
    
                            // --- Value ---
                            ImGui::Text("Value:");
                            ImGui::SameLine();
    
                            if (auto param_ptr = machine_asset->GetParameter(condition.ParameterHash); param_ptr)
                            {
                                switch (param_ptr->Type)
                                {
                                    case ParameterType::Bool:
                                    {
                                        bool val = condition.ReferenceThreshold >= 0.5f;
                                        if (ImGui::Checkbox("##BoolVal", &val))
                                        {
                                            condition.ReferenceThreshold = val ? 1.0f : 0.0f;
                                            s_Edited = true;
                                        }
                                        break;
                                    }
                                    case ParameterType::Int:
                                    {
                                        int32_t val = static_cast<int32_t>(condition.ReferenceThreshold);
                                        if (ImGui::InputScalar("##IntVal", ImGuiDataType_S32, &val))
                                        {
                                            condition.ReferenceThreshold = static_cast<float>(val);
                                            s_Edited = true;
                                        }
                                        break;
                                    }
                                    case ParameterType::UInt:
                                    {
                                        uint32_t val = static_cast<uint32_t>(condition.ReferenceThreshold);
                                        if (ImGui::InputScalar("##UIntVal", ImGuiDataType_U32, &val))
                                        {
                                            condition.ReferenceThreshold = static_cast<float>(val);
                                            s_Edited = true;
                                        }
                                        break;
                                    }
                                    case ParameterType::Float:
                                    {
                                        float val = condition.ReferenceThreshold;
                                        if (ImGui::InputFloat("##FloatVal", &val))
                                        {
                                            condition.ReferenceThreshold = val;
                                            s_Edited = true;
                                        }
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                ImGui::Text("Invalid Parameter");
                            }
    
                            // --- Remove Button ---
                            ImGui::SameLine();
                            if (ImGui::Button("Remove"))
                            {
                                s_Edited = true;
                                transition->Conditions.erase(transition->Conditions.begin() + i);
                                ImGui::PopID(); // pop before breaking
                                break; // exit loop since vector is now modified
                            }
    
                            ImGui::Dummy({0.0f, 4.0f});
                            ImGui::Separator();
                            ImGui::Dummy({0.0f, 4.0f});
                            ImGui::PopID();
                        }
    
                        // --- Add Condition Button ---
                        if (ImGui::Button("+ Add Condition"))
                        {
                            s_Edited = true;
                            // Default new condition
                            transition->Conditions.push_back({
                                .ParameterHash = machine_asset->GetParameters().empty()
                                    ? NULL_UUID
                                    : machine_asset->GetParameters().begin()->first,
                                .Operation = ComparisonType::Equal,
                                .ReferenceThreshold = 0.0f
                            });
                        }
                    }
                }
            };

            auto draw_blend_node_inspector = [&](BlendNode& blend_node) -> void
            {
                static char blend_node_name_buf[256] = {};
                static auto old_level = s_BlendTreeChildLevel;

                if (static bool once = true; once)
                {
                    memset(blend_node_name_buf, 0, sizeof(blend_node_name_buf));
                    
                    const std::string& blend_tree_name = (s_BlendTreeChildLevel.empty()) ? machine_asset->GetAnimationState(s_SelectedLayer, s_StateBlendTreeNode)->Name : blend_node.Name;

                #if defined(L_PLATFORM_WINDOWS)
                    strncpy_s(blend_node_name_buf, blend_tree_name.c_str(), sizeof(blend_node_name_buf));
                #else
                    strncpy(blend_node_name_buf, blend_tree_name.c_str(), sizeof(blend_node_name_buf));
                #endif
                    
                    once = false;
                }

                if (old_level != s_BlendTreeChildLevel)
                {
                    const std::string& blend_tree_name = (s_BlendTreeChildLevel.empty()) ? machine_asset->GetAnimationState(s_SelectedLayer, s_StateBlendTreeNode)->Name : blend_node.Name;

                    #if defined(L_PLATFORM_WINDOWS)
                        strncpy_s(blend_node_name_buf, blend_tree_name.c_str(), sizeof(blend_node_name_buf));
                    #else
                        strncpy(blend_node_name_buf, blend_tree_name.c_str(), sizeof(blend_node_name_buf));
                    #endif
    
                    old_level = s_BlendTreeChildLevel;
                }

                ImGui::Text("Blend Tree Node Name:");
                ImGui::SameLine();
                if (ImGui::InputText("##BlendTreeNodeName", blend_node_name_buf, sizeof(blend_node_name_buf), ImGuiInputTextFlags_EnterReturnsTrue) || (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
                {
                    if (s_BlendTreeChildLevel.empty())
                    {
                        machine_asset->RenameState(s_SelectedLayer, s_StateBlendTreeNode, blend_node_name_buf);

                        if(s_SelectedNode == s_StateBlendTreeNode)
                            s_SelectedNode = Louron::Utils::fnv1a_hash(blend_node_name_buf);

                        s_StateBlendTreeNode = Louron::Utils::fnv1a_hash(blend_node_name_buf);

                        s_NodeMap.clear();
                        s_TransitionMap.clear();

                    }
                    blend_node.Name = blend_node_name_buf;
                }

                // Tree Type
                ImGui::Text("Blend Type:");
                ImGui::SameLine();
                const char* types[] = { "One Dimensional", "Two Dimensional Free Form" };
                int current_type = (int)blend_node.BlendType;
                if (ImGui::Combo("##BlendType", &current_type, types, IM_ARRAYSIZE(types)))
                    blend_node.BlendType = (TreeType)current_type;
            
                // Blend Params
                const auto& params = machine_asset->GetParameters();
                std::vector<const char*> float_param_names;
                std::vector<StringHash> float_param_hashes;
                for (const auto& [hash, param] : params)
                {
                    if (param.Type == ParameterType::Float)
                    {
                        float_param_names.push_back(param.Name.c_str());
                        float_param_hashes.push_back(hash);
                    }
                }

                if (blend_node.BlendType == TreeType::OneDimensional)
                {
                    ImGui::Text("Blend Parameter:");
                    ImGui::SameLine();
                    int current = -1;
                    for (size_t i = 0; i < float_param_hashes.size(); ++i)
                    {
                        if (blend_node.BlendParam[0] == float_param_hashes[i])
                        {
                            current = (int)i;
                            break;
                        }
                    }
            
                    if (ImGui::Combo("##BlendParam1D", &current, float_param_names.data(), (int)float_param_names.size()))
                        blend_node.BlendParam[0] = float_param_hashes[current];
                }
                else if (blend_node.BlendType == TreeType::TwoDimensionalFreeForm)
                {
                    ImGui::Text("Blend Param X:");
                    ImGui::SameLine();
                    int currentX = -1, currentY = -1;
                    for (size_t i = 0; i < float_param_hashes.size(); ++i)
                    {
                        if (blend_node.BlendParam[0] == float_param_hashes[i]) currentX = (int)i;
                        if (blend_node.BlendParam[1] == float_param_hashes[i]) currentY = (int)i;
                    }
            
                    if (ImGui::Combo("##BlendParamX", &currentX, float_param_names.data(), (int)float_param_names.size()))
                        blend_node.BlendParam[0] = float_param_hashes[currentX];
            
                    ImGui::Text("Blend Param Y:");
                    ImGui::SameLine();
                    if (ImGui::Combo("##BlendParamY", &currentY, float_param_names.data(), (int)float_param_names.size()))
                        blend_node.BlendParam[1] = float_param_hashes[currentY];
                }

                ImGui::Dummy({ 0.0f, 5.0f });
                ImGui::Separator();
                
                const char* table_id = "##BlendMotionsTable";
                int column_count = (blend_node.BlendType == TreeType::OneDimensional) ? 3 : 4;
                if (ImGui::BeginTable(table_id, column_count, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
                {
                    // Column sizing: give "Motion" more space, others minimal
                    ImGui::TableSetupColumn("Motion", ImGuiTableColumnFlags_WidthStretch, 1.5f);
                    if (blend_node.BlendType == TreeType::OneDimensional)
                    {
                        ImGui::TableSetupColumn("Threshold", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    }
                    else
                    {
                        ImGui::TableSetupColumn("Pos X", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableSetupColumn("Pos Y", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    }
                    ImGui::TableSetupColumn("Speed", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableHeadersRow();
                
                    int drag_src = -1, drag_dst = -1;
                
                    for (size_t i = 0; i < blend_node.ChildNode.size(); ++i)
                    {
                        auto& motion = blend_node.ChildNode[i];
                        if (!motion) continue;
                
                        ImGui::PushID(static_cast<int>(i));
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                
                        switch (motion->GetType())
                        {
                            case MotionType::Clip:
                            {
                                MotionAnimation* motion_animation = reinterpret_cast<MotionAnimation*>(motion.get());
                                std::string label = "None (Animation)";
                                if (AssetManager::IsAssetHandleValid(motion_animation->AnimClipHandle))
                                {
                                    label = Project::GetStaticEditorAssetManager()->GetMetadata(motion_animation->AnimClipHandle).AssetName;
                                }
                
                                char anim_name_buf[256];
                #if defined(L_PLATFORM_WINDOWS)
                                strncpy_s(anim_name_buf, label.c_str(), sizeof(anim_name_buf));
                #else
                                strncpy(anim_name_buf, label.c_str(), sizeof(anim_name_buf));
                #endif
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::InputText("##AnimClipName", anim_name_buf, sizeof(anim_name_buf), ImGuiInputTextFlags_ReadOnly);
                
                                if (ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
                                        AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;
                                        if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) &&
                                            Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
                                            motion_animation->AnimClipHandle = dropped_asset_handle;
                                            s_Edited = true;
                                        }
                                        else {
                                            L_APP_WARN("Invalid Asset Type Dropped on Animation Clip Target.");
                                        }
                                    }
                
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
                                        std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
                                        std::filesystem::path dropped_asset_path = dropped_asset_path_string;
                                        if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {
                                            AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(
                                                dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());
                
                                            if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
                                                motion_animation->AnimClipHandle = dropped_asset_handle;
                                                s_Edited = true;
                                            } else {
                                                L_APP_WARN("Invalid Asset Type Dropped on Animation Clip Target.");
                                            }
                                        } else {
                                            L_APP_WARN("Invalid File Path Dropped on Animation Clip Target.");
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                                break;
                            }
                            case MotionType::BlendTree:
                            {
                                MotionBlendTree* motion_blend = reinterpret_cast<MotionBlendTree*>(motion.get());
                                std::string label = motion_blend->RootNode.Name;
                                ImGui::TextUnformatted(label.c_str());
                                break;
                            }
                        }
                
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                        {
                            ImGui::SetDragDropPayload("BLEND_MOTION", &i, sizeof(size_t));
                            ImGui::Text("Reorder Motion %zu", i);
                            ImGui::EndDragDropSource();
                        }
                
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BLEND_MOTION"))
                            {
                                size_t payload_index = *(const size_t*)payload->Data;
                                if (payload_index != i)
                                {
                                    drag_src = (int)payload_index;
                                    drag_dst = (int)i;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                
                        if (blend_node.BlendType == TreeType::OneDimensional)
                        {
                            ImGui::TableSetColumnIndex(1);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragFloat(("##BlendX" + std::to_string(i)).c_str(), &motion->BlendPosition.x, 0.01f, 0.0f, 0.0f, "%.2f");
                        }
                        else
                        {
                            ImGui::TableSetColumnIndex(1);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragFloat(("##BlendX" + std::to_string(i)).c_str(), &motion->BlendPosition.x, 0.01f, 0.0f, 0.0f, "%.2f");
                            ImGui::TableSetColumnIndex(2);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragFloat(("##BlendY" + std::to_string(i)).c_str(), &motion->BlendPosition.y, 0.01f, 0.0f, 0.0f, "%.2f");
                        }
                
                        auto* anim = dynamic_cast<MotionAnimation*>(motion.get());
                        if (anim)
                        {                
                            ImGui::TableSetColumnIndex((blend_node.BlendType == TreeType::OneDimensional) ? 2 : 3);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::DragFloat(("##Speed" + std::to_string(i)).c_str(), &anim->PlaybackSpeed, 0.01f, 0.1f, 10.0f);
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                
                    if (drag_src != -1 && drag_dst != -1 && drag_src < (int)blend_node.ChildNode.size() && drag_dst < (int)blend_node.ChildNode.size())
                    {
                        auto motion = std::move(blend_node.ChildNode[drag_src]);
                        blend_node.ChildNode.erase(blend_node.ChildNode.begin() + drag_src);
                        blend_node.ChildNode.insert(blend_node.ChildNode.begin() + drag_dst, std::move(motion));
                    }
                }
                
                ImGui::Dummy({ 0.0f, 5.0f });
                if (ImGui::Button("+ Add Motion"))
                    ImGui::OpenPopup("AddMotionPopup");
                ImGui::SameLine();
                if (ImGui::Button("- Remove Last") && !blend_node.ChildNode.empty())
                    blend_node.ChildNode.pop_back();
                if (ImGui::BeginPopup("AddMotionPopup"))
                {
                    if (ImGui::MenuItem("Animation Clip"))
                        blend_node.AddMotion(MotionType::Clip);
                    if (ImGui::MenuItem("Blend Tree"))
                        blend_node.AddMotion(MotionType::BlendTree);
                    ImGui::EndPopup();
                }

                if (auto layer = machine_asset->GetLayer(s_SelectedLayer); layer && layer->BlendType == StateMachine::Layer::LayerBlendType::Additive)
                {
                    ImGui::Dummy({ 0.0f, 2.5f });
                    ImGui::Separator();
                    ImGui::Dummy({ 0.0f, 2.5f });

                    if (ImGui::TreeNode("Additive Reference Clips"))
                    {
                        for (int i = 0; i < static_cast<int>(blend_node.ChildNode.size()); ++i)
                        {
                            auto& motion_base = blend_node.ChildNode[i];

                            if (!motion_base)
                                continue;

                            if (motion_base->GetType() == MotionType::BlendTree)
                            {
                                MotionBlendTree* motion = reinterpret_cast<MotionBlendTree*>(motion_base.get());
                                
                                std::string tree_label = "Motion Index " + std::to_string(i) + ":";
                                if (ImGui::TreeNode(tree_label.c_str()))
                                {
                                    ImGui::Text("Open Child Blend Tree to Set Reference Clips In Animation Motions.");
                                    ImGui::TreePop();
                                }
                            }
                            else if (motion_base->GetType() == MotionType::Clip)
                            {
                                MotionAnimation* motion = reinterpret_cast<MotionAnimation*>(motion_base.get());

                                std::string tree_label = "Motion Index " + std::to_string(i) + ":";
                                if (ImGui::TreeNode(tree_label.c_str()))
                                {
                                    ImGui::Text("Reference Clip: ");
                                    ImGui::SameLine();
                
                                    char ref_buf[256];
                                    ref_buf[0] = '\0';
                
                                    if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(motion->ReferenceClipHandle))
                                    {
                                        auto& meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(motion->ReferenceClipHandle);
                    
                                        #if defined(L_PLATFORM_WINDOWS)
                                            strncpy_s(ref_buf, meta_data.AssetName.c_str(), sizeof(ref_buf));
                                        #else
                                            strncpy(ref_buf, meta_data.AssetName.c_str(), sizeof(ref_buf));
                                        #endif
                                    }
                                    else
                                    {
                                        #if defined(L_PLATFORM_WINDOWS)
                                            strncpy_s(ref_buf, "None (Referencing Current Anim)", sizeof(ref_buf));
                                        #else
                                            strncpy(ref_buf, "None (Referencing Current Anim)", sizeof(ref_buf));
                                        #endif
                                    }
                
                                    ImGui::InputText("##RefAnimClip_Handle", ref_buf, sizeof(ref_buf), ImGuiInputTextFlags_ReadOnly);
                
                                    if (ImGui::BeginDragDropTarget()) 
                                    {
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
                                            AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;
                
                                            if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) 
                                            {
                                                motion->ReferenceClipHandle = dropped_asset_handle;
                                                s_Edited = true;
                                            }
                                            else {
                                                L_APP_WARN("Invalid Asset Type Dropped on Animation Clip Target.");
                                            }
                                        }
                
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {
                
                                            std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
                                            std::filesystem::path dropped_asset_path = dropped_asset_path_string;
                
                                            if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {
                
                                                AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());
                
                                                if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
                                                    motion->ReferenceClipHandle = dropped_asset_handle;
                                                    s_Edited = true;
                                                }
                                                else {
                                                    L_APP_WARN("Invalid Asset Type Dropped on Reference Animation Clip Target.");
                                                }
                                            }
                                            else {
                                                L_APP_WARN("Invalid File Path Dropped on Reference Animation Clip Target.");
                                            }
                                        }
                                        ImGui::EndDragDropTarget();
                                    }
                                                                    
                                    ImGui::SameLine();
                                    if (ImGui::SmallButton("x##ReferenceClipHandle"))
                                    {
                                        s_Edited = true;
                                        motion->ReferenceClipHandle = NULL_UUID;
                                    }

                                    ImGui::Text("Frame Reference: ");
                                    ImGui::SameLine();

                                    uint32_t min_frame = 0;
                                    uint32_t max_frame = 0;

                                    if (AssetManager::IsAssetHandleValid(motion->ReferenceClipHandle))
                                    {
                                        max_frame = AssetManager::GetAsset<AnimationClip>(motion->ReferenceClipHandle)->GetDuration();
                                    }
                                    else if (AssetManager::IsAssetHandleValid(motion->AnimClipHandle)) // If No Reference Pose, we refer to AnimClipHandle
                                    {
                                        max_frame = AssetManager::GetAsset<AnimationClip>(motion->AnimClipHandle)->GetDuration();
                                    }

                                    // ensure valid memory pointers
                                    if (max_frame > 0)
                                    {
                                        if (ImGui::SliderScalar("##ReferenceFrameSlider", ImGuiDataType_U32, &motion->ReferencePoseFrame, &min_frame, &max_frame))
                                            s_Edited = true;
                                    }
                                    else
                                    {
                                        if (motion->ReferencePoseFrame != 0)
                                        {
                                            s_Edited = true;
                                            motion->ReferencePoseFrame = 0;
                                        }

                                        max_frame = 100; // ensure some padding in dummy slider
                                        
                                        ImGui::BeginDisabled();
                                        
                                        ImGui::SliderScalar("##ReferenceFrameSlider", ImGuiDataType_U32, &min_frame, &min_frame, &max_frame);
                                        
                                        ImGui::EndDisabled();
                                    }

                                    ImGui::TreePop();
                                }
                            }
                        }
                        
                        ImGui::TreePop();
                    }
                }
            };

            if (auto blend_state = machine_asset->GetAnimationState(s_SelectedLayer, s_StateBlendTreeNode); s_StateBlendTreeNode != NULL_UUID && blend_state && blend_state->GetType() == StateType::BlendTree)
            {
                BlendNode* node = &reinterpret_cast<AnimationState_BlendTree*>(blend_state)->AnimBlendTree;

                for (int i = 0; i < s_BlendTreeChildLevel.size(); i++)
                {
                    const size_t& index = s_BlendTreeChildLevel[i];
                    if (!node->ChildNode.empty() && index >= 0 && index < node->ChildNode.size())
                    {
                        if (node->ChildNode[index] && node->ChildNode[index]->GetType() == MotionType::BlendTree)
                        {
                            node = &reinterpret_cast<MotionBlendTree*>(node->ChildNode[index].get())->RootNode;
                            continue;
                        }
                        else
                        {
                            s_BlendTreeChildLevel.erase(s_BlendTreeChildLevel.begin() + i, s_BlendTreeChildLevel.end());
                            break;
                        }
                    }
                }

                draw_blend_node_inspector(*node);
            }
            else
            {
                draw_base_inspector();
            }

            ImGui::EndChild();
        
        #pragma endregion

            ImGui::EndTable();

        #pragma region Input Handling
        
            // Handle deletion of selected node or transition
            if (Engine::Get().GetInput().GetKeyDown(GLFW_KEY_DELETE) && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows))
            {
                if (s_SelectedNode != NULL_UUID)
                {
                    s_OldVersions.push_back(*machine_asset.get()); // Copy into old versions

                    machine_asset->RemoveState(s_SelectedLayer, s_SelectedNode);
                    s_SelectedNode = NULL_UUID;
    
                    // Also clear any transitions and UI data related to it if needed
                    s_SelectedTransition = {};
    
                    s_NodeMap.clear();
                    s_TransitionMap.clear();

                    s_Edited = true;
                }
                else if (s_SelectedTransition.IsValid())
                {
                    s_OldVersions.push_back(*machine_asset.get()); // Copy into old versions

                    StringHash src = s_SelectedTransition.source_node.Get();
                    StringHash dst = s_SelectedTransition.dest_node.Get();
    
                    machine_asset->RemoveTransition(s_SelectedLayer, src, dst);
                    s_SelectedTransition = {};
                    
                    s_NodeMap.clear();
                    s_TransitionMap.clear();

                    s_Edited = true;
                }
            }
    
            if (Engine::Get().GetInput().GetKey(GLFW_KEY_LEFT_CONTROL) && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows))
            {
                if (Engine::Get().GetInput().GetKeyDown(GLFW_KEY_S) && 
                    AssetManager::IsAssetHandleValid(s_AssetContext) && 
                    AssetManager::GetAssetType(s_AssetContext) == AssetType::AnimationStateMachine)
                {
                    auto meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(s_AssetContext);
                    YAML::Emitter out;
                    {
                        out << YAML::BeginMap;
        
                        machine_asset->Serialise(out);
        
                        out << YAML::EndMap;
                    }
        
                    std::ofstream fout(Project::GetActiveProject()->GetAssetDirectory() / meta_data.FilePath); // Create the file
                    fout << out.c_str(); // Save
                    fout.close();

                    s_Edited = false;
                }
            }

        #pragma endregion

        }        
    }
    ImGui::End();
    ax::NodeEditor::SetCurrentEditor(nullptr);
}

void AnimatorPanel::DrawBlendTree(Louron::Animation::BlendNode& blend_node)
{
    using namespace Louron::Animation;

    ed::Begin("BlendTreeViewer");
    {
        const ImVec2 node_size  = { 240.0f, 80.0f  };
        const ImVec2 spacing    = { 50.0f,  20.0f  };
        const ImVec2 root_pos   = { 100.0f, 200.0f };
        
        // Draw root blend node (represents the blend tree logic)
        const ed::NodeId root_id = ed::NodeId(1); // Static ID for root
        ed::BeginNode(root_id);
        ImGui::Dummy({ 1.0f, 1.0f });
        ed::EndNode();
        ed::SetNodePosition(root_id, root_pos + (node_size * 0.5f));

        if (ImDrawList* draw_list = ed::GetNodeBackgroundDrawList(root_id))
        {
            ImVec2 tl = root_pos;
            ImVec2 br = tl + node_size;
            draw_list->AddRectFilled(tl, br, IM_COL32(80.0f, 80.0f, 150.0f, 255.0f), 6.0f);
            draw_list->AddText(tl + ImVec2(10.0f, 10.0f), IM_COL32_WHITE, (blend_node.BlendType == TreeType::OneDimensional ? std::string(blend_node.Name + " - 1D Blend").c_str() : std::string(blend_node.Name + " - 2D Blend").c_str()));
        }

        // Draw all child motions
        for (size_t i = 0; i < blend_node.ChildNode.size(); ++i)
        {
            const auto& motion_ptr = blend_node.ChildNode[i];
            if (!motion_ptr) continue;
    
            ImVec2 child_pos = root_pos + ImVec2(node_size.x + spacing.x, i * (node_size.y + spacing.y));
            ed::NodeId motion_id = ed::NodeId(static_cast<int>(i + 10)); // Offset IDs for child nodes
            
            ed::BeginNode(motion_id);
            ImGui::Dummy({ 1.0f, 1.0f });
            ed::EndNode();
            ed::SetNodePosition(motion_id, child_pos + (node_size * 0.5f));
    
            if (ImDrawList* draw_list = ed::GetNodeBackgroundDrawList(motion_id))
            {
                ImVec2 tl = child_pos;
                ImVec2 br = tl + node_size;
    
                MotionType type = motion_ptr->GetType();
                std::string label;
                ImU32 color;
    
                if (type == MotionType::Clip)
                {
                    MotionAnimation* motion_anim_ptr = reinterpret_cast<MotionAnimation*>(motion_ptr.get());

                    if (AssetManager::IsAssetHandleValid(motion_anim_ptr->AnimClipHandle))
                    {
                        label = Project::GetStaticEditorAssetManager()->GetMetadata(motion_anim_ptr->AnimClipHandle).AssetName;

                        float width = glm::max<float>(node_size.x, ImGui::CalcTextSize(label.c_str()).x + 50.0f);
                        br = tl + ImVec2(width, node_size.y);
                    }
                    else
                    {
                        label = "None (Animation)";
                    }
                    color = IM_COL32(76.0f, 0.0f, 120.0f, 255.0f);
                }
                else if (type == MotionType::BlendTree)
                {
                    MotionBlendTree* motion_blend_ptr = reinterpret_cast<MotionBlendTree*>(motion_ptr.get());
                    label = motion_blend_ptr->RootNode.Name;
                    color = IM_COL32(0.0f, 95.0f, 143.0f, 255.0f);
                }
                else
                {
                    label = "Unknown";
                    color = IM_COL32(80.0f, 80.0f, 80.0f, 255.0f);
                }
    
                draw_list->AddRectFilled(tl, br, color, 6.0f);
                draw_list->AddText(tl + ImVec2(10.0f, 10.0f), IM_COL32_WHITE, label.c_str());
    
                // Show blend position for debugging
                std::string pos_text = "BlendPos: (" + std::to_string(motion_ptr->BlendPosition.x) + ", " +
                                       std::to_string(motion_ptr->BlendPosition.y) + ")";
                draw_list->AddText(tl + ImVec2(10.0f, 30.0f), IM_COL32(255.0f, 255.0f, 255.0f, 255.0f), pos_text.c_str());
            }
    
            // Draw bezier curve from root to child
            ImVec2 p1 = root_pos + ImVec2(node_size.x, node_size.y * 0.5f);
            ImVec2 p2 = child_pos + ImVec2(0.0f, node_size.y * 0.5f);
            ImVec2 c1 = p1 + ImVec2(40.0f, 0.0f);
            ImVec2 c2 = p2 - ImVec2(40.0f, 0.0f);
    
            ImGui::GetWindowDrawList()->AddBezierCubic(p1, c1, c2, p2, IM_COL32(255.0f, 255.0f, 255.0f, 200.0f), 2.0f);
    
            // Handle recursive navigation if double-clicked on blend tree motion
            ImVec2 mouse = ImGui::GetIO().MousePos;
            if (ImGui::IsMouseHoveringRect(child_pos, child_pos + node_size) &&
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
                motion_ptr->GetType() == MotionType::BlendTree)
            {
                s_BlendTreeChildLevel.push_back(i);
            }
        }

    }
    ed::End();
    
}

void AnimatorPanel::DrawGraph(Animation::StateMachine& machine)
{
    using namespace Louron::Animation;
    
    #pragma region Node Lazy Init

    if (s_NodeMap.empty())
    {
        for (auto& [hash,state] : machine.GetAllStates(s_SelectedLayer))
            s_NodeMap[hash] = Node{
                state->Name,
                hash,
                Louron::Utils::fnv1a_hash(std::to_string(hash) + "In"),
                Louron::Utils::fnv1a_hash(std::to_string(hash) + "Out"),
                hash == machine.GetDefaultStateHash(s_SelectedLayer),
                state->GetType()
            };
            
        s_NodeMap[StateMachine::DefaultEntryHash] = Node{
            "Entry State", 
            StateMachine::DefaultEntryHash, 
            NULL_UUID,
            Louron::Utils::fnv1a_hash(std::to_string(StateMachine::DefaultEntryHash) + "Out"),
            false, 
            StateType::Unknown
        };

        s_NodeMap[StateMachine::DefaultExitHash] = Node{
            "Exit State",
            StateMachine::DefaultExitHash, 
            Louron::Utils::fnv1a_hash(std::to_string(StateMachine::DefaultExitHash) + "In"),
            NULL_UUID,
            false, 
            StateType::Unknown
        };
    }

    if (auto all_transitions = machine.GetAllTransitions(s_SelectedLayer); all_transitions && s_TransitionMap.empty())
    {
        for (auto& t : *all_transitions)
        {
            if (!t) continue;
        
            auto src_hash = t->SourceStateHash;
            auto dst_hash = t->DestStateHash;
        
            if (!s_NodeMap.count(src_hash) || !s_NodeMap.count(dst_hash))
                continue;
        
            const auto& src_node = s_NodeMap[src_hash];
            const auto& dst_node = s_NodeMap[dst_hash];
        
            // Determine best available pins
            ed::PinId from_pin = src_node.output_pin;
            ed::PinId to_pin   = dst_node.input_pin;
        
            if (from_pin.Get() == NULL_UUID && src_node.input_pin.Get() != NULL_UUID)
                from_pin = src_node.input_pin; // fallback
        
            if (to_pin.Get() == NULL_UUID && dst_node.output_pin.Get() != NULL_UUID)
                to_pin = dst_node.output_pin; // fallback
        
            if (from_pin.Get() == NULL_UUID || to_pin.Get() == NULL_UUID)
                continue; // still unusable
        
            std::string id_string = std::to_string(src_node.node_id.Get()) + "->" + std::to_string(dst_node.node_id.Get());
            ImGuiID link_id = ImHashStr(id_string.c_str());
        
            s_TransitionMap.emplace_back(
                src_hash, dst_hash,
                from_pin,
                to_pin,
                link_id,
                true
            );
        }
    }
        
    #pragma endregion

    #pragma region Node Setup

    auto& style = ed::GetStyle();
    style.NodePadding     = ImVec4(0,0,0,0);
    style.NodeRounding    = 6.0f;
    style.NodeBorderWidth = 0.0f;
    style.LinkStrength    = 0.0f;    // straight links
    style.PinRadius       = 6.0f;
    style.PinBorderWidth  = 1.0f;

    // Link Creation
    static bool       creating_link     = false;
    static StringHash creating_link_source_node  = NULL_UUID;

    // Recording Centre of Each Node
    std::unordered_map<StringHash, ImVec2> centres;

    #pragma endregion

    #pragma region Node Rendering

    // 4) Begin editor
    ed::Begin("Animation State Machine");
    {
        const ImVec2 nodeSize = {200, 60};
        const ImVec2 pinSize  = {12, 12};

        #pragma region Node Render

        for (auto& [hash,node] : s_NodeMap)
        {
            ed::BeginNode(node.node_id);

            // Force the size of the node using dummy
            ImGui::Dummy(nodeSize);
            ImVec2 top_left = ImGui::GetItemRectMin();
            ImVec2 bottom_right = ImGui::GetItemRectMax();

            centres[hash] = (top_left + bottom_right) * 0.5f;

            // right‐click starts transition
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && hash != StateMachine::DefaultExitHash)
            {
                creating_link     = true;
                creating_link_source_node  = hash;
            }

            // left‐click on another node finishes transition
            if (creating_link
                && ImGui::IsItemHovered()
                && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
                && hash != creating_link_source_node)
            {
                auto& src = s_NodeMap[creating_link_source_node];

                // Avoid creating a duplicate transition
                if (machine.GetTransition(s_SelectedLayer, creating_link_source_node, hash) == nullptr && hash != StateMachine::DefaultEntryHash)
                {
                    std::string id_string = std::to_string(src.node_id.Get()) + "->" + std::to_string(s_NodeMap[hash].node_id.Get());
                    ImGuiID link_id = ImHashStr(id_string.c_str());
            
                    machine.CreateTransition(s_SelectedLayer, creating_link_source_node, hash);

                    s_Edited = true;
            
                    s_TransitionMap.emplace_back(
                        creating_link_source_node, hash,
                        src.output_pin,
                        s_NodeMap[hash].input_pin,
                        link_id,
                        true
                    );
                }

                creating_link = false;
            }
            
            const float pin_offset_x = 1.5f * pinSize.x;
            const float pin_offset_y = 1.5f * pinSize.y;

            // INPUT PIN
            {
                ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersAll);
                ed::PushStyleVar(ed::StyleVar_PinRadius, style.PinRadius);
                ed::PushStyleColor(ed::StyleColor_PinRect, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));         // No Highlight on Hover
                ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));   // No Highlight on Hover

                ed::BeginPin(node.input_pin, ed::PinKind::Input);
                {
                    ImVec2 pin_center = ImVec2(
                        (top_left.x + bottom_right.x) * 0.5f - pin_offset_x,    // Centered but offset to the left on X
                        (top_left.y + bottom_right.y) * 0.5f - pin_offset_y     // Centered by offset upward 
                    );
                    ImVec2 pin_half = pinSize * 0.5f;
                    ed::PinPivotRect(pin_center - pin_half, pin_center + pin_half);
                    ed::PinRect(pin_center - pin_half, pin_center + pin_half);
                }
                ed::EndPin();

                ed::PopStyleColor(2);
                ed::PopStyleVar(2);
            }

            // OUTPUT PIN
            {
                ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersAll);
                ed::PushStyleVar(ed::StyleVar_PinRadius, style.PinRadius);
                ed::PushStyleColor(ed::StyleColor_PinRect, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));         // No Highlight on Hover
                ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));   // No Highlight on Hover

                ed::BeginPin(node.output_pin, ed::PinKind::Output);
                ImVec2 pin_center = ImVec2(
                    (top_left.x + bottom_right.x) * 0.5f + pin_offset_x,    // Centered but offset to the right on X
                    (top_left.y + bottom_right.y) * 0.5f + pin_offset_y     // Centered by offset downward
                );
                ImVec2 pin_half = pinSize * 0.5f;
                ed::PinPivotRect(pin_center - pin_half, pin_center + pin_half);
                ed::PinRect(pin_center - pin_half, pin_center + pin_half);
                ed::EndPin();

                ed::PopStyleColor(2);
                ed::PopStyleVar(2);
            }

            ed::EndNode();

            // Manually Draw Node
            if (auto draw_list = ed::GetNodeBackgroundDrawList(node.node_id))
            {
                ImU32 fill = IM_COL32(80,80,80,255);
                if (node.default_node)              fill = IM_COL32(255,165, 0,255);
                if (hash == StateMachine::DefaultEntryHash) fill = IM_COL32( 50,200,80,255);
                if (hash == StateMachine::DefaultExitHash)  fill = IM_COL32(200, 50,50,255);
            
                draw_list->AddRectFilled(top_left, bottom_right, fill, style.NodeRounding);

                if (ImGui::IsMouseHoveringRect(top_left, bottom_right) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && machine.GetAnimationState(s_SelectedLayer, hash) && machine.GetAnimationState(s_SelectedLayer, hash)->GetType() == StateType::BlendTree)
                {
                    s_StateBlendTreeNode = hash;
                    s_BlendTreeChildLevel.clear();
                }   
            
                ImVec2 text_size = ImGui::CalcTextSize(node.node_name.c_str());
                ImVec2 text_pos = top_left + (nodeSize - text_size) * 0.5f;

                draw_list->AddText(text_pos, IM_COL32_WHITE, node.node_name.c_str());
            }
        }

        #pragma endregion

        #pragma region Create Links
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 mouse_pos = ImGui::GetIO().MousePos;

        // Check if Clicked Blank Space
        if (creating_link && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            bool hit_node = false;
            for (auto& [node_hash, centre] : centres)
            {
                ImRect r(centre - nodeSize * 0.5f, centre + nodeSize * 0.5f);
                if (r.Contains(mouse_pos)) 
                { 
                    hit_node = true; 
                    break; 
                }
            }
            if (!hit_node)
                creating_link = false;
        }

        // Draw In Flight Link Line
        if (creating_link && centres.count(creating_link_source_node))
        {
            draw_list->AddLine(centres[creating_link_source_node], mouse_pos, IM_COL32(255,255,255,200), 2.0f);
        }
        
        #pragma endregion

        #pragma region Draw Links
        
        // Map the Directions from Node to Node
        std::unordered_map<uint64_t, bool> direction_map; // True = forward uses visually input->input
        auto MakeLinkPairHash = [](ImGuiID a, ImGuiID b) -> uint64_t { return (uint64_t)std::min(a, b) << 32 | (uint64_t)std::max(a, b); };

        for (auto& link : s_TransitionMap)
        {
            ImGuiID source_id = link.source_node.Get();
            ImGuiID dest_id   = link.dest_node.Get();
            uint64_t pair_hash = MakeLinkPairHash(source_id, dest_id);
        
            bool forward = source_id < dest_id;
        
            // Determine and store preferred flow direction on first encounter
            if (!direction_map.count(pair_hash))
            {
                direction_map[pair_hash] = forward;
            }
        
            bool use_input_side = (direction_map[pair_hash] == forward);
        
            // Choose pins based on agreed direction
            ed::PinId from_pin = use_input_side
                ? s_NodeMap[source_id].input_pin
                : s_NodeMap[source_id].output_pin;
        
            ed::PinId to_pin = use_input_side
                ? s_NodeMap[dest_id].input_pin
                : s_NodeMap[dest_id].output_pin;

            bool from_pin_fallback = false;
            if (from_pin.Get() == NULL_UUID)
            {
                from_pin_fallback = true;
                from_pin = s_NodeMap[source_id].output_pin;
            }
            
            bool to_pin_fallback = false;
            if (to_pin.Get() == NULL_UUID)
            {
                to_pin_fallback = true;
                to_pin = s_NodeMap[dest_id].input_pin;
            }
        
            // Draw Link's from Pin to Pin
            ed::Link(link.link_id, from_pin, to_pin, ImVec4(1, 1, 1, 1), 2.0f);

            // Flow Highlight
            auto source_state = machine.GetAnimationState(s_SelectedLayer, (StringHash)link.source_node.Get());
            auto target_state = machine.GetAnimationState(s_SelectedLayer, (StringHash)link.dest_node.Get());
            if (source_state && target_state && 
                source_state == machine.GetCurrentAnimationState(s_SelectedLayer) &&
                target_state == machine.GetTargetAnimationState(s_SelectedLayer))
            {
                ed::Flow(link.link_id);
            }

            #pragma region Manual Arrow Direction Drawing

            float pin_offset_x = 1.5f * pinSize.x;
            float pin_offset_y = 1.5f * pinSize.y; // NEW

            // Determine actual pins used (after fallback)
            bool from_is_input = (from_pin == s_NodeMap[source_id].input_pin);
            bool to_is_input   = (to_pin   == s_NodeMap[dest_id].input_pin);

            // Compute start and end positions based on actual pins
            ImVec2 point_from = centres[source_id];
            ImVec2 point_to   = centres[dest_id];

            // Determine 
            if (from_is_input && to_is_input)
            {
                point_from.x -= pin_offset_x;
                point_from.y -= pin_offset_y;

                point_to.x   -= pin_offset_x;
                point_to.y   -= pin_offset_y;
            }
            else if (!from_is_input && !to_is_input)
            {
                point_from.x += pin_offset_x;
                point_from.y += pin_offset_y;

                point_to.x   += pin_offset_x;
                point_to.y   += pin_offset_y;
            }
            else if (!from_is_input && to_is_input)
            {
                point_from.x += pin_offset_x;
                point_from.y += pin_offset_y;

                point_to.x   -= pin_offset_x;
                point_to.y   -= pin_offset_y;
            }
            else if (from_is_input && !to_is_input)
            {
                point_from.x -= pin_offset_x;
                point_from.y -= pin_offset_y;

                point_to.x   += pin_offset_x;
                point_to.y   += pin_offset_y;
            }

            // Arrow direction and shape (from → to)
            ImVec2 direction = point_to - point_from;
            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
            if (length > 0.0f)
            {
                direction = ImVec2(direction.x / length, direction.y / length);
                ImVec2 perp = ImVec2(-direction.y, direction.x);

                ImVec2 mid = (point_from + point_to) * 0.5f;

                float arrow_size = 16.0f;
                ImVec2 tip   = mid;
                ImVec2 base  = tip - direction * arrow_size;
                ImVec2 left  = base + perp * (arrow_size * 0.5f);
                ImVec2 right = base - perp * (arrow_size * 0.5f);

                draw_list->AddTriangleFilled(tip, left, right, IM_COL32_WHITE);
            }

            #pragma endregion
            
        }
    
        #pragma endregion
    
        ed::Suspend();
        
        if (ed::ShowBackgroundContextMenu())
        {
            ImGui::OpenPopup("Create New State");
        }
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("Create New State"))
        {
            if (ImGui::MenuItem("Create New Animation Clip State"))
            {
                StringHash hash = machine.CreateState(s_SelectedLayer, "New Animation State", StateType::Clip);
                s_NodeMap[hash] = Node
                {
                    "New Animation State",
                    hash,
                    Louron::Utils::fnv1a_hash(std::to_string(hash) + "In"),
                    Louron::Utils::fnv1a_hash(std::to_string(hash) + "Out"),
                    hash == machine.GetDefaultStateHash(s_SelectedLayer),
                    StateType::Clip
                };
            }

            if (ImGui::MenuItem("Create New Blend Tree State"))
            {
                StringHash hash = machine.CreateState(s_SelectedLayer, "New Blend Tree State", StateType::BlendTree);
                s_NodeMap[hash] = Node
                {
                    "New Blend Tree State",
                    hash,
                    Louron::Utils::fnv1a_hash(std::to_string(hash) + "In"),
                    Louron::Utils::fnv1a_hash(std::to_string(hash) + "Out"),
                    hash == machine.GetDefaultStateHash(s_SelectedLayer),
                    StateType::BlendTree
                };
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        ed::Resume();
    }
    ed::End();



    #pragma endregion

    #pragma region Node/Link Selection
    
    ed::NodeId selected_nodes[16];
    int count = ed::GetSelectedNodes(selected_nodes, IM_ARRAYSIZE(selected_nodes));
    if (count == 1)
    {
        s_SelectedNode = (StringHash)selected_nodes[0].Get();
        s_SelectedTransition = {};
    }
    else 
    {
        s_SelectedNode = NULL_UUID;
    }
    
    ed::LinkId selected_links[16];
    count = ed::GetSelectedLinks(selected_links, IM_ARRAYSIZE(selected_links));
    if (count == 1)
    {
        ImGuiID selected_id = selected_links[0].Get();
        auto it = std::find_if(s_TransitionMap.begin(), s_TransitionMap.end(),
            [&](const TransitionLinks& link)
            {
                return link.link_id == selected_id;
            });
    
        if (it != s_TransitionMap.end())
        {
            s_SelectedTransition = *it;
            s_SelectedNode = NULL_UUID;
        }
    }
    else
    {
        s_SelectedTransition = {};
    }
    
    #pragma endregion
    
}