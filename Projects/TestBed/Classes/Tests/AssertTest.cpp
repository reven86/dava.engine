
#include "Tests/AssertTest.h"
#include "UI/Input/UIActionBindingComponent.h"

const static DAVA::float32 DEFAULT_TIMEOUT = 3.f;

AssertTest::AssertTest(GameCore* g)
    : BaseScreen(g, "AssertTest")
{
}

void AssertTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/AssertTestScreen.yaml", &pkgBuilder);
    DAVA::UIControl* dialog = pkgBuilder.GetPackage()->GetControl(0);
    dialog->SetSize(DAVA::Vector2(500, 500));
    AddControl(dialog);

    DAVA::UIActionBindingComponent* actions = dialog->GetComponent<DAVA::UIActionBindingComponent>();
    if (actions)
    {
        actions->GetActionMap().Put(DAVA::FastName("NON_MODAL_ASSERT"), [&]() {
            // DVWARNING(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("MODAL_ASSERT"), [&]() {
            DVASSERT(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("DELAYED_MODAL_ASSERT"), [&]() {
            timeOut = DEFAULT_TIMEOUT;
        });
    }

    countdownText = static_cast<DAVA::UIStaticText*>(dialog->FindByName("Countdown"));
}

void AssertTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    countdownText.Set(nullptr);
}

void AssertTest::Update(DAVA::float32 timeElapsed)
{
    if (timeOut > 0.f)
    {
        timeOut -= timeElapsed;
        if (timeOut <= 0.f)
        {
            timeOut = 0.f;
            DVASSERT(false, "Demo assert");
        }
    }
    if (countdownText.Get())
    {
        countdownText->SetText(DAVA::Format(L"%1.1f", timeOut));
    }
}
