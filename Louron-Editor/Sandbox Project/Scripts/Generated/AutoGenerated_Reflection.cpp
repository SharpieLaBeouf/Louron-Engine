#include "../Script Core API/ScriptAPI.h"
#include "../Character Controller.h"
#include "../Knerpix\Collission Push.h"
#include "../Knerpix\GameManager.h"
#include "../NativeTesting\NativeAPIUnitTests.h"
#include "../NativeTesting\NativeScriptingTypesTest.h"
using namespace BackEndAPI;

extern void RegisterScript(const char* name, std::vector<FieldInfo> fields, void* (*factory)(uint32_t), void(*release)(uint32_t));
extern void FinalizeScriptRegistry();

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
void* Create_NativeTestScript(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "NativeTestScript"] = std::make_unique<NativeTestScript>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "NativeTestScript"].get();
}
void Release_NativeTestScript(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "NativeTestScript"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}
void* Create_NativeScriptingTypesTest(uint32_t entity_uuid) {
    s_ScriptInstanceMap[std::to_string(entity_uuid) + "NativeScriptingTypesTest"] = std::make_unique<NativeScriptingTypesTest>();
    return s_ScriptInstanceMap[std::to_string(entity_uuid) + "NativeScriptingTypesTest"].get();
}
void Release_NativeScriptingTypesTest(uint32_t entity_uuid) {
    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + "NativeScriptingTypesTest"); found_instance != s_ScriptInstanceMap.end())
        s_ScriptInstanceMap.erase(found_instance);
}

SCRIPT_API void LoadScripts() {
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
    {
        std::vector<FieldInfo> fields;
        fields.push_back({ "shader", offsetof(NativeTestScript, shader), FieldType::ComputeShader });
        fields.push_back({ "bc", offsetof(NativeTestScript, bc), FieldType::BoxColliderComponent });
        RegisterScript("NativeTestScript", std::move(fields), &Create_NativeTestScript, &Release_NativeTestScript);
    }
    {
        std::vector<FieldInfo> fields;
        fields.push_back({ "my_float", offsetof(NativeScriptingTypesTest, my_float), FieldType::Float });
        fields.push_back({ "my_double", offsetof(NativeScriptingTypesTest, my_double), FieldType::Double });
        fields.push_back({ "my_int8", offsetof(NativeScriptingTypesTest, my_int8), FieldType::Int8 });
        fields.push_back({ "my_int16", offsetof(NativeScriptingTypesTest, my_int16), FieldType::Int16 });
        fields.push_back({ "my_int32", offsetof(NativeScriptingTypesTest, my_int32), FieldType::Int32 });
        fields.push_back({ "my_int64", offsetof(NativeScriptingTypesTest, my_int64), FieldType::Int64 });
        fields.push_back({ "my_uint8", offsetof(NativeScriptingTypesTest, my_uint8), FieldType::UInt8 });
        fields.push_back({ "my_uint16", offsetof(NativeScriptingTypesTest, my_uint16), FieldType::UInt16 });
        fields.push_back({ "my_uint32", offsetof(NativeScriptingTypesTest, my_uint32), FieldType::UInt32 });
        fields.push_back({ "my_uint64", offsetof(NativeScriptingTypesTest, my_uint64), FieldType::UInt64 });
        fields.push_back({ "my_bool", offsetof(NativeScriptingTypesTest, my_bool), FieldType::Bool });
        fields.push_back({ "my_cstring", offsetof(NativeScriptingTypesTest, my_cstring), FieldType::CString });
        fields.push_back({ "my_vector2", offsetof(NativeScriptingTypesTest, my_vector2), FieldType::Vector2 });
        fields.push_back({ "my_vector3", offsetof(NativeScriptingTypesTest, my_vector3), FieldType::Vector3 });
        fields.push_back({ "my_vector4", offsetof(NativeScriptingTypesTest, my_vector4), FieldType::Vector4 });
        fields.push_back({ "my_uvector2", offsetof(NativeScriptingTypesTest, my_uvector2), FieldType::UVector2 });
        fields.push_back({ "my_uvector3", offsetof(NativeScriptingTypesTest, my_uvector3), FieldType::UVector3 });
        fields.push_back({ "my_uvector4", offsetof(NativeScriptingTypesTest, my_uvector4), FieldType::UVector4 });
        fields.push_back({ "my_ivector2", offsetof(NativeScriptingTypesTest, my_ivector2), FieldType::IVector2 });
        fields.push_back({ "my_ivector3", offsetof(NativeScriptingTypesTest, my_ivector3), FieldType::IVector3 });
        fields.push_back({ "my_ivector4", offsetof(NativeScriptingTypesTest, my_ivector4), FieldType::IVector4 });
        fields.push_back({ "my_dvector2", offsetof(NativeScriptingTypesTest, my_dvector2), FieldType::DVector2 });
        fields.push_back({ "my_dvector3", offsetof(NativeScriptingTypesTest, my_dvector3), FieldType::DVector3 });
        fields.push_back({ "my_dvector4", offsetof(NativeScriptingTypesTest, my_dvector4), FieldType::Unknown });
        fields.push_back({ "my_bvector2", offsetof(NativeScriptingTypesTest, my_bvector2), FieldType::BVector2 });
        fields.push_back({ "my_bvector3", offsetof(NativeScriptingTypesTest, my_bvector3), FieldType::BVector3 });
        fields.push_back({ "my_bvector4", offsetof(NativeScriptingTypesTest, my_bvector4), FieldType::BVector4 });
        RegisterScript("NativeScriptingTypesTest", std::move(fields), &Create_NativeScriptingTypesTest, &Release_NativeScriptingTypesTest);
    }
    FinalizeScriptRegistry();
}
