#pragma once

#include "Base/Platform.h"
#include "TArc/Core/ControllerModule.h"

class QEModule : public DAVA::TArc::ControllerModule
{
public:
    ~QEModule();

protected:
    void OnRenderSystemInitialized(DAVA::Window* w) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(QEModule, DAVA::TArc::ControllerModule)
    {
        DAVA::ReflectionRegistrator<QEModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
