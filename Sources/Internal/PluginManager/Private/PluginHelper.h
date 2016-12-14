#pragma once

///

#if defined(__DAVAENGINE_MACOS__) 

#include <dlfcn.h>

#elif defined(__DAVAENGINE_WIN32__)

#include <Windows.h>

#endif

///

#if defined(__DAVAENGINE_MACOS__) 

typedef void* PluginHandle;
#define NULL_PLUGIN_HANDLE nullptr

#elif defined(__DAVAENGINE_WIN32__)

typedef HINSTANCE PluginHandle;
#define NULL_PLUGIN_HANDLE NULL

#else 

typedef HINSTANCE PluginHandle;
#define NULL_PLUGIN_HANDLE nullptr

#endif

///

#if defined(__DAVAENGINE_MACOS__) 

PluginHandle OpenPlugin( const char* pluginPath )
{
    return dlopen(pluginPath, RTLD_NOW);
}

template<class T>
T LoadFunction( PluginHandle handle, const char* funcName )
{
    return reinterpret_cast<T>( dlsym( handle, funcName ) );
}

void ClosePlugin( PluginHandle handle )
{
    dlclose( handle );
}

#elif defined(__DAVAENGINE_WIN32__)

PluginHandle OpenPlugin( const char* pluginPath )
{
    return LoadLibraryA( pluginPath );
}

template<class T>
T LoadFunction( PluginHandle handle, const char* funcName )
{
    return reinterpret_cast<T>(GetProcAddress(handle, funcName));
}

void ClosePlugin( PluginHandle handle )
{
    FreeLibrary( handle );
}

#else 

PluginHandle OpenPlugin(const char* pluginPath)
{
    return NULL_PLUGIN_HANDLE;
}

template<class T>
T LoadFunction(PluginHandle handle, const char* funcName)
{
    return nullptr;
}

void ClosePlugin(PluginHandle handle)
{
    dlclose( handle );
}
 
#endif





