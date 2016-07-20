
#include "Tests/AssertTest.h"
#include "UI/Input/UIActionBindingComponent.h"

using namespace DAVA;

const static float32 DEFAULT_TIMEOUT = 5.f;

AssertTest::AssertTest()
    : BaseScreen("AssertTest")
{
}

void AssertTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/AssertTestScreen.yaml", &pkgBuilder);
    auto dialog = pkgBuilder.GetPackage()->GetControl(0);
    dialog->SetSize(DAVA::Vector2(500, 500));
    AddControl(dialog);

    DAVA::UIActionBindingComponent* actions = dialog->GetComponent<DAVA::UIActionBindingComponent>();
    if (actions)
    {
        actions->GetActionMap().Put(DAVA::FastName("NON_MODAL_ASSERT"), [&]() {
            DVWARNING(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("MODAL_ASSERT"), [&]() {
            DVASSERT_MSG(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("DELAYED_MODAL_ASSERT"), [&]() {
            timeOut = DEFAULT_TIMEOUT;
        });
    }

    countdownText = static_cast<UIStaticText*>(dialog->FindByName("Countdown"));
}

void AssertTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    countdownText.Set(nullptr);
}

void AssertTest::Update(float32 timeElapsed)
{
    if (timeOut > 0.f)
    {
        timeOut -= timeElapsed;
        if (timeOut <= 0.f)
        {
            timeOut = 0.f;
            DVASSERT_MSG(false, "Demo assert");
        }
    }
    if (countdownText.Get())
    {
        countdownText->SetText(DAVA::Format(L"%1.1f", timeOut));
    }
}
