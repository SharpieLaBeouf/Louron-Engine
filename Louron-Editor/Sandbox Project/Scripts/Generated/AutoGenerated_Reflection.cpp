#include "../Script Core API/ScriptAPI.h"
#include "../Animator Controller.h"
#include "../Bullet Script.h"
#include "../Character Controller.h"
#include "../Knerpix\Collission Push.h"
#include "../Knerpix\GameManager.h"
using namespace BackEndAPI;

extern void RegisterScript(const char* name, std::vector<FieldInfo> fields, void* (*factory)(uint32_t), void(*release)(uint32_t));
extern void FinalizeScriptRegistry();

void* Create_AnimatorController(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "AnimatorController"] = std::make_unique<AnimatorController>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "AnimatorController"].get();
}
void Release_AnimatorController(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "AnimatorController"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}
void* Create_BulletScript(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "BulletScript"] = std::make_unique<BulletScript>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "BulletScript"].get();
}
void Release_BulletScript(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "BulletScript"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}
void* Create_CharacterController(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "CharacterController"] = std::make_unique<CharacterController>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "CharacterController"].get();
}
void Release_CharacterController(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "CharacterController"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}
void* Create_CollissionPush(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "CollissionPush"] = std::make_unique<CollissionPush>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "CollissionPush"].get();
}
void Release_CollissionPush(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "CollissionPush"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}
void* Create_GameManager(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "GameManager"] = std::make_unique<GameManager>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "GameManager"].get();
}
void Release_GameManager(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "GameManager"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}

SCRIPT_API void LoadScripts() {
    {
        std::vector<FieldInfo> fields;
        fields.push_back({ "animator", offsetof(AnimatorController, animator), FieldType::AnimatorComponent });
        fields.push_back({ "shoot_entity", offsetof(AnimatorController, shoot_entity), FieldType::Entity });
        fields.push_back({ "bullet_prefab", offsetof(AnimatorController, bullet_prefab), FieldType::Prefab });
        RegisterScript("AnimatorController", std::move(fields), &Create_AnimatorController, &Release_AnimatorController);
    }
    {
        std::vector<FieldInfo> fields;
        fields.push_back({ "timer", offsetof(BulletScript, timer), FieldType::Float });
        RegisterScript("BulletScript", std::move(fields), &Create_BulletScript, &Release_BulletScript);
    }
    {
        std::vector<FieldInfo> fields;
        fields.push_back({ "speed", offsetof(CharacterController, speed), FieldType::Float });
        fields.push_back({ "rotation_speed", offsetof(CharacterController, rotation_speed), FieldType::Float });
        fields.push_back({ "rotation_smoothing", offsetof(CharacterController, rotation_smoothing), FieldType::Float });
        RegisterScript("CharacterController", std::move(fields), &Create_CharacterController, &Release_CharacterController);
    }
    {
        std::vector<FieldInfo> fields;
        RegisterScript("CollissionPush", std::move(fields), &Create_CollissionPush, &Release_CollissionPush);
    }
    {
        std::vector<FieldInfo> fields;
        fields.push_back({ "Player_1_Prefab", offsetof(GameManager, Player_1_Prefab), FieldType::Prefab });
        fields.push_back({ "Player_2_Prefab", offsetof(GameManager, Player_2_Prefab), FieldType::Prefab });
        fields.push_back({ "Players_Parent_Entity", offsetof(GameManager, Players_Parent_Entity), FieldType::Entity });
        fields.push_back({ "Player_1_Spawn_Point", offsetof(GameManager, Player_1_Spawn_Point), FieldType::Entity });
        fields.push_back({ "Player_2_Spawn_Point", offsetof(GameManager, Player_2_Spawn_Point), FieldType::Entity });
        fields.push_back({ "Winner_Location", offsetof(GameManager, Winner_Location), FieldType::Entity });
        fields.push_back({ "Speed_Multiplier", offsetof(GameManager, Speed_Multiplier), FieldType::Float });
        fields.push_back({ "DeathFloorY", offsetof(GameManager, DeathFloorY), FieldType::Float });
        RegisterScript("GameManager", std::move(fields), &Create_GameManager, &Release_GameManager);
    }
    FinalizeScriptRegistry();
}
