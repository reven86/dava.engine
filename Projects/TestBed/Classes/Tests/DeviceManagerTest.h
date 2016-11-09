#pragma once

#include "Infrastructure/BaseScreen.h"

#include <Engine/DeviceManagerTypes.h>

namespace DAVA
{
class DeviceManager;
}

class TestBed;
class DeviceManagerTest : public BaseScreen
{
public:
    DeviceManagerTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnDisplayConfigChanged();
    void OnDisplayClick(DAVA::BaseObject*, void* args, void*);

    DAVA::DeviceManager* deviceManager = nullptr;
    DAVA::Vector<DAVA::UIStaticText*> uiDisplays;
    DAVA::UIStaticText* uiDisplayDescr = nullptr;
    size_t tokenDisplayConfigChanged;

    DAVA::Vector<DAVA::DisplayInfo> displays;
};
