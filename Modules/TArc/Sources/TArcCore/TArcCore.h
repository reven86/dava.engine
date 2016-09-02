#pragma once

#include "TArcCore/Private/CoreInterface.h"
#include "WindowSubSystem/Private/UIManager.h"

#include "Base/BaseTypes.h"
#include "Functional/SignalBase.h"

#include <memory>

namespace DAVA
{
class Engine;

namespace TArc
{
class ClientModule;
class ControllerModule;
class ConsoleModule;

class Core final : public TrackedObject
{
public:
    Core(Engine& engine);
    ~Core();

    template<typename T, typename... Args>
    void CreateModule(const Args&... args)
    {
        static_assert(std::is_base_of<ConsoleModule, T>::value ||
                        std::is_base_of<ClientModule, T>::value ||
                        std::is_base_of<ControllerModule, T>::value,
                        "Module should be Derived from one of base classes: ControllerModule, ClientModule, ConsoleModule");

        bool isConsoleMode = IsConsoleMode();
        bool isConsoleModule = std::is_base_of<ConsoleModule, T>::value;
        if (isConsoleMode == true && isConsoleModule == false)
        {
            DVASSERT_MSG(false, "In console mode module should be Derived from ConsoleModule");
            return;
        }

        if (isConsoleMode == false && isConsoleModule == true)
        {
            DVASSERT_MSG(false, "In GUI mode module should be Derived from ControllerModule or ClientModule");
            return;
        }

        AddModule(new T(args...));
    }

private:
    // in testing enviroment Core shouldn't connect to Engine signals.
    // TArcTestClass wrap signals and call Core method directly
    Core(Engine& engine, bool connectSignals);
    bool IsConsoleMode() const;
    void AddModule(ConsoleModule* module);
    void AddModule(ClientModule* module);
    void AddModule(ControllerModule* module);

    friend class TestClass;
    void OnLoopStarted();
    void OnLoopStopped();
    void OnFrame(float32 delta);
    void OnWindowCreated(DAVA::Window& w);
    bool HasControllerModule() const;

    OperationInvoker* GetMockInvoker();
    DataContext& GetActiveContext();
    DataWrapper CreateWrapper(const DAVA::ReflectedType* type);

private:
    class Impl;
    class GuiImpl;
    class ConsoleImpl;

    std::unique_ptr<Impl> impl;
};

} // namespace TArc
} // namespace DAVA
