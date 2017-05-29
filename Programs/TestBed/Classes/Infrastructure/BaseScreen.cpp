#include "Infrastructure/BaseScreen.h"
#include "Infrastructure/TestBed.h"

#include <UI/Layouts/UIAnchorComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>

DAVA::int32 BaseScreen::globalScreenId = 1;

BaseScreen::BaseScreen(TestBed& app, const DAVA::String& screenName)
    : UIScreen()
    , app(app)
    , currentScreenId(globalScreenId++)
{
    SetName(screenName);
    app.RegisterScreen(this);
}

bool BaseScreen::SystemInput(DAVA::UIEvent* currentInput)
{
    using namespace DAVA;
    if ((currentInput->key == Key::BACK) && (currentInput->phase == UIEvent::Phase::KEY_DOWN))
    {
        OnExitButton(nullptr, nullptr, nullptr);
    }
    else
    {
        return UIScreen::SystemInput(currentInput);
    }
    return true;
}

void BaseScreen::LoadResources()
{
    using namespace DAVA;
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    font->SetSize(30);

    Size2i screenSize = UIControlSystem::Instance()->vcs->GetVirtualScreenSize();
    exitButton = new UIButton(Rect(static_cast<DAVA::float32>(screenSize.dx - 300), static_cast<DAVA::float32>(screenSize.dy - 30), 300.0, 30.0));
    exitButton->SetStateFont(0xFF, font);
    exitButton->SetStateFontColor(0xFF, Color::White);
    exitButton->SetStateText(0xFF, L"Exit From Screen");

    exitButton->GetOrCreateComponent<UIDebugRenderComponent>();
    exitButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &BaseScreen::OnExitButton));

    {
        // Stick button to bottom right corner
        UIAnchorComponent* anchor = exitButton->GetOrCreateComponent<UIAnchorComponent>();
        anchor->SetBottomAnchorEnabled(true);
        anchor->SetRightAnchorEnabled(true);
    }

    AddControl(exitButton);
}

void BaseScreen::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(exitButton);

    UIScreen::UnloadResources();
}

void BaseScreen::OnExitButton(BaseObject* obj, void* data, void* callerData)
{
    app.ShowStartScreen();
}
