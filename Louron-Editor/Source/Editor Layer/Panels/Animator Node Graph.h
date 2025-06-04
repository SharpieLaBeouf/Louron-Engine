#pragma once

#include "Louron.h"

#include <imgui_node_editor.h>

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 operator*(const ImVec2& a, float s)         { return ImVec2(a.x * s, a.y * s); }
inline ImVec2 operator/(const ImVec2& a, float s)         { return ImVec2(a.x / s, a.y / s); }

namespace ed = ax::NodeEditor;
struct AnimatorPanel 
{

private:

    struct Node
    {
        std::string node_name;

        ed::NodeId node_id;
        ed::PinId input_pin;
        ed::PinId output_pin;
        
        bool default_node;
        Louron::Animation::StateType node_type;
    };
    
    struct TransitionLinks
    {
        TransitionLinks() : source_node(-1), dest_node(-1), source_pin(-1), dest_pin(-1), link_id(0), valid(false) {}
        TransitionLinks(ed::NodeId source_n, ed::NodeId dest_n, ed::PinId source_p, ed::PinId dest_p, ImGuiID _link_id, bool _valid) : source_node(source_n), dest_node(dest_n), source_pin(source_p), dest_pin(dest_p), link_id(_link_id), valid(_valid) {}

        ed::NodeId source_node;
        ed::NodeId dest_node;

        ed::PinId source_pin;
        ed::PinId dest_pin;

        ImGuiID link_id; // Stable ID

        bool valid;

        bool IsValid() const { return valid; }
    };

    static inline std::vector<Louron::Animation::StateMachine> s_OldVersions = {};

    static inline Louron::AssetHandle s_AssetContext = NULL_UUID;
    static inline std::unordered_map<Louron::Animation::StringHash, Node> s_NodeMap = {};
    static inline std::vector<TransitionLinks> s_TransitionMap = {};

    static inline size_t s_SelectedLayer = 0;
    static inline TransitionLinks s_SelectedTransition = {};
    static inline Louron::Animation::StringHash s_SelectedNode = NULL_UUID;

    // These two are used to determine a. what blend tree state in the state
    // machine are we traversing into, and b. what child of child blend trees
    // are we traversing intO
    static inline Louron::Animation::StringHash s_StateBlendTreeNode = NULL_UUID;
    
    // This holds the index of the motion vector in which the child blend tree
    // we're referring to resides. This will never be an issue because we can
    // only ever change the index of the node we may be in whilst we are viewing
    // its parent, so there will never be an issue of using an invalid index
    //
    // Push back when going down the tree, and pop back when going back up the tree
    static inline std::vector<size_t> s_BlendTreeChildLevel = {};

public:

    static void SetAnimatorAssetContext(Louron::AssetHandle animator_asset) { s_NodeMap.clear(); s_TransitionMap.clear(); s_AssetContext = animator_asset; }
    
    static void Draw(bool& show_window);

    static void DrawBlendTree(Louron::Animation::BlendNode& blend_node);
    
    static void DrawGraph(Louron::Animation::StateMachine& machine);

};