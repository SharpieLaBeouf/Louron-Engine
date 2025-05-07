#include "../Script Core API/ScriptAPI.h"
using namespace BackEndAPI;

extern void RegisterScript(const char* name, std::vector<FieldInfo> fields, void* (*factory)(uint32_t), void(*release)(uint32_t));
extern void FinalizeScriptRegistry();


SCRIPT_API void LoadScripts() {
    FinalizeScriptRegistry();
}
