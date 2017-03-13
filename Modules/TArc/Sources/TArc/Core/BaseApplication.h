#pragma once

#include "Base/RefPtr.h"

#include "Engine/EngineTypes.h"
#include "FileSystem/KeyedArchive.h"

#include <QString>

namespace DAVA
{
class EngineContext;
namespace TArc
{
class Core;
class BaseApplication
{
public:
    BaseApplication() = default;

    int Run();

protected:
    struct EngineInitInfo
    {
        RefPtr<KeyedArchive> options;
        eEngineRunMode runMode;
        Vector<String> modules;
    };

    virtual EngineInitInfo GetInitInfo() const = 0;
    virtual void CreateModules(Core* tarcCore) const = 0;
    // Method init has been written only for backward compatibility. Try not using it
    virtual void Init(const EngineContext* engineContext);
    virtual void Init(Core* tarcCore);
    virtual void RegisterEditorAnyCasts();
    virtual void RegisterReflectionExtensions();
    virtual void Cleanup();

    virtual bool AllowMultipleInstances() const;
    virtual QString GetInstanceKey() const;

    bool IsTestEnvironment() const;

private:
    int RunImpl();

    bool isTestEnv = false;
};

} // namespace TArc
} // namespace DAVA
