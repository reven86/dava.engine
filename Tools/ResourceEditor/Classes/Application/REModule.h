#pragma once

#include "TArc/Core/ControllerModule.h"

#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"

class TextureCache;
class ResourceEditorLauncher;
class REModule : public DAVA::TArc::ControllerModule, private DAVA::TArc::DataListener
{
public:
    ~REModule();

protected:
    void OnRenderSystemInitialized(DAVA::Window* w) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key) override;
    bool ControlWindowClosing(const DAVA::TArc::WindowKey& key, QCloseEvent* event) override;
    void SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void PostInit() override;

private:
    void UnpackHelpDoc();
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) override;

private:
    DAVA::TArc::DataWrapper launchDataWrapper;
};