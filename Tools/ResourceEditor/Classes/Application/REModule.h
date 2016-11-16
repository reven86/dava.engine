#pragma once

#include "TArc/Core/ClientModule.h"

#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"

class TextureCache;
class ResourceEditorLauncher;
class REModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
public:
    ~REModule();

protected:
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void PostInit() override;

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

private:
    DAVA::TArc::DataWrapper launchDataWrapper;
};