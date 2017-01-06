#pragma once

#include "TArc/Core//BaseApplication.h"

namespace DAVA
{
namespace TArc
{
class Core;
}
}

class QEApplication : public DAVA::TArc::BaseApplication
{
public:
    QEApplication(DAVA::Vector<DAVA::String>&& cmdLine);
    // desctructor will never call on some platforms,
    // so release all resources in Cleanup method.
    ~QEApplication() = default;

protected:
    EngineInitInfo GetInitInfo() const override;
    void CreateModules(DAVA::TArc::Core* tarcCore) const override;

private:
    void CreateGUIModules(DAVA::TArc::Core* tarcCore) const;
    void CreateConsoleModules(DAVA::TArc::Core* tarcCore) const;
    void Init(const DAVA::EngineContext* engineContext) override;
    void Cleanup() override;

    bool AllowMultipleInstances() const override;
    QString GetInstanceKey() const override;

    bool isConsoleMode = false;
    DAVA::Vector<DAVA::String> cmdLine;
};
