#include "ScriptAPI.h"

#include <vector>

// --- Internal Representation ---
namespace BackEndAPI
{

    struct InternalScriptType {
        const char* name;
        std::vector<BackEndAPI::FieldInfo> fields;
        void* (*factory)(uint32_t);
        void(*release)(uint32_t);
    };

    static std::vector<InternalScriptType> s_InternalTypes;
    static std::vector<BackEndAPI::ScriptTypeInfo> s_ExportedTypes;
    static std::vector<const char*> s_NameStorage;
}

// These are internal to the DLL, not exported externally
void RegisterScript(const char* name, std::vector<BackEndAPI::FieldInfo> fields, void*(*factory)(uint32_t), void(*release)(uint32_t))
{
    BackEndAPI::InternalScriptType internal;
    internal.name = name;
    internal.fields = std::move(fields);
    internal.factory = factory;
    internal.release = release;
    BackEndAPI::s_InternalTypes.emplace_back(std::move(internal));
}

void FinalizeScriptRegistry() 
{
    BackEndAPI::s_ExportedTypes.clear();
    BackEndAPI::s_NameStorage.clear();

    for (const auto& internal : BackEndAPI::s_InternalTypes)
    {
        BackEndAPI::s_NameStorage.push_back(internal.name);

        BackEndAPI::ScriptTypeInfo info;
        info.name = BackEndAPI::s_NameStorage.back();
        info.field_count = static_cast<int>(internal.fields.size());
        info.fields = internal.fields.data();
        info.factory_function = internal.factory;
        info.release_function = internal.release;
        BackEndAPI::s_ExportedTypes.push_back(info);
    }
}

SCRIPT_API const BackEndAPI::ScriptTypeInfo* GetScriptTypes(size_t* count)
{
    if (!count)
        return nullptr;

    *count = BackEndAPI::s_ExportedTypes.size();
    return BackEndAPI::s_ExportedTypes.data();
}

SCRIPT_API const char** GetScriptNames(size_t* count)
{
    if (!count)
        return nullptr;

    *count = BackEndAPI::s_NameStorage.size();
    return BackEndAPI::s_NameStorage.data();
}