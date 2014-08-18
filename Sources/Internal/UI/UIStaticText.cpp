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


#include "Utils/Utils.h"
#include "UI/UIStaticText.h"
#include "Render/RenderManager.h"
#include "Base/ObjectFactory.h"
#include "UI/UIYamlLoader.h"
#include "Utils/StringFormat.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/YamlNode.h"
#include "Render/2D/FontManager.h"
#include "Animation/LinearAnimation.h"

namespace DAVA
{

UIStaticText::UIStaticText(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
:	UIControl(rect, rectInAbsoluteCoordinates)
    , shadowOffset(0, 0)
{
    SetInputEnabled(false, false);
    textBlock = TextBlock::Create(Vector2(rect.dx, rect.dy));
    background->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);

    shadowBg = new UIControlBackground();
    textBg = new UIControlBackground();
    textBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);

    SetTextColor(Color::White);
    SetShadowColor(Color::Black);
    SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
}

UIStaticText::~UIStaticText()
{
    SafeRelease(textBlock);
    SafeRelease(shadowBg);
    SafeRelease(textBg);
}


UIStaticText *UIStaticText::Clone()
{
    UIStaticText *t = new UIStaticText(GetRect());
    t->CopyDataFrom(this);
    return t;
}

void UIStaticText::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIStaticText *t = (UIStaticText *)srcControl;

    SafeRelease(textBlock);
    textBlock = t->textBlock->Clone();

    shadowOffset = t->shadowOffset;

    SafeRelease(shadowBg);
    SafeRelease(textBg);
    shadowBg = t->shadowBg->Clone();
    textBg = t->textBg->Clone();
}

void UIStaticText::SetText(const WideString& _string, const Vector2 &requestedTextRectSize/* = Vector2(0,0)*/)
{
    textBlock->SetRectSize(size);
    textBlock->SetText(_string, requestedTextRectSize);
    PrepareSprite();
}

void UIStaticText::SetFittingOption(int32 fittingType)
{
    textBlock->SetRectSize(size);
    textBlock->SetFittingOption(fittingType);
    PrepareSprite();
}

int32 UIStaticText::GetFittingOption() const
{
    return textBlock->GetFittingOption();
}

void UIStaticText::SetFont(Font * _font)
{
    textBlock->SetRectSize(size);
    textBlock->SetFont(_font);
    PrepareSprite();
}

void UIStaticText::SetTextColor(const Color& color)
{
    textBg->SetColor(color);
}

void UIStaticText::SetShadowOffset(const Vector2 &offset)
{
    shadowOffset = offset;
}

void UIStaticText::SetShadowColor(const Color &color)
{
    shadowBg->SetColor(color);
}

void UIStaticText::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    textBlock->SetRectSize(size);
    textBlock->SetMultiline(_isMultilineEnabled, bySymbol);
    PrepareSprite();
}

bool UIStaticText::GetMultiline() const
{
    return textBlock->GetMultiline();
}

bool UIStaticText::GetMultilineBySymbol() const
{
    return textBlock->GetMultilineBySymbol();
}

void UIStaticText::SetAlign(int32 _align)
{
    UIControl::SetSpriteAlign(_align);
}

int32 UIStaticText::GetAlign() const
{
    return UIControl::GetSpriteAlign();
}

void UIStaticText::SetTextAlign(int32 _align)
{
    textBg->SetAlign(_align);
    textBlock->SetAlign(_align);
}

int32 UIStaticText::GetTextAlign() const
{
    return textBg->GetAlign();
}

const Vector2 & UIStaticText::GetTextSize()
{
    return textBlock->GetTextSize();
}

const Color &UIStaticText::GetTextColor() const
{
    return textBg->GetColor();
}

const Color &UIStaticText::GetShadowColor() const
{
    return shadowBg->GetColor();
}

const Vector2 &UIStaticText::GetShadowOffset() const
{
    return shadowOffset;
}

void UIStaticText::Draw(const UIGeometricData &geometricData)
{
	if(GetText().empty()) return;

	textBlock->SetRectSize(size);
	textBlock->SetPosition(geometricData.position);
	textBlock->SetPivotPoint(geometricData.pivotPoint);
	textBlock->PreDraw();
	PrepareSprite();

    UIControl::Draw(geometricData);

    if(!FLOAT_EQUAL(shadowBg->GetDrawColor().a, 0.0f) && (!FLOAT_EQUAL(shadowOffset.dx, 0.0f) || !FLOAT_EQUAL(shadowOffset.dy, 0.0f)))
    {
		textBlock->Draw(GetShadowColor(), &shadowOffset);
        UIGeometricData shadowGeomData;
        shadowGeomData.position = shadowOffset;
        shadowGeomData.size = GetSize();
        shadowGeomData.AddToGeometricData(geometricData);

        shadowBg->SetAlign(textBg->GetAlign());
        shadowBg->SetPerPixelAccuracyType(background->GetPerPixelAccuracyType());
        shadowBg->Draw(shadowGeomData);
    }

    textBlock->Draw(GetTextColor());
    textBg->SetPerPixelAccuracyType(background->GetPerPixelAccuracyType());
    textBg->Draw(geometricData);
}

void UIStaticText::SetParentColor(const Color &parentColor)
{
    UIControl::SetParentColor(parentColor);
    shadowBg->SetParentColor(parentColor);
    textBg->SetParentColor(parentColor);
}

const Vector<WideString> & UIStaticText::GetMultilineStrings() const
{
    return textBlock->GetMultilineStrings();
}

const WideString & UIStaticText::GetText() const
{
    return textBlock->GetText();
}

void UIStaticText::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * fontNode = node->Get("font");
    const YamlNode * textNode = node->Get("text");
    const YamlNode * multilineNode = node->Get("multiline");
    const YamlNode * multilineBySymbolNode = node->Get("multilineBySymbol");
    const YamlNode * fittingNode = node->Get("fitting");
    const YamlNode * textColorNode = node->Get("textcolor");
    const YamlNode * shadowColorNode = node->Get("shadowcolor");
    const YamlNode * shadowOffsetNode = node->Get("shadowoffset");
    const YamlNode * textAlignNode = node->Get("textalign");
    const YamlNode * textColorInheritTypeNode = node->Get("textcolorInheritType");
    const YamlNode * shadowColorInheritTypeNode = node->Get("shadowcolorInheritType");

    if (fontNode)
    {
        const String & fontName = fontNode->AsString();
        Font * font = loader->GetFontByName(fontName);
        SetFont(font);
    }

    bool multiline = loader->GetBoolFromYamlNode(multilineNode, false);
    bool multilineBySymbol = loader->GetBoolFromYamlNode(multilineBySymbolNode, false);
    SetMultiline(multiline, multilineBySymbol);

    if(fittingNode)
    {
        SetFittingOption(loader->GetFittingOptionFromYamlNode(fittingNode));
    }

    if(textColorNode)
    {
        SetTextColor(textColorNode->AsColor());
    }

    if(shadowColorNode)
    {
        SetShadowColor(shadowColorNode->AsColor());
    }

    if(shadowOffsetNode)
    {
        SetShadowOffset(shadowOffsetNode->AsVector2());
    }

    if (textAlignNode)
    {
        SetTextAlign(loader->GetAlignFromYamlNode(textAlignNode));
    }

    if (textNode)
    {
        SetText(LocalizedString(textNode->AsWString()));
    }

    if (textColorInheritTypeNode)
    {
        GetTextBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(textColorInheritTypeNode));
    }

    if (shadowColorInheritTypeNode)
    {
        GetShadowBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(shadowColorInheritTypeNode));
    }
}

YamlNode * UIStaticText::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);

    UIStaticText *baseControl = new UIStaticText();

    //Temp variable
    VariantType *nodeValue = new VariantType();

    //Font
    //Get font name and put it here
    nodeValue->SetString(FontManager::Instance()->GetFontName(this->GetFont()));
    node->Set("font", nodeValue);

    //TextColor
    const Color &textColor = GetTextColor();
    if (baseControl->GetTextColor() != textColor)
    {
        nodeValue->SetColor(textColor);
        node->Set("textcolor", nodeValue);
    }

    // Text Color Inherit Type.
    int32 colorInheritType = (int32)GetTextBackground()->GetColorInheritType();
    if (baseControl->GetTextBackground()->GetColorInheritType() != colorInheritType)
    {
        node->Set("textcolorInheritType", loader->GetColorInheritTypeNodeValue(colorInheritType));
    }

    // ShadowColor
    const Color &shadowColor = GetShadowColor();
    if (baseControl->GetShadowColor() != shadowColor)
    {
        nodeValue->SetColor(shadowColor);
        node->Set("shadowcolor", nodeValue);
    }

    // Shadow Color Inherit Type.
    colorInheritType = (int32)GetShadowBackground()->GetColorInheritType();
    if (baseControl->GetShadowBackground()->GetColorInheritType() != colorInheritType)
    {
        node->Set("shadowcolorInheritType", loader->GetColorInheritTypeNodeValue(colorInheritType));
    }

    // ShadowOffset
    const Vector2 &shadowOffset = GetShadowOffset();
    if (baseControl->GetShadowOffset() != shadowOffset)
    {
        nodeValue->SetVector2(shadowOffset);
        node->Set("shadowoffset", nodeValue);
    }

    //Text
    const WideString &text = GetText();
    if (baseControl->GetText() != text)
    {
        node->Set("text", text);
    }
    //Multiline
    if (baseControl->textBlock->GetMultiline() != this->textBlock->GetMultiline())
    {
        node->Set("multiline", this->textBlock->GetMultiline());
    }
    //multilineBySymbol
    if (baseControl->textBlock->GetMultilineBySymbol() != this->textBlock->GetMultilineBySymbol())
    {
        node->Set("multilineBySymbol", this->textBlock->GetMultilineBySymbol());
    }
    //fitting - Array of strings
    if (baseControl->textBlock->GetFittingOption() != this->textBlock->GetFittingOption())
    {
        node->SetNodeToMap("fitting", loader->GetFittingOptionNodeValue(textBlock->GetFittingOption()));
    }

    // Text Align
    if (baseControl->GetTextAlign() != this->GetTextAlign())
    {
        node->SetNodeToMap("textalign", loader->GetAlignNodeValue(this->GetTextAlign()));
    }

    // Draw type. Must be overriden for UITextControls.
    if (baseControl->GetBackground()->GetDrawType() != this->GetBackground()->GetDrawType())
    {
        node->Set("drawType", loader->GetDrawTypeNodeValue(this->GetBackground()->GetDrawType()));
    }

    SafeDelete(nodeValue);
    SafeRelease(baseControl);

    return node;
}

Animation * UIStaticText::TextColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
{
    LinearAnimation<Color> * animation = new LinearAnimation<Color>(this, &textBg->color, finalColor, time, interpolationFunc);
    animation->Start(track);
    return animation;
}

Animation * UIStaticText::ShadowColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 1*/)
{
    LinearAnimation<Color> * animation = new LinearAnimation<Color>(this, &shadowBg->color, finalColor, time, interpolationFunc);
    animation->Start(track);
    return animation;
}

const Vector<int32> & UIStaticText::GetStringSizes() const
{
    return textBlock->GetStringSizes();
}

void UIStaticText::PrepareSprite()
{
    ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &UIStaticText::PrepareSpriteInternal));
}

void UIStaticText::PrepareSpriteInternal(DAVA::BaseObject *caller, void *param, void *callerData)
{
    if (textBlock->IsSpriteReady())
    {
        Sprite *sprite = textBlock->GetSprite();
        shadowBg->SetSprite(sprite, 0);
        textBg->SetSprite(sprite, 0);

        Texture *tex = sprite->GetTexture();
        if(tex && tex->GetFormat() == FORMAT_A8)
        {
            textBg->SetShader(RenderManager::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8);
            shadowBg->SetShader(RenderManager::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8);
        }
        else
        {
            textBg->SetShader(RenderManager::TEXTURE_MUL_FLAT_COLOR);
            shadowBg->SetShader(RenderManager::TEXTURE_MUL_FLAT_COLOR);
        }
    }
    else
    {
        shadowBg->SetSprite(NULL, 0);
        textBg->SetSprite(NULL, 0);
    }
}

};