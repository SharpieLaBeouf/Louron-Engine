#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <iostream>

#ifdef _WIN32
#define SCRIPT_API extern "C" __declspec(dllexport)
#else
#define SCRIPT_API extern "C"
#endif

#pragma region EngineCallbackAPI

namespace EngineCallbackAPI
{
    inline std::unordered_map<std::string, void*> s_FunctionMap;

    inline void RegisterFunction(const char* name, void* fn)
    {
        s_FunctionMap[name] = fn;
    }

    template<typename T>
    inline T GetFunction(const char* name)
    {
        auto it = s_FunctionMap.find(name);
        assert(it != s_FunctionMap.end() && "Function not registered");
        return reinterpret_cast<T>(it->second);
    }
}

SCRIPT_API void RegisterHostFunction(const char* name, void* fn);

// Calls a registered engine function without safety check.
#define ENGINE_CALL_TYPED(fn_ptr_type, name, ...) \
    (reinterpret_cast<fn_ptr_type>(EngineCallbackAPI::s_FunctionMap[#name])(__VA_ARGS__))

// Safe call for functions returning a non-pointer type.
// Requires the return type and the full function pointer signature.
#define ENGINE_SAFE_CALL_RET(ret_type, fn_ptr_type, name, ...) \
    ([&]() -> ret_type { \
        if (EngineCallbackAPI::s_FunctionMap.count(#name)) \
            return reinterpret_cast<fn_ptr_type>(EngineCallbackAPI::s_FunctionMap[#name])(__VA_ARGS__); \
        std::cerr << "[SCRIPT DLL] ENGINE_SAFE_CALL_RET failed: Function \"" << #name << "\" is not registered in host process.\n"; \
        return ret_type{}; \
    })()

// Safe call for functions returning a non-pointer type with default fallback if no function registered.
// Requires the return type and the full function pointer signature.
#define ENGINE_SAFE_CALL_RET_FALLBACK(ret_type, fn_ptr_type, name, fallback_value, ...) \
    ([&]() -> ret_type { \
        if (EngineCallbackAPI::s_FunctionMap.count(#name)) \
            return reinterpret_cast<fn_ptr_type>(EngineCallbackAPI::s_FunctionMap[#name])(__VA_ARGS__); \
        std::cerr << "[SCRIPT DLL] ENGINE_SAFE_CALL_RET failed: Function \"" << #name << "\" is not registered in host process.\n"; \
        return fallback_value; \
    })()

// Safe call for functions returning a pointer type.
// Returns nullptr on failure instead of a default constructed value.
#define ENGINE_SAFE_CALL_RET_PTR(ret_type, fn_ptr_type, name, ...) \
    ([&]() -> ret_type { \
        if (EngineCallbackAPI::s_FunctionMap.count(#name)) \
            return reinterpret_cast<fn_ptr_type>(EngineCallbackAPI::s_FunctionMap[#name])(__VA_ARGS__); \
        std::cerr << "[SCRIPT DLL] ENGINE_SAFE_CALL_RET_PTR failed: Function \"" << #name << "\" is not registered in host process.\n"; \
        return nullptr; \
    })()

// Safe call for void-returning functions.
#define ENGINE_SAFE_CALL_VOID(fn_ptr_type, name, ...) \
    do { \
        if (EngineCallbackAPI::s_FunctionMap.count(#name)) \
            ENGINE_CALL_TYPED(fn_ptr_type, name, __VA_ARGS__); \
        else \
            std::cerr << "[SCRIPT DLL] ENGINE_SAFE_CALL_VOID failed: Function \"" << #name << "\" is not registered in host process.\n"; \
    } while(0)

#pragma endregion
