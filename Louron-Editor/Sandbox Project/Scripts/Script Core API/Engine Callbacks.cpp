#include "Engine Callbacks.h"

SCRIPT_API void RegisterHostFunction(const char* name, void* fn)
{
    EngineCallbackAPI::RegisterFunction(name, fn);
}
