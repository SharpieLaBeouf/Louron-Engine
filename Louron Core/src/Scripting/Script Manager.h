#pragma once

// Louron Core Headers
#include "../Core/Platform.h"
#include "Script Defines.h"

// C++ Standard Library Headers
#include <memory>
#include <string>
#include <format>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#ifdef L_PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

// External Vendor Library Headers


namespace Louron
{

    class Scene;

    class ScriptManager
    {

    public:

        using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

        static ScriptManager* Get();

        static bool Init(const std::filesystem::path& path);
        static void Shutdown();

        void RemoveAllScriptInstances();
        void RemoveAllScriptFields();

        void FreeAssembly();
        bool ReloadAssembly(const std::filesystem::path& path);

        // --------------------------------------
        //                Getters
        // --------------------------------------
        const auto& GetAllClasses() const { return m_ScriptClasses; }
        std::shared_ptr<ScriptClass> GetScriptClass(const std::string& name);

        ScriptClassInstance* GetScriptClassInstance(uint32_t entity_uuid, const std::string& script_name);
        ScriptFieldInstance* GetScriptFieldInstance(uint32_t entity_uuid, const std::string& script_name, const std::string& field_name);
        ScriptFieldMap* GetScriptFieldMap(uint32_t entity_uuid, const std::string& script_name);

        void AddDummyScriptClass(const std::string& script_name);

        bool ScriptClassExists(const std::string& script_name);

        // --------------------------------------
        //      Script Interface Functions
        // --------------------------------------
        void OnCreateScript(uint32_t entity_uuid, const std::string& script_name);
        void OnDestroyScript(uint32_t entity_uuid, const std::string& script_name);
        void OnDestroyAllScripts(uint32_t entity_uuid);

        void OnUpdateScript(uint32_t entity_uuid, const std::string& script_name);
        void OnLateUpdateScript(uint32_t entity_uuid, const std::string& script_name);
        void OnFixedUpdateScript(uint32_t entity_uuid, const std::string& script_name);

        void OnCollideScript(uint32_t entity_uuid, uint32_t other_uuid, const std::string& script_name, Script_Collision_Type collision_type);

    private:

        bool LoadAssembly(const std::filesystem::path& path);

        bool CreateInstance(uint32_t entity_uuid, const std::string& script_name);

        // KEY = script_name, VALUE = script class
        std::unordered_map<std::string, std::shared_ptr<ScriptClass>> m_ScriptClasses;
        // KEY = uuid + script_name, VALUE = script class instance
        std::unordered_map<std::string, std::unique_ptr<ScriptClassInstance>> m_ScriptInstances;
        // KEY = uuid + script_name, VALUE = script field map
        std::unordered_map<std::string, ScriptFieldMap> m_ScriptFieldInstances;

        HMODULE m_DLL = nullptr;
        void (*m_LoadScripts)() = nullptr;
        const ScriptClass* (*m_GetScriptTypes)(size_t*) = nullptr;

        Scene* scene_ref = nullptr;

        static ScriptManager* s_Instance;
    };

}