#pragma once

///
#include "Base/Platform.h"

#if defined(__DAVAENGINE_MACOS__) 

#include <dlfcn.h>

#endif

///

using PluginHandle = void*;

///

#if defined(__DAVAENGINE_MACOS__)

PluginHandle OpenPlugin(const char* pluginPath)
{
    return dlopen(pluginPath, RTLD_NOW);
}

template <class T>
T LoadFunction(PluginHandle handle, const char* funcName)
{
    return reinterpret_cast<T>(dlsym(handle, funcName));
}

void ClosePlugin(PluginHandle handle)
{
    dlclose(handle);
}

#elif defined(__DAVAENGINE_WIN32__)

PluginHandle OpenPlugin(const char* pluginPath)
{
    return LoadLibraryA(pluginPath);
}

void* LoadFunction(PluginHandle handle, const char* funcName)
{
    return GetProcAddress(static_cast<HMODULE>(handle), funcName);
}

template <class T>
T LoadFunction(PluginHandle handle, const char* funcName)
{
    return reinterpret_cast<T>( LoadFunction(handle, funcName) );
}

void ClosePlugin(PluginHandle handle)
{
    FreeLibrary( static_cast<HMODULE>(handle) );
}

#else

PluginHandle OpenPlugin(const char* pluginPath)
{
    return nullptr;
}

template <class T>
T LoadFunction(PluginHandle handle, const char* funcName)
{
    return nullptr;
}

void ClosePlugin(PluginHandle handle)
{
    dlclose(handle);
}
 
#endif
