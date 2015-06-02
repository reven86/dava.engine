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


#include "ParseTextTest.h"

ParseTextTest::ParseTextTest(Font::eFontType fontType)
:   UITestTemplate<ParseTextTest>("ParseTextTest")
,   wrapBySymbolShort(NULL)
,   wrapByWordShort(NULL)
,   wrapBySymbolLong(NULL)
,   wrapByWordLong(NULL)
,   requestedFontType(fontType)
{
    RegisterFunction(this, &ParseTextTest::ParseTestFunction, Format("ParseTextTest"), NULL);
}

ParseTextTest::~ParseTextTest()
{
    SafeRelease(wrapBySymbolShort);
    SafeRelease(wrapByWordShort);
    SafeRelease(wrapBySymbolLong);
    SafeRelease(wrapByWordLong);
}

void ParseTextTest::LoadResources()
{
    UITestTemplate<ParseTextTest>::LoadResources();
    
	WideString textShort, textLong;

    textShort = L"The_Very_Long_Phrase_Without_Spaces Short_Word";
//	textShort = L"test";

	textLong = L"This is test of multistring text with very long phrase The_Very_Long_Phrase_Without_Spaces. We use this phrase for test of wrapping";
//	textLong = L"ThisistestofmultistringtextwithverylongphraseTheVeryLongPhraseWithoutSpacesWeusethisphrasefortestofwrapping";

    wrapBySymbolShort = CreateTextControl(Rect(0, 50, 200, 200), textShort, true);
    AddControl(wrapBySymbolShort);
    wrapBySymbolLong = CreateTextControl(Rect(250, 50, 200, 200), textLong, true);
    AddControl(wrapBySymbolLong);

    wrapByWordShort = CreateTextControl(Rect(0, 250, 200, 200), textShort, false);
    AddControl(wrapByWordShort);
    wrapByWordLong = CreateTextControl(Rect(250, 250, 200, 200), textLong, false);
    AddControl(wrapByWordLong);

// 	UIStaticText * text = NULL;
//     text = CreateTextControl(Rect(), L"test2\n    test4", false, Vector2(-1.0f, -1.0f));
//     text->SetSize( text->GetTextSize() );
//     text->SetPosition(Vector2(0, 500));
//     AddControl( text );
//     SafeRelease(text);
// 
//     
//     text = CreateTextControl(Rect(), L"test2\n\n    test4", false, Vector2(-1.0f, -1.0f));
//     text->SetSize( text->GetTextSize() );
//     text->SetPosition(Vector2(250, 500));
//     AddControl( text );
//     SafeRelease(text);
//     
//     text = CreateTextControl(Rect(10, 650, 200, 50), L"test2  test4", false);
//     AddControl( text );
//     SafeRelease(text);
}

void ParseTextTest::UnloadResources()
{
    SafeRelease(wrapBySymbolShort);
    SafeRelease(wrapByWordShort);
    SafeRelease(wrapBySymbolLong);
    SafeRelease(wrapByWordLong);
    
    UITestTemplate<ParseTextTest>::UnloadResources();
}

UIStaticText * ParseTextTest::CreateTextControl(const Rect &rect, const WideString & text, bool wrapBySymbol, const Vector2 &requestedSize /*= Vector2(0, 0)*/)
{
    UIStaticText* textControl = new UIStaticText(rect);
    textControl->SetText(text, requestedSize);
    textControl->SetDebugDraw(true);
    textControl->SetTextAlign(ALIGN_VCENTER | ALIGN_LEFT);
    textControl->SetMultiline(true, wrapBySymbol);

	Font *font = NULL;
	switch(requestedFontType)
	{
	case Font::TYPE_FT:
		font = FTFont::Create("~res:/Fonts/korinna.ttf");
		font->SetSize(14.f);
		break;

	case Font::TYPE_GRAPHICAL:
		font = GraphicsFont::Create("~res:/Fonts/korinna.def", "~res:/Gfx/Fonts/korinna");
		break;

		default: break;
	}

    DVASSERT(font);

	if(font)
	{
		textControl->SetFont(font);
		font->Release();
	}

    return textControl;
}

                                     
                                     
void ParseTextTest::ParseTestFunction(PerfFuncData * testData)
{
    
}
