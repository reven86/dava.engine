
#include "Tests/AssertTest.h"
#include "UI/Input/UIActionBindingComponent.h"

using namespace DAVA;

AssertTest::AssertTest()
    : BaseScreen("AssertTest")
{
}

void AssertTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::RefPtr<DAVA::Font> korinnaFont(DAVA::FTFont::Create("~res:/Fonts/korinna.ttf"));
    DAVA::Map<DAVA::String, DAVA::Font*> fonts;
    fonts["Font_18"] = korinnaFont.Get();
    DAVA::FontManager::Instance()->RegisterFonts(fonts);

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/AssertTestScreen.yaml", &pkgBuilder);
    auto dialog = pkgBuilder.GetPackage()->GetControl(0);
    dialog->SetSize(DAVA::Vector2(500, 500));
    AddControl(dialog);

    DAVA::UIActionBindingComponent* actions = dialog->GetComponent<DAVA::UIActionBindingComponent>();
    if (actions)
    {
        actions->GetActionMap().Put(DAVA::FastName("NON_MODAL_ASSERT"), [&]() {
            DVASSERT_MSG(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("MODAL_ASSERT"), [&]() {
            DVASSERT_MSG(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("DELAYED_MODAL_ASSERT"), [&]() {
            timeOut = 5.f;
        });
    }

    SafeRelease(dialog);
}

void AssertTest::UnloadResources()
{
    BaseScreen::UnloadResources();
}

void AssertTest::Update(float32 timeElapsed)
{
    if (timeOut > 0.f)
    {
        timeOut -= timeElapsed;
        if (timeOut <= 0.f)
        {
            DVASSERT_MSG(false, "Demo assert");
        }
    }
}
