#include "Tests/UILoggingTest.h"
#include "Analytics/Analytics.h"
#include "Analytics/LoggingBackend.h"
#include "UI/Render/UIDebugRenderComponent.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#else
#include "Core/Core.h"
#endif

using namespace DAVA;

Analytics::Core& GetCore()
{
#if defined(__DAVAENGINE_COREV2__)
    return *Engine::Instance()->GetContext()->analyticsCore;
#else
    return Core::Instance()->GetAnalyticsCore();
#endif
}

UILoggingTest::UILoggingTest(TestBed& app)
    : BaseScreen(app, "UI Logging test")
{
    auto backend = std::make_unique<Analytics::LoggingBackend>("~doc:/AnalyticsLog.txt");
    GetCore().AddBackend("LoggingBackend", std::move(backend));
}

void UILoggingTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    font->SetSize(12);

    Rect btnRect(10.0f, 10.0f, 200.0f, 40.0f);
    switchButton.Set(CreateUIButton(font, btnRect, &UILoggingTest::OnSwitch));
    AddControl(switchButton.Get());
    UpdateSwithButton();

    RefPtr<KeyedArchive> config(new KeyedArchive);
    config->LoadFromYamlFile("~res:/AnalyticsConfig.yaml");
    GetCore().SetConfig(config.Get());
}

void UILoggingTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    switchButton.Set(nullptr);
}

UIButton* UILoggingTest::CreateUIButton(Font* font, const Rect& rect,
                                        void (UILoggingTest::*onClick)(BaseObject*, void*, void*))
{
    using namespace DAVA;

    UIButton* button = new UIButton(rect);

    button->SetStateFont(0xFF, font);
    button->SetStateFontColor(0xFF, Color::White);
    button->GetOrCreateComponent<UIDebugRenderComponent>();
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, onClick));

    return button;
}

void UILoggingTest::OnSwitch(DAVA::BaseObject* obj, void* data, void* callerData)
{
    Analytics::Core& core = GetCore();

    if (core.IsStarted())
    {
        core.Stop();
    }
    else
    {
        core.Start();
    }

    UpdateSwithButton();
}

void UILoggingTest::UpdateSwithButton()
{
    Analytics::Core& core = GetCore();
    WideString text = core.IsStarted() ? L"Stop UI Logging" : L"Start UI Logging";
    switchButton->SetStateText(0xFF, text);
}
