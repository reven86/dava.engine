#pragma once

#if defined(__DAVAENGINE_MACOS__) 

#define PLUGIN_FUNCTION_EXPORT __attribute__((visibility("default")))

#elif defined(__DAVAENGINE_WIN32__) 

#define PLUGIN_FUNCTION_EXPORT __declspec(dllexport)

#else

#define PLUGIN_FUNCTION_EXPORT 

#endif

//

#define EXPORT_PLUGIN(PLUGIN) \
extern "C" { \
    PLUGIN_FUNCTION_EXPORT \
    DAVA::IModule* CreatePlugin(DAVA::Engine* engine)\
    {\
        return new PLUGIN(engine);\
    }\
    PLUGIN_FUNCTION_EXPORT\
    void DestroyPlugin(DAVA::IModule* plugin)\
    {\
        delete plugin;\
    }\
}

using PluginHandle = void*;

///

PluginHandle OpenPlugin(const char* pluginPath);

void* LoadFunction(PluginHandle handle, const char* funcName);

void ClosePlugin(PluginHandle handle);

template <class T>
T LoadFunction(PluginHandle handle, const char* funcName)
{
    return reinterpret_cast<T>(LoadFunction(handle, funcName));
}
