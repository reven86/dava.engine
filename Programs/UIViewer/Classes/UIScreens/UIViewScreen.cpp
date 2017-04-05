#include "UIScreens/UIViewScreen.h"

#include <Base/ScopedPtr.h>
#include <UI/UIStaticText.h>
#include <UI/Update/UIUpdateComponent.h>

UIViewScreen::UIViewScreen()
    : BaseScreen()
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();
}

void UIViewScreen::LoadResources()
{
    using namespace DAVA;

    BaseScreen::LoadResources();

    ScopedPtr<UIStaticText> dummyText(new UIStaticText(GetRect()));
    dummyText->SetFont(font);
    dummyText->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    dummyText->SetTextColor(Color(1.f, 1.f, 1.f, 1.f));
    dummyText->SetText(L"UIViewer. Dummy Text");
    AddControl(dummyText);
}
