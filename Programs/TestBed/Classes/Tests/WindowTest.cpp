#include "Tests/WindowTest.h"

#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <Engine/Window.h>
#include <Utils/StringFormat.h>

using namespace DAVA;

class TextFieldDelegate : public UITextFieldDelegate
{
public:
    TextFieldDelegate(Function<void(const Rect& keyboardRect)> callback)
        : callback(callback)
    {
    }

    void OnKeyboardShown(const Rect& keyboardRect) override
    {
        callback(keyboardRect);
    }

    void OnKeyboardHidden() override
    {
        callback(Rect());
    }

private:
    const Function<void(const Rect& keyboardRect)> callback;
};

WindowTest::WindowTest(TestBed& app)
    : BaseScreen(app, "WindowTest")
{
}

void WindowTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/WindowTest.yaml", &pkgBuilder);
    UIControl* main = pkgBuilder.GetPackage()->GetControl("MainFrame");
    AddControl(main);

    visibleFrame = main->FindByPath("**/VisibleFrame");
    visibleFrameRectText = main->FindByPath<UIStaticText*>("**/VisibleFrameRectText");
    keyboardFrameRectText = main->FindByPath<UIStaticText*>("**/KeyboardFrameRectText");
    textField = main->FindByPath<UITextField*>("**/TextField");

    tfDelegate = new TextFieldDelegate([this](const Rect& r) {
        UpdateKeyboardFrameSize(r);
    });
    textField->SetDelegate(tfDelegate);

    visibleFrameChangedId = GetPrimaryWindow()->visibleFrameChanged.Connect([this](Window*, const Rect& r) {
        Rect converted = UIControlSystem::Instance()->vcs->ConvertInputToVirtual(r);
        UpdateVisibleFrameSize(converted);
    });

    UpdateVisibleFrameSize(main->GetRect());
}

void WindowTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    textField->SetDelegate(nullptr);
    SafeDelete(tfDelegate);

    GetPrimaryWindow()->visibleFrameChanged.Disconnect(visibleFrameChangedId);
    visibleFrameChangedId = 0;
}

void WindowTest::UpdateKeyboardFrameSize(const Rect& r)
{
    Vector2 virtualSize = Vector2(static_cast<float32>(UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dx), static_cast<float32>(UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dy));
    Rect clamped = r.Intersection(Rect(Vector2(), virtualSize));
    keyboardFrameRectText->SetUtf8Text(Format("%.3f, %.3f - %.3f, %.3f", clamped.x, clamped.y, clamped.dx, clamped.dy));
}

void WindowTest::UpdateVisibleFrameSize(const Rect& r)
{
    Vector2 virtualSize = Vector2(static_cast<float32>(UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dx), static_cast<float32>(UIControlSystem::Instance()->vcs->GetVirtualScreenSize().dy));
    Rect clamped = r.Intersection(Rect(Vector2(), virtualSize));
    visibleFrameRectText->SetUtf8Text(Format("%.3f, %.3f - %.3f, %.3f", clamped.x, clamped.y, clamped.dx, clamped.dy));
    visibleFrame->SetRect(clamped);
}
