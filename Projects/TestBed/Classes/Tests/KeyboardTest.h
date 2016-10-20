#ifndef __KEYBOARDTEST_TEST_H__
#define __KEYBOARDTEST_TEST_H__

#include "Infrastructure/BaseScreen.h"

class TestBed;
class KeyboardTest : public BaseScreen
{
public:
    KeyboardTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnResetClick(DAVA::BaseObject* sender, void* data, void* callerData);

    TestBed& app;
    DAVA::UIStaticText* previewText = nullptr;
    DAVA::UIButton* resetButton = nullptr;
    DAVA::UIControl* gamepad = nullptr;
};

#endif //__KEYBOARDTEST_TEST_H__
