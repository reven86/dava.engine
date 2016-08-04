#include "Tests/ExceptionAndAssertionTest.h"

ExceptionAndAssertionTest::ExceptionAndAssertionTest()
    : BaseScreen("ExceptionAndAssertionTest")
{
}

void DoAssertion(DAVA::BaseObject*, void*, void*)
{
    DVASSERT_MSG(true == false, "THIS IS ASSERT!")
}

void ThrowException(DAVA::BaseObject*, void*, void*)
{
    try
    {
        throw std::logic_error("Hello world!");
    }
    catch (std::exception& e)
    {
        DVASSERT_MSG(true == false, (String("THIS IS EXCEPTION: ") + e.what()).c_str());
    }
}

void ExceptionAndAssertionTest::LoadResources()
{
    BaseScreen::LoadResources();
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    ScopedPtr<UIButton> assertButton(new UIButton(Rect(420, 30, 200, 60)));
    assertButton->SetDebugDraw(true);
    assertButton->SetStateFont(0xFF, font);
    assertButton->SetStateFontColor(0xFF, Color::White);
    assertButton->SetStateText(0xFF, L"Generate assertion");
    assertButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(&DoAssertion));
    AddControl(assertButton.get());

    ScopedPtr<UIButton> exceptionButton(new UIButton(Rect(420, 100, 200, 60)));
    exceptionButton->SetDebugDraw(true);
    exceptionButton->SetStateFont(0xFF, font);
    exceptionButton->SetStateFontColor(0xFF, Color::White);
    exceptionButton->SetStateText(0xFF, L"Generate exception");
    exceptionButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(&ThrowException));
    AddControl(exceptionButton.get());
}

void ExceptionAndAssertionTest::UnloadResources()
{
    BaseScreen::UnloadResources();
}
