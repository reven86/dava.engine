/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

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
