#pragma once

#include <TArc/Core/BaseApplication.h>
#include <FileSystem/KeyedArchive.h>
#include <Base/RefPtr.h>

#include <QString>

namespace DAVA
{
namespace TArc
{
class Core;
}
}

#ifdef __DAVAENGINE_BEAST__
#define BEAST_PROXY_TYPE BeastProxyImpl
#else
#define BEAST_PROXY_TYPE BeastProxy
#endif

class EditorConfig;
class SettingsManager;
class SceneValidator;
class BEAST_PROXY_TYPE;

class REApplication : public DAVA::TArc::BaseApplication
{
public:
    REApplication(DAVA::Vector<DAVA::String>&& cmdLine);
    // desctructor will never call on some platforms,
    // so release all resources in Cleanup method.
    ~REApplication() = default;

protected:
    EngineInitInfo GetInitInfo() const override;
    void CreateModules(DAVA::TArc::Core* tarcCore) const override;

private:
    void CreateGUIModules(DAVA::TArc::Core* tarcCore) const;
    void CreateConsoleModules(DAVA::TArc::Core* tarcCore) const;
    void Init(const DAVA::EngineContext* engineContext) override;
    void Cleanup() override;

    void RegisterEditorAnyCasts() override;
    bool AllowMultipleInstances() const override;
    QString GetInstanceKey() const override;

    DAVA::KeyedArchive* CreateOptions() const;

    bool isConsoleMode = false;
    DAVA::Vector<DAVA::String> cmdLine;

private:
    // singletons. In future we probably will try to move them into special module, or completely decompose
    SettingsManager* settingsManager = nullptr;
    BEAST_PROXY_TYPE* beastProxy = nullptr;
    mutable DAVA::RefPtr<DAVA::KeyedArchive> appOptions;
};
