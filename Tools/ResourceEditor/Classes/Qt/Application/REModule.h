#pragma once

#include "TArcCore/ControllerModule.h"

class REModule : public DAVA::TArc::ControllerModule
{
protected:
    void OnRenderSystemInitialized(DAVA::Window* w) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void PostInit() override;
};