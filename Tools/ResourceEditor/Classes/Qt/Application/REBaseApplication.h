#pragma once

#include "Engine/Engine.h"

#ifdef __DAVAENGINE_BEAST__
class BeastProxyImpl;
#else
class BeastProxy;
#endif //__DAVAENGINE_BEAST__

class SettingsManager;
class SceneValidator;
class EditorConfig;

class REBaseApplication
{
public:
    REBaseApplication();
    ~REBaseApplication() = default; // on MacOS destructor will never be called. all clean up should be done in OnLoopStopped

    virtual void OnLoopStarted();
    virtual void OnLoopStopped();
    virtual void OnUpdate(DAVA::float32 delta);
    virtual void OnWindowCreated(DAVA::Window* w);

    int Run();

    DAVA::Signal<> beforeTerminate;

protected:
    virtual DAVA::KeyedArchive* GetEngineOptions();
    virtual DAVA::Vector<DAVA::String> GetEngineModules();
    virtual DAVA::eEngineRunMode GetEngineMode() const = 0;

    void Init();
    DAVA::Engine engine;

private:
#ifdef __DAVAENGINE_BEAST__
    BeastProxyImpl* beastProxy = nullptr;
#else
    BeastProxy* beastProxy = nullptr;
#endif //__DAVAENGINE_BEAST__
    EditorConfig* config = nullptr;
    SettingsManager* settingsManager = nullptr;
    SceneValidator* sceneValidator = nullptr;
};
