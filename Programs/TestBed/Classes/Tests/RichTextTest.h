#pragma once

#include "Infrastructure/BaseScreen.h"
#include <Base/RefPtr.h>
#include <UI/UITextField.h>

class TestBed;
class RichInputDelegate;

class RichTextTest : public BaseScreen
{
public:
    RichTextTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    RichInputDelegate* inputDelegate = nullptr;
    RefPtr<UITextField> inputField;
    RefPtr<UIControl> richText;
};
