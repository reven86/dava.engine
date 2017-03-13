#pragma once

#include "TArc/Core/Private/CoreInterface.h"
#include "TArc/Core/ControllerModule.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Core/ConsoleModule.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"

#include <memory>

class QWidget;
namespace DAVA
{
class Engine;

namespace TArc
{
// back compatibility
class CoreInterface;

class Core final : public TrackedObject
{
public:
    Core(Engine& engine);
    ~Core();

    template <typename T, typename... Args>
    void CreateModule(Args&&... args)
    {
        CreateModule(ReflectedTypeDB::Get<T>(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void CreateModule(const ReflectedType* reflectedType, Args&&... args)
    {
        const Type* type = reflectedType->GetType()->Pointer();
        bool isClientModule = TypeInheritance::CanCast(type, Type::Instance<ClientModule*>());
        bool isControllerModule = TypeInheritance::CanCast(type, Type::Instance<ControllerModule*>());
        bool isConsoleModule = TypeInheritance::CanCast(type, Type::Instance<ConsoleModule*>());

        DVASSERT(isClientModule == true || isConsoleModule == true || isConsoleModule, "Module should be Derived from one of base classes: ControllerModule, ClientModule, ConsoleModule");

        bool isConsoleMode = IsConsoleMode();
        if (isConsoleMode == true && isConsoleModule == false)
        {
            DVASSERT(false, "In console mode module should be Derived from ConsoleModule");
            return;
        }

        if (isConsoleMode == false && isConsoleModule == true)
        {
            DVASSERT(false, "In GUI mode module should be Derived from ControllerModule or ClientModule");
            return;
        }

        Any object = reflectedType->CreateObject(ReflectedType::CreatePolicy::ByPointer, std::forward<Args>(args)...);
        if (isControllerModule)
        {
            AddModule(object.Cast<ControllerModule*>());
        }
        else if (isClientModule)
        {
            AddModule(object.Cast<ClientModule*>());
        }
        else
        {
            AddModule(object.Cast<ConsoleModule*>());
        }
    }

    DAVA_DEPRECATED(const EngineContext* GetEngineContext());
    DAVA_DEPRECATED(CoreInterface* GetCoreInterface());
    DAVA_DEPRECATED(const CoreInterface* GetCoreInterface() const);
    DAVA_DEPRECATED(UI* GetUI());

private:
    // in testing environment Core shouldn't connect to Engine signals.
    // TArcTestClass wrap signals and call Core method directly
    Core(Engine& engine, bool connectSignals);
    bool IsConsoleMode() const;
    // Don't put AddModule methods into public sections.
    // There is only one orthodox way to inject Module into TArcCore : CreateModule
    void AddModule(ConsoleModule* module);
    void AddModule(ClientModule* module);
    void AddModule(ControllerModule* module);

    friend class TestClass;
    void OnLoopStarted();
    void OnLoopStopped();
    void OnFrame(float32 delta);
    void OnWindowCreated(DAVA::Window* w);
    bool HasControllerModule() const;
    void SetInvokeListener(OperationInvoker* proxyInvoker);
    DAVA::Signal<> syncSignal;

private:
    class Impl;
    class GuiImpl;
    class ConsoleImpl;

    std::unique_ptr<Impl> impl;
};

} // namespace TArc
} // namespace DAVA
