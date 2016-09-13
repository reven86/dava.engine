#pragma once

#include "Infrastructure/BaseScreen.h"

class GameCore;
class DeviceInfoTest : public BaseScreen
{
public:
    DeviceInfoTest(GameCore* g);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void UpdateTestInfo();
    void OnInputChanged(DAVA::DeviceInfo::eHIDType hidType, bool connected);
    void OnCarrierChanged(const DAVA::String&);
    DAVA::UIStaticText* info = nullptr;
    DAVA::Map<DAVA::DeviceInfo::eHIDType, bool> hidDevices;
};
