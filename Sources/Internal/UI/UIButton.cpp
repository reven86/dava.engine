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


#include "UI/UIButton.h"
#include "UI/UIStaticText.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIEvent.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/YamlNode.h"
#include "Render/2D/FontManager.h"

namespace DAVA
{
static const UIControl::eControlState stateArray[] = {UIControl::STATE_NORMAL, UIControl::STATE_PRESSED_OUTSIDE, UIControl::STATE_PRESSED_INSIDE, UIControl::STATE_DISABLED, UIControl::STATE_SELECTED, UIControl::STATE_HOVER};
static const String statePostfix[] = {"Normal", "PressedOutside", "PressedInside", "Disabled", "Selected", "Hover"};

UIButton::UIButton(const Rect& rect)
    : UIControl(rect)
    , selectedBackground(NULL)
    , selectedTextBlock(NULL)
    , oldControlState(0)
{
    for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
    {
        stateBacks[i] = NULL;
        stateTexts[i] = NULL;
    }

    stateBacks[DRAW_STATE_UNPRESSED] = SafeRetain(background);

    SetExclusiveInput(true, false);
    SetInputEnabled(true, false);

    selectedBackground = GetActualBackgroundForState(controlState);
}

UIButton::~UIButton()
{
    selectedBackground = NULL;
    selectedTextBlock = NULL;
    for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
    {
        SafeRelease(stateBacks[i]);
        SafeRelease(stateTexts[i]);
    }
}

UIButton *UIButton::Clone()
{
    UIButton *b = new UIButton(GetRect());
    b->CopyDataFrom(this);
    return b;
}

void UIButton::CopyDataFrom(UIControl *srcControl)
{
    selectedBackground = NULL;
    selectedTextBlock = NULL;

    UIControl::CopyDataFrom(srcControl);
    UIButton *srcButton = static_cast<UIButton *>(srcControl);
    for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
    {
        eButtonDrawState drawState = (eButtonDrawState)i;
        if (srcButton->GetBackground(drawState))
        {
            SetBackground(drawState, ScopedPtr<UIControlBackground>(srcButton->GetBackground(drawState)->Clone()));
        }
        if (srcButton->GetTextBlock(drawState))
        {
            SetTextBlock(drawState, ScopedPtr<UIStaticText>(srcButton->GetTextBlock(drawState)->Clone()));
        }
    }
}

void UIButton::SetRect(const Rect &rect)
{
    UIControl::SetRect(rect);

    UpdateStateTextControlSize();
}

void UIButton::SetSize( const Vector2 &newSize )
{
    UIControl::SetSize(newSize);

    UpdateStateTextControlSize();
}

void UIButton::SetStateSprite(int32 state, const FilePath &spriteName, int32 spriteFrame/* = 0*/)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetSprite(spriteName, spriteFrame);
        }
        state >>= 1;
    }
}

void UIButton::SetStateSprite(int32 state, Sprite *newSprite, int32 spriteFrame/* = 0*/)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetSprite(newSprite, spriteFrame);
        }
        state >>= 1;
    }
}

void UIButton::SetStateFrame(int32 state, int32 spriteFrame)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetFrame(spriteFrame);
        }
        state >>= 1;
    }
}

void UIButton::SetStateDrawType(int32 state, UIControlBackground::eDrawType drawType)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetDrawType(drawType);
        }
        state >>= 1;
    }
}

void UIButton::SetStateAlign(int32 state, int32 align)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetAlign(align);
        }
        state >>= 1;
    }
}

void UIButton::SetStateModification(int32 state, int32 modification)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetModification(modification);
        }
        state >>= 1;
    }
}

void UIButton::SetStateColor(int32 state, Color color)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetColor(color);
        }
        state >>= 1;
    }
}

void UIButton::SetStateColorInheritType(int32 state, UIControlBackground::eColorInheritType value)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetColorInheritType(value);
        }
        state >>= 1;
    }
}

void UIButton::SetStatePerPixelAccuracyType(int32 state, UIControlBackground::ePerPixelAccuracyType value)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetPerPixelAccuracyType(value);
        }
        state >>= 1;
    }
}

void UIButton::CreateBackgroundForState(int32 state)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i);
        }

        state >>= 1;
    }
}

Sprite* UIButton::GetStateSprite(int32 state)
{
    return GetActualBackgroundForState(state)->GetSprite();
}
int32 UIButton::GetStateFrame(int32 state)
{
    return GetActualBackgroundForState(state)->GetFrame();
}
UIControlBackground::eDrawType UIButton::GetStateDrawType(int32 state)
{
    return GetActualBackgroundForState(state)->GetDrawType();
}
int32 UIButton::GetStateAlign(int32 state)
{
    return GetActualBackgroundForState(state)->GetAlign();
}

UIControlBackground *UIButton::GetStateBackground(int32 state)
{
    return GetActualBackgroundForState(state);
}

void UIButton::SetStateBackground(int32 state, UIControlBackground *newBackground)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            SetBackground((eButtonDrawState)i, ScopedPtr<UIControlBackground>(newBackground->Clone()));
        }
        state >>= 1;
    }
}

void UIButton::SetStateFont(int32 state, Font *font)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetFont(font);
        }
        state >>= 1;
    }
}

void UIButton::SetStateFontColor(int32 state, const Color& fontColor)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetTextColor(fontColor);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextColorInheritType(int32 state, UIControlBackground::eColorInheritType colorInheritType)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            UIStaticText* staticText = GetOrCreateTextBlock((eButtonDrawState)i);
            staticText->GetTextBackground()->SetColorInheritType(colorInheritType);
            staticText->GetShadowBackground()->SetColorInheritType(colorInheritType);
        }

        state >>= 1;
    }
}

void UIButton::SetStateTextPerPixelAccuracyType(int32 state, UIControlBackground::ePerPixelAccuracyType pixelAccuracyType)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            UIStaticText* staticText = GetOrCreateTextBlock((eButtonDrawState)i);
            staticText->GetTextBackground()->SetPerPixelAccuracyType(pixelAccuracyType);
            staticText->GetShadowBackground()->SetPerPixelAccuracyType(pixelAccuracyType);
        }

        state >>= 1;
    }
}

void UIButton::SetStateShadowColor(int32 state, const Color& shadowColor)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetShadowColor(shadowColor);
        }
        state >>= 1;
    }
}

void UIButton::SetStateShadowOffset(int32 state, const Vector2& offset)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetShadowOffset(offset);
        }
        state >>= 1;
    }
}

void UIButton::SetStateFittingOption(int32 state, int32 fittingOption)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetFittingOption(fittingOption);
        }
        state >>= 1;
    }
}

void UIButton::SetStateText(int32 state, const WideString &text, const Vector2 &requestedTextRectSize/* = Vector2(0,0)*/)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetText(text, requestedTextRectSize);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextAlign(int32 state, int32 align)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetTextAlign(align);
        }
        state >>= 1;
    }
}
	
void UIButton::SetStateTextUseRtlAlign(int32 state, TextBlock::eUseRtlAlign value)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetTextUseRtlAlign(value);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextMultiline(int32 state, bool value)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            UIStaticText* text = GetOrCreateTextBlock((eButtonDrawState)i);
            text->SetMultiline(value, text->GetMultilineBySymbol());
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextMultilineBySymbol(int32 state, bool value)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            UIStaticText* text = GetOrCreateTextBlock((eButtonDrawState)i);
            text->SetMultiline(text->GetMultiline(), value);
        }
        state >>= 1;
    }
}

void UIButton::SetStateMargins(int32 state, const UIControlBackground::UIMargins* margins)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateBackground((eButtonDrawState)i)->SetMargins(margins);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextMargins(int32 state, const UIControlBackground::UIMargins* margins)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            GetOrCreateTextBlock((eButtonDrawState)i)->SetMargins(margins);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextControl(int32 state, UIStaticText *textControl)
{
    for(int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if(state & 0x01)
        {
            SetTextBlock((eButtonDrawState)i, ScopedPtr<UIStaticText>(textControl->Clone()));
        }
        state >>= 1;
    }
}

UIStaticText *UIButton::GetStateTextControl(int32 state)
{
    return GetActualTextBlockForState(state);
}

void UIButton::Input(UIEvent *currentInput)
{
    UIControl::Input(currentInput);
    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

void UIButton::SetBackground(UIControlBackground *newBg)
{
    DVASSERT(false);
}

UIControlBackground * UIButton::GetBackground() const
{
    return selectedBackground;
}

void UIButton::SystemDraw( const UIGeometricData &geometricData )
{
    if (oldControlState != controlState)
    {
        oldControlState = controlState;
        selectedTextBlock = GetActualTextBlockForState(controlState);
        selectedBackground = GetActualBackgroundForState(controlState);
    }

    UIControl::SystemDraw(geometricData);
}

void UIButton::Draw(const UIGeometricData &geometricData)
{
    DVASSERT(selectedBackground);
    selectedBackground->Draw(geometricData);
    
    if (selectedTextBlock)
    {
        selectedTextBlock->Draw(geometricData);
    }
}

void UIButton::SetParentColor( const Color &parentColor )
{
    UIControl::SetParentColor(parentColor);
    DVASSERT(selectedBackground);
    selectedBackground->SetParentColor(parentColor);
    if (selectedTextBlock)
        selectedTextBlock->SetParentColor(selectedBackground->GetDrawColor());
}

UIControlBackground *UIButton::GetActualBackgroundForState(int32 state) const
{
    return GetActualBackground(ControlStateToDrawState(state));
}

UIStaticText *UIButton::GetActualTextBlockForState(int32 state) const
{
    return stateTexts[GetActualTextBlockState(ControlStateToDrawState(state))];
}

UIControlBackground *UIButton::GetOrCreateBackground(eButtonDrawState drawState)
{
    if (!GetBackground(drawState))
    {
        UIControlBackground* targetBack = GetActualBackground(drawState);
        ScopedPtr<UIControlBackground> stateBackground( targetBack ? targetBack->Clone() : CreateDefaultBackground() );
        SetBackground(drawState, stateBackground);
    }

    return GetBackground(drawState);
}
    
void UIButton::SetBackground(eButtonDrawState drawState, UIControlBackground *newBackground)
{
    DVASSERT(0 <= drawState && drawState < DRAW_STATE_COUNT);
    
    if (drawState == DRAW_STATE_UNPRESSED)
    {
        SafeRelease(background);
        background = SafeRetain(newBackground);
    }

    SafeRetain(newBackground);
    SafeRelease(stateBacks[drawState]);
    stateBacks[drawState] = newBackground;
    
    selectedBackground = GetActualBackgroundForState(controlState);
}

UIStaticText *UIButton::GetOrCreateTextBlock(eButtonDrawState drawState)
{
    if (!GetTextBlock(drawState))
    {
        UIStaticText* targetTextBlock = GetActualTextBlock(drawState);
        ScopedPtr<UIStaticText> stateTextBlock( targetTextBlock ? targetTextBlock->Clone() : CreateDefaultTextBlock() );
        SetTextBlock(drawState, stateTextBlock);
    }
    return GetTextBlock(drawState);
}


UIButton::eButtonDrawState UIButton::ControlStateToDrawState(int32 state)
{
    if(state & UIControl::STATE_DISABLED)
    {
        return DRAW_STATE_DISABLED;
    }
    else if(state & UIControl::STATE_SELECTED)
    {
        return DRAW_STATE_SELECTED;
    }
    else if(state & UIControl::STATE_PRESSED_INSIDE)
    {
        return DRAW_STATE_PRESSED_INSIDE;
    }
    else if(state & UIControl::STATE_PRESSED_OUTSIDE)
    {
        return DRAW_STATE_PRESSED_OUTSIDE;
    }
    else if(state & UIControl::STATE_HOVER)
    {
        return DRAW_STATE_HOVERED;
    }

    return DRAW_STATE_UNPRESSED;
}

UIControl::eControlState UIButton::DrawStateToControlState( eButtonDrawState state )
{
    return stateArray[state];
}

const String & UIButton::DrawStatePostfix( eButtonDrawState state )
{
    return statePostfix[state];
}

UIButton::eButtonDrawState UIButton::GetStateReplacer(eButtonDrawState drawState)
{
    eButtonDrawState stateReplacer = DRAW_STATE_UNPRESSED;
    switch(drawState)
    {
    case DRAW_STATE_PRESSED_INSIDE: stateReplacer = DRAW_STATE_PRESSED_OUTSIDE; break;
    case DRAW_STATE_SELECTED: stateReplacer = DRAW_STATE_PRESSED_INSIDE; break;
    default: break;
    }

    return stateReplacer;
}

UIButton::eButtonDrawState UIButton::GetActualBackgroundState(eButtonDrawState drawState) const
{
    while(!GetBackground(drawState) && drawState != DRAW_STATE_UNPRESSED)
    {
        drawState = GetStateReplacer(drawState);
    }

    return drawState;
}

UIButton::eButtonDrawState UIButton::GetActualTextBlockState(eButtonDrawState drawState) const
{
    while(!GetTextBlock(drawState) && drawState != DRAW_STATE_UNPRESSED)
    {
        drawState = GetStateReplacer(drawState);
    }

    return drawState;
}

void UIButton::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);

    for (int32 i = 0; i < STATE_COUNT; ++i)
    {
        eButtonDrawState drawState = (eButtonDrawState)i;
        const String &statePostfixes = DrawStatePostfix(drawState);

        const YamlNode * stateSpriteNode = node->Get(Format("stateSprite%s", statePostfixes.c_str()));
        const YamlNode * stateDrawTypeNode = node->Get(Format("stateDrawType%s", statePostfixes.c_str()));
        const YamlNode * stateAlignNode = node->Get(Format("stateAlign%s", statePostfixes.c_str()));
        const YamlNode * colorInheritNode = node->Get(Format("stateColorInherit%s", statePostfixes.c_str()));
        const YamlNode * perPixelAccuracyNode = node->Get(Format("statePerPixelAccuracy%s", statePostfixes.c_str()));
        const YamlNode * colorNode = node->Get(Format("stateColor%s", statePostfixes.c_str()));
        const YamlNode * leftRightStretchCapNode = node->Get(Format("leftRightStretchCap%s", statePostfixes.c_str()));
        const YamlNode * topBottomStretchCapNode = node->Get(Format("topBottomStretchCap%s", statePostfixes.c_str()));
        const YamlNode* marginsNode = node->Get(Format("stateMargins%s", statePostfixes.c_str()));

        if (stateSpriteNode || stateDrawTypeNode || stateAlignNode ||
            colorInheritNode || colorNode || leftRightStretchCapNode ||
            topBottomStretchCapNode || perPixelAccuracyNode || marginsNode)
        {
            RefPtr<UIControlBackground> stateBackground;
            if (drawState == DRAW_STATE_UNPRESSED)
            {
                stateBackground.Set(CreateDefaultBackground());
            }
            else
            {
                stateBackground.Set(GetActualBackground(GetStateReplacer(drawState))->Clone());
            }
            
            SetBackground(drawState, stateBackground.Get());

            if (stateSpriteNode)
            {
                DVASSERT(stateSpriteNode->GetCount() == 3);
                const YamlNode * spriteNode = stateSpriteNode->Get(0);
                const YamlNode * frameNode = stateSpriteNode->Get(1);
                const YamlNode * backgroundModificationNode = NULL;
                if(stateSpriteNode->GetCount() > 2)
                {
                    backgroundModificationNode = stateSpriteNode->Get(2);
                }

                int32 frame = 0;
                if (frameNode)frame = frameNode->AsInt32();
                if (spriteNode)
                {
                    stateBackground->SetSprite(spriteNode->AsString(), frame);
                }
                if (backgroundModificationNode)
                {
                    stateBackground->SetModification(backgroundModificationNode->AsInt32());
                }
            }

            if (stateDrawTypeNode)
            {
                UIControlBackground::eDrawType type = (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(stateDrawTypeNode);
                stateBackground->SetDrawType(type);
            }

            if(leftRightStretchCapNode)
            {
                float32 leftStretchCap = leftRightStretchCapNode->AsFloat();
                stateBackground->SetLeftRightStretchCap(leftStretchCap);
            }

            if(topBottomStretchCapNode)
            {
                float32 topStretchCap = topBottomStretchCapNode->AsFloat();
                stateBackground->SetTopBottomStretchCap(topStretchCap);
            }

            if (stateAlignNode)
            {
                int32 align = loader->GetAlignFromYamlNode(stateAlignNode);
                stateBackground->SetAlign(align);
            }

            if(colorInheritNode)
            {
                UIControlBackground::eColorInheritType type = (UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode);
                stateBackground->SetColorInheritType(type);
            }
            
            if (perPixelAccuracyNode)
            {
                UIControlBackground::ePerPixelAccuracyType type = (UIControlBackground::ePerPixelAccuracyType)loader->GetPerPixelAccuracyTypeFromNode(perPixelAccuracyNode);
                stateBackground->SetPerPixelAccuracyType(type);
            }

            if(colorNode)
            {
                Color color = loader->GetColorFromYamlNode(colorNode);
                stateBackground->SetColor(color);
            }
            
            if (marginsNode)
            {
                UIControlBackground::UIMargins margins(marginsNode->AsVector4());
                stateBackground->SetMargins(&margins);
            }
        }

        const YamlNode * stateFontNode = node->Get(Format("stateFont%s", statePostfixes.c_str()));
        const YamlNode * stateTextAlignNode = node->Get(Format("stateTextAlign%s", statePostfixes.c_str()));
        const YamlNode * stateTextColorNode = node->Get(Format("stateTextcolor%s", statePostfixes.c_str()));
        const YamlNode * stateTextUseRtlAlignNode = node->Get(Format("stateTextUseRtlAlign%s", statePostfixes.c_str()));
        const YamlNode * stateShadowColorNode = node->Get(Format("stateShadowcolor%s", statePostfixes.c_str()));
        const YamlNode * stateShadowOffsetNode = node->Get(Format("stateShadowoffset%s", statePostfixes.c_str()));
        const YamlNode * stateFittingOptionNode = node->Get(Format("stateFittingOption%s", statePostfixes.c_str()));
        const YamlNode * stateTextNode = node->Get(Format("stateText%s", statePostfixes.c_str()));
        const YamlNode * multilineNode = node->Get(Format("stateMultiline%s", statePostfixes.c_str()));
        const YamlNode * multilineBySymbolNode = node->Get(Format("stateMultilineBySymbol%s", statePostfixes.c_str()));
        const YamlNode * textColorInheritTypeNode = node->Get(Format("stateTextColorInheritType%s", statePostfixes.c_str()));
        const YamlNode * stateTextMarginsNode = node->Get(Format("stateTextMargins%s", statePostfixes.c_str()));
        const YamlNode * textPerPixelAccuracyTypeNode = node->Get(Format("stateTextPerPixelAccuracyType%s", statePostfixes.c_str()));
 
        if (stateFontNode || stateTextAlignNode || stateTextColorNode ||
            stateShadowColorNode || stateShadowOffsetNode || stateFittingOptionNode ||
            stateTextNode || multilineNode || multilineBySymbolNode || textColorInheritTypeNode ||
            stateTextMarginsNode || textPerPixelAccuracyTypeNode || stateTextUseRtlAlignNode)
        {
            RefPtr<UIStaticText> stateTextBlock;
            if (drawState == DRAW_STATE_UNPRESSED)
            {
                stateTextBlock.Set(CreateDefaultTextBlock());
            }
            else
            {
                stateTextBlock.Set(GetActualTextBlock(GetStateReplacer(drawState))->Clone());
            }

            SetTextBlock(drawState, stateTextBlock.Get());

            if (stateFontNode)
            {
                Font * font = loader->GetFontByName(stateFontNode->AsString());
                if (font)stateTextBlock->SetFont(font);
            }

            if (stateTextAlignNode)
            {
                stateTextBlock->SetTextAlign(loader->GetAlignFromYamlNode(stateTextAlignNode));
            }
			
			if (stateTextUseRtlAlignNode)
            {
                stateTextBlock->SetTextUseRtlAlign(stateTextUseRtlAlignNode->AsBool() ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE);
            }

            if (stateTextColorNode)
            {
                stateTextBlock->SetTextColor(stateTextColorNode->AsColor());
            }

            if (stateShadowColorNode)
            {
                stateTextBlock->SetShadowColor(stateShadowColorNode->AsColor());
            }

            if (stateShadowOffsetNode)
            {
                stateTextBlock->SetShadowOffset(stateShadowOffsetNode->AsVector2());
            }

            if (stateFittingOptionNode)
            {
                stateTextBlock->SetFittingOption(stateFittingOptionNode->AsInt32());
            }

            if (stateTextNode)
            {
                stateTextBlock->SetText(LocalizedString(stateTextNode->AsWString()));
            }
            
            if (textColorInheritTypeNode)
            {
                UIControlBackground::eColorInheritType type = (UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(textColorInheritTypeNode);
                stateTextBlock->GetTextBackground()->SetColorInheritType(type);
                stateTextBlock->GetShadowBackground()->SetColorInheritType(type);
            }
            
            if (textPerPixelAccuracyTypeNode)
            {
                UIControlBackground::ePerPixelAccuracyType type = (UIControlBackground::ePerPixelAccuracyType)loader->GetPerPixelAccuracyTypeFromNode(textPerPixelAccuracyTypeNode);
                stateTextBlock->GetTextBackground()->SetPerPixelAccuracyType(type);
                stateTextBlock->GetShadowBackground()->SetPerPixelAccuracyType(type);
            }

            if (stateTextMarginsNode)
            {
                UIControlBackground::UIMargins textMargins(stateTextMarginsNode->AsVector4());
                stateTextBlock->SetMargins(&textMargins);
            }

            bool multiline = loader->GetBoolFromYamlNode(multilineNode, false);
            bool multilineBySymbol = loader->GetBoolFromYamlNode(multilineBySymbolNode, false);
            stateTextBlock->SetMultiline(multiline, multilineBySymbol);
        }
    }
}

YamlNode * UIButton::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);

    //Remove values of UIControl
    //UIButton has state specific properties
    node->RemoveNodeFromMap("color");
    node->RemoveNodeFromMap("sprite");
    node->RemoveNodeFromMap("frame");
    node->RemoveNodeFromMap("drawType");
    node->RemoveNodeFromMap("colorInherit");
    node->RemoveNodeFromMap("perPixelAccuracy");
    node->RemoveNodeFromMap("align");
    node->RemoveNodeFromMap("leftRightStretchCap");
    node->RemoveNodeFromMap("topBottomStretchCap");
    node->RemoveNodeFromMap("spriteModification");
    node->RemoveNodeFromMap("margins");
    node->RemoveNodeFromMap("textMargins");
    
    ScopedPtr<UIButton> baseControl( new UIButton() );

    //States cycle for values
    for (int32 i = 0; i < STATE_COUNT; ++i)
    {
        eButtonDrawState drawState = (eButtonDrawState)i;
        const String &statePostfixLocal = DrawStatePostfix(drawState);
        const UIControlBackground *stateBackground = GetBackground(drawState);
        if (stateBackground)
        {
            const UIControlBackground *baseStateBackground = NULL;
            if (drawState == DRAW_STATE_UNPRESSED)
                baseStateBackground = baseControl->GetBackground(drawState);
            else
                baseStateBackground = GetActualBackground(GetStateReplacer(drawState));

            String spritePath = Sprite::GetPathString(stateBackground->GetSprite());
            if( spritePath != Sprite::GetPathString(baseStateBackground->GetSprite()) ||
                stateBackground->GetFrame() != baseStateBackground->GetFrame() ||
                stateBackground->GetModification() != baseStateBackground->GetModification() )
            {
                //Create array yamlnode and add it to map
                YamlNode *spriteNode = YamlNode::CreateArrayNode(YamlNode::AR_FLOW_REPRESENTATION);
                spriteNode->Add(spritePath);
                spriteNode->Add(stateBackground->GetFrame());
                spriteNode->Add(stateBackground->GetModification());
                node->AddNodeToMap(Format("stateSprite%s", statePostfixLocal.c_str()), spriteNode);
            }

            //StateDrawType
            UIControlBackground::eDrawType drawType = stateBackground->GetDrawType();
            if (baseStateBackground->GetDrawType() != drawType)
            {
                node->Set(Format("stateDrawType%s", statePostfixLocal.c_str()), loader->GetDrawTypeNodeValue(drawType));
            }
            //leftRightStretchCap
            float32 leftStretchCap = stateBackground->GetLeftRightStretchCap();
            if (baseStateBackground->GetLeftRightStretchCap() != leftStretchCap)
            {
                node->Set(Format("leftRightStretchCap%s", statePostfixLocal.c_str()), leftStretchCap);
            }
            //topBottomStretchCap
            float32 topBottomStretchCap = stateBackground->GetTopBottomStretchCap();
            if (baseStateBackground->GetTopBottomStretchCap() != topBottomStretchCap)
            {
                node->Set(Format("topBottomStretchCap%s", statePostfixLocal.c_str()), topBottomStretchCap);
            }
            //State align
            int32 stateAlign = stateBackground->GetAlign();
            if (baseStateBackground->GetAlign() != stateAlign)
            {
                node->AddNodeToMap(Format("stateAlign%s", statePostfixLocal.c_str()), loader->GetAlignNodeValue(stateAlign));
            }

            // State background color
            const Color &color = stateBackground->GetColor();
            if (baseStateBackground->GetColor() != color)
            {
                VariantType colorVariant(color);
                node->Set(Format("stateColor%s", statePostfixLocal.c_str()), &colorVariant);
            }

            // State color inherittype
            UIControlBackground::eColorInheritType colorInheritType = stateBackground->GetColorInheritType();
            if (baseStateBackground->GetColorInheritType() != colorInheritType)
            {
                node->Set(Format("stateColorInherit%s", statePostfixLocal.c_str()), loader->GetColorInheritTypeNodeValue(colorInheritType));
            }

            // State margins.
            const UIControlBackground::UIMargins* margins = stateBackground->GetMargins();
            if (margins)
            {
                node->Set(Format("stateMargins%s", statePostfixLocal.c_str()), margins->AsVector4());
			}

			// State per pixel accuracy
            UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType = stateBackground->GetPerPixelAccuracyType();
            if (baseStateBackground->GetPerPixelAccuracyType() != perPixelAccuracyType)
            {
                node->Set(Format("statePerPixelAccuracy%s", statePostfixLocal.c_str()), loader->GetPerPixelAccuracyTypeNodeValue(perPixelAccuracyType));
            }
        }

        //State font, state text, text color, shadow color and shadow offset
        const UIStaticText *stateTextBlock = GetTextBlock(drawState);
        if (stateTextBlock)
        {
            RefPtr<UIStaticText> baseStaticText;
            if (drawState == DRAW_STATE_UNPRESSED)
                baseStaticText.Set(CreateDefaultTextBlock());
            else
                baseStaticText = GetActualTextBlock(GetStateReplacer(drawState));

            String fontName = FontManager::Instance()->GetFontName(stateTextBlock->GetFont());
            String baseFontName = FontManager::Instance()->GetFontName(baseStaticText->GetFont());
            if (baseFontName != fontName)
            {
                node->Set(Format("stateFont%s", statePostfixLocal.c_str()), fontName);
            }

            const WideString &text = stateTextBlock->GetText();
            if (baseStaticText->GetText() != text)
            {
                node->Set(Format("stateText%s", statePostfixLocal.c_str()), text);
            }

            bool multiline = stateTextBlock->GetMultiline();
            if (baseStaticText->GetMultiline() != multiline)
            {
                node->Set(Format("stateMultiline%s", statePostfixLocal.c_str()), multiline);
            }

            bool multilineBySymbol = stateTextBlock->GetMultilineBySymbol();
            if (baseStaticText->GetMultilineBySymbol() != multilineBySymbol)
            {
                node->Set(Format("stateMultilineBySymbol%s", statePostfixLocal.c_str()), multilineBySymbol);
            }

            const Color &textColor = stateTextBlock->GetTextColor();
            if (baseStaticText->GetTextColor() != textColor)
            {
                VariantType colorVariant(textColor);
                node->Set(Format("stateTextcolor%s", statePostfixLocal.c_str()), &colorVariant);
            }

            const Color &shadowColor = stateTextBlock->GetShadowColor();
            if( baseStaticText->GetShadowColor() != shadowColor )
            {
                VariantType colorVariant(shadowColor);
                node->Set(Format("stateShadowcolor%s", statePostfixLocal.c_str()), &colorVariant);
            }

            const Vector2 &shadowOffset = stateTextBlock->GetShadowOffset();
            if (baseStaticText->GetShadowOffset() != shadowOffset)
            {
                node->Set(Format("stateShadowoffset%s", statePostfixLocal.c_str()), shadowOffset);
            }

            int32 fittingOption = stateTextBlock->GetFittingOption();
            if (baseStaticText->GetFittingOption() != fittingOption)
            {
                node->Set(Format("stateFittingOption%s", statePostfixLocal.c_str()), fittingOption);
            }

            int32 textAlign = stateTextBlock->GetTextAlign();
            if (baseStaticText->GetTextAlign() != textAlign)
            {
                node->SetNodeToMap(Format("stateTextAlign%s", statePostfixLocal.c_str()), loader->GetAlignNodeValue(textAlign));
            }
			
            bool textUseRtlAlign = stateTextBlock->GetTextUseRtlAlign() == TextBlock::RTL_USE_BY_CONTENT;
            if (baseStaticText->GetTextUseRtlAlign() != stateTextBlock->GetTextUseRtlAlign())
            {
                node->Set(Format("stateTextUseRtlAlign%s", statePostfixLocal.c_str()), textUseRtlAlign);
            }
            
            UIControlBackground::eColorInheritType colorInheritType = stateTextBlock->GetTextBackground()->GetColorInheritType();
            if (baseStaticText->GetTextBackground()->GetColorInheritType() != colorInheritType)
            {
                node->Set(Format("stateTextColorInheritType%s", statePostfixLocal.c_str()), loader->GetColorInheritTypeNodeValue(colorInheritType));
            }

            // State text margins.
            const UIControlBackground::UIMargins* textMargins = stateTextBlock->GetMargins();
            if (textMargins)
            {
                node->Set(Format("stateTextMargins%s", statePostfixLocal.c_str()), textMargins->AsVector4());
            }
            
            UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType = stateTextBlock->GetTextBackground()->GetPerPixelAccuracyType();
            if (baseStaticText->GetTextBackground()->GetPerPixelAccuracyType() != perPixelAccuracyType)
            {
                node->Set(Format("stateTextPerPixelAccuracyType%s", statePostfixLocal.c_str()), loader->GetPerPixelAccuracyTypeNodeValue(perPixelAccuracyType));
            }
        }
    }

    return node;
}

void UIButton::SetTextBlock( eButtonDrawState drawState, UIStaticText * newTextBlock )
{
    SafeRelease(stateTexts[drawState]);
    stateTexts[drawState] = SafeRetain(newTextBlock);
    selectedTextBlock = GetActualTextBlockForState(controlState);
}

void UIButton::UpdateStateTextControlSize()
{
    // Current control rect
    const Rect &rect = GetRect();
    // Update size of texcontrol for each state
    for(int i = 0; i < DRAW_STATE_COUNT; i++)
    {
        if(stateTexts[i])
        {
            stateTexts[i]->SetRect(Rect(0, 0, rect.dx, rect.dy));
        }
    }
}

UIStaticText * UIButton::CreateDefaultTextBlock() const
{
    return new UIStaticText(Rect(Vector2(), GetSize()));
}

int32 UIButton::GetBackgroundComponentsCount() const
{
    return DRAW_STATE_COUNT;
}
    
UIControlBackground *UIButton::GetBackgroundComponent(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    return stateBacks[index];
}
 
UIControlBackground *UIButton::CreateBackgroundComponent(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    UIControlBackground *bg = GetActualBackground((eButtonDrawState) index);
    return bg ? bg->Clone() : CreateDefaultBackground();
}

void UIButton::SetBackgroundComponent(int32 drawState, UIControlBackground *newBackground)
{
    DVASSERT(0 <= drawState && drawState < DRAW_STATE_COUNT);
    SetBackground((eButtonDrawState) drawState, newBackground);
}

String UIButton::GetBackgroundComponentName(int32 index) const
{
    return statePostfix[index];
}

int32 UIButton::GetInternalControlsCount() const
{
    return DRAW_STATE_COUNT;
}
    
UIControl *UIButton::GetInternalControl(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    return stateTexts[index];
}

UIControl *UIButton::CreateInternalControl(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    UIStaticText* targetTextBlock = GetActualTextBlock((eButtonDrawState) index);
    return targetTextBlock ? targetTextBlock->Clone() : CreateDefaultTextBlock();
}
    
void UIButton::SetInternalControl(int32 index, UIControl *control)
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    SetTextBlock((eButtonDrawState) index, DynamicTypeCheck<UIStaticText*>(control));
}
    
String UIButton::GetInternalControlName(int32 index) const
{
    return statePostfix[index];
}

String UIButton::GetInternalControlDescriptions() const
{
    return "Text";
}

};
