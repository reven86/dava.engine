
///
#include "Base/Platform.h"
#include "PluginManager/Plugin.h"

#if defined(__DAVAENGINE_MACOS__) 

#include <dlfcn.h>

#endif

///

#if defined(__DAVAENGINE_MACOS__)

PluginHandle OpenPlugin(const char* pluginPath)
{
    return dlopen(pluginPath, RTLD_NOW);
}

void* LoadFunction(PluginHandle handle, const char* funcName)
{
    return dlsym(handle, funcName);
}

void ClosePlugin(PluginHandle handle)
{
    dlclose(handle);
}

#elif defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_WIN_UAP__)

PluginHandle OpenPlugin(const char* pluginPath)
{
    wchar_t pluginWcharPath[MAX_PATH];
    ::MultiByteToWideChar(CP_ACP, 0, (const char*)(pluginPath), -1, pluginWcharPath, sizeof(pluginWcharPath) / sizeof(pluginWcharPath[0]));
    return LoadLibraryW(pluginWcharPath);
}

void* LoadFunction(PluginHandle handle, const char* funcName)
{
    return GetProcAddress(static_cast<HMODULE>(handle), funcName);
}

void ClosePlugin(PluginHandle handle)
{
    FreeLibrary(static_cast<HMODULE>(handle));
}

#else

PluginHandle OpenPlugin(const char* pluginPath)
{
    return nullptr;
}

void* LoadFunction(PluginHandle handle, const char* funcName)
{
    return nullptr;
}

void ClosePlugin(PluginHandle handle)
{
}
 
#endif
