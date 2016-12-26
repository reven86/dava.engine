#pragma once

#if defined(__DAVAENGINE_MACOS__) 

#define PLUGIN_FUNCTION_EXPORT __attribute__((visibility("default")))

#elif defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_WIN_UAP__)

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

/**
 \brief handle plugin
 */
using PluginHandle = void*;

///

/**
 \brief plugin download function
 \param[in] pluginPath - path to plugin file
 \returns - returns handle descriptor to plugin
 */
PluginHandle OpenPlugin(const char* pluginPath);

/**
 \brief loading function of plugin
 \param[in] handle - handle plugin, funcName - function name
 \returns - returns pointer to function of plugin
 */
void* LoadFunction(PluginHandle handle, const char* funcName);

/**
 \brief close plugin function
 \param[in] handle - handle plugin
 */
void ClosePlugin(PluginHandle handle);

/**
 \brief loading function of plugin
 \param[in] handle - handle plugin, funcName - function name
 \returns - returns pointer to function of plugin
 */
template <class T>
T LoadFunction(PluginHandle handle, const char* funcName)
{
    return reinterpret_cast<T>(LoadFunction(handle, funcName));
}
