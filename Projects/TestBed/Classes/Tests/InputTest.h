#ifndef __INPUTTEST_TEST_H__
#define __INPUTTEST_TEST_H__

#include "Infrastructure/BaseScreen.h"

class InputTest : public BaseScreen
{
public:
    InputTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnInputChanged(DAVA::DeviceInfo::eHIDType hidType, bool connected);

    void MousePressed(DAVA::BaseObject* obj, void* data, void* callerData);
    void TouchPressed(DAVA::BaseObject* obj, void* data, void* callerData);
    void KeyboardPressed(DAVA::BaseObject* obj, void* data, void* callerData);

    DAVA::UIButton* mouse = nullptr;
    DAVA::UIButton* touch = nullptr;
    DAVA::UIButton* keyboard = nullptr;
    DAVA::Bitset<DAVA::DeviceInfo::eHIDType::HID_COUNT_TYPE> input;
};
#endif //__INPUTTEST_TEST_H__
