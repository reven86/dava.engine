#include "Tests/UIBackgroundTest.h"
#include "UI/UIControlBackground.h"

using namespace DAVA;

UIBackgroundTest::UIBackgroundTest()
    : BaseScreen("UIBackgroundTest")
{
}

void UIBackgroundTest::LoadResources()
{
    BaseScreen::LoadResources();
    //TODO: Initialize resources here

    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(14);

    text_orig = new UIStaticText(Rect(0, 0, 100, 100));
    text_orig->SetText(L"orig");
    text_orig->SetFont(font);
    text_orig->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    text_orig->GetBackground()->SetSprite("~res:/Gfx/UI/box", 0);

    text_modif_h = new UIStaticText(Rect(0, 120, 100, 100));
    text_modif_h->SetText(L"H");
    text_modif_h->SetFont(font);
    text_modif_h->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    text_modif_h->GetBackground()->SetModification(ESM_HFLIP);
    text_modif_h->GetBackground()->SetSprite("~res:/Gfx/UI/box", 0);

    text_modif_v = new UIStaticText(Rect(120, 0, 100, 100));
    text_modif_v->SetText(L"V");
    text_modif_v->SetFont(font);
    text_modif_v->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    text_modif_v->GetBackground()->SetModification(ESM_VFLIP);
    text_modif_v->GetBackground()->SetSprite("~res:/Gfx/UI/box", 0);

    text_modif_hv = new UIStaticText(Rect(120, 120, 100, 100));
    text_modif_hv->SetText(L"HV");
    text_modif_hv->SetFont(font);
    text_modif_hv->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_BOTH);
    text_modif_hv->GetBackground()->SetModification(ESM_VFLIP | ESM_HFLIP);
    text_modif_hv->GetBackground()->SetSprite("~res:/Gfx/UI/box", 0);

    AddControl(text_orig);
    AddControl(text_modif_h);
    AddControl(text_modif_v);
    AddControl(text_modif_hv);
}

void UIBackgroundTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    //TODO: Release resources here

    SafeRelease(text_orig);
    SafeRelease(text_modif_h);
    SafeRelease(text_modif_v);
    SafeRelease(text_modif_hv);
}
