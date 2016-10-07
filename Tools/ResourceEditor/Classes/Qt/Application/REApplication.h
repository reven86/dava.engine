#pragma once

#include "TArcCore/BaseApplication.h"

namespace DAVA
{
namespace TArc
{
class Core;
}
}

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
    void Cleanup() override;

    bool isConsoleMode = false;
    DAVA::Vector<DAVA::String> cmdLine;
};