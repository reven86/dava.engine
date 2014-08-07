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
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/YamlNode.h"
#include "Render/2D/FontManager.h"

namespace DAVA
{
static const UIControl::eControlState stateArray[] = {UIControl::STATE_NORMAL, UIControl::STATE_PRESSED_INSIDE, UIControl::STATE_PRESSED_OUTSIDE, UIControl::STATE_DISABLED, UIControl::STATE_SELECTED, UIControl::STATE_HOVER};
static const String statePostfix[] = {"Normal", "PressedInside", "PressedOutside", "Disabled", "Selected", "Hover"};

UIButton::UIButton(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
    : UIControl(rect, rectInAbsoluteCoordinates)
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

void UIButton::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
{
    UIControl::SetRect(rect, rectInAbsoluteCoordinates);

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

UIControlBackground * UIButton::GetBackground()
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
    if (selectedBackground)
        selectedBackground->Draw(geometricData);
    if (selectedTextBlock)
        selectedTextBlock->Draw(geometricData);
}

void UIButton::SetParentColor( const Color &parentColor )
{
    UIControl::SetParentColor(parentColor);
    if (selectedTextBlock)
        selectedTextBlock->SetParentColor(parentColor);
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
        ScopedPtr<UIControlBackground> stateBackground( targetBack ? targetBack->Clone() : new UIControlBackground() );
        SetBackground(drawState, stateBackground);
    }

    return GetBackground(drawState);
}

UIStaticText *UIButton::GetOrCreateTextBlock(eButtonDrawState drawState)
{
    if (!GetTextBlock(drawState))
    {
        UIStaticText* targetTextBlock = GetActualTextBlock(drawState);
        ScopedPtr<UIStaticText> stateTextBlock( targetTextBlock ? targetTextBlock->Clone() : new UIStaticText(Rect(0, 0, size.x, size.y)) );
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
        const String &statePostfix = DrawStatePostfix(drawState);

        const YamlNode * stateSpriteNode = node->Get(Format("stateSprite%s", statePostfix.c_str()));
        const YamlNode * stateDrawTypeNode = node->Get(Format("stateDrawType%s", statePostfix.c_str()));
        const YamlNode * stateAlignNode = node->Get(Format("stateAlign%s", statePostfix.c_str()));
        const YamlNode * colorInheritNode = node->Get(Format("stateColorInherit%s", statePostfix.c_str()));
        const YamlNode * colorNode = node->Get(Format("stateColor%s", statePostfix.c_str()));

        if (stateSpriteNode || stateDrawTypeNode || stateAlignNode ||
            colorInheritNode || colorNode)
        {
            ScopedPtr<UIControlBackground> stateBackground(new UIControlBackground());
            SetBackground(drawState, stateBackground);

            if (stateSpriteNode)
            {
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

                const YamlNode * leftRightStretchCapNode = node->Get(Format("leftRightStretchCap%s", statePostfix.c_str()));
                const YamlNode * topBottomStretchCapNode = node->Get(Format("topBottomStretchCap%s", statePostfix.c_str()));

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

            if(colorNode)
            {
                Color color = loader->GetColorFromYamlNode(colorNode);
                stateBackground->SetColor(color);
            }
        }

        const YamlNode * stateFontNode = node->Get(Format("stateFont%s", statePostfix.c_str()));
        const YamlNode * stateTextAlignNode = node->Get(Format("stateTextAlign%s", statePostfix.c_str()));
        const YamlNode * stateTextColorNode = node->Get(Format("stateTextcolor%s", statePostfix.c_str()));
        const YamlNode * stateShadowColorNode = node->Get(Format("stateShadowcolor%s", statePostfix.c_str()));
        const YamlNode * stateShadowOffsetNode = node->Get(Format("stateShadowoffset%s", statePostfix.c_str()));
        const YamlNode * stateFittingOptionNode = node->Get(Format("stateFittingOption%s", statePostfix.c_str()));
        const YamlNode * stateTextNode = node->Get(Format("stateText%s", statePostfix.c_str()));
        const YamlNode * multilineNode = node->Get(Format("stateMultiline%s", statePostfix.c_str()));
        const YamlNode * multilineBySymbolNode = node->Get(Format("stateMultilineBySymbol%s", statePostfix.c_str()));

        if (stateFontNode || stateTextAlignNode || stateTextColorNode ||
            stateShadowColorNode || stateShadowOffsetNode || stateFittingOptionNode ||
            stateTextNode || multilineNode || multilineBySymbolNode)
        {
            ScopedPtr<UIStaticText> stateTextBlock(new UIStaticText(Rect(0, 0, size.x, size.y)));
            SetTextBlock(drawState, stateTextBlock);

            if (stateFontNode)
            {
                Font * font = loader->GetFontByName(stateFontNode->AsString());
                if (font)stateTextBlock->SetFont(font);
            }

            if (stateTextAlignNode)
            {
                stateTextBlock->SetAlign(loader->GetAlignFromYamlNode(stateTextAlignNode));
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
    node->RemoveNodeFromMap("align");
    node->RemoveNodeFromMap("leftRightStretchCap");
    node->RemoveNodeFromMap("topBottomStretchCap");
    node->RemoveNodeFromMap("spriteModification");

    ScopedPtr<UIButton> baseControl( new UIButton() );
    //States cycle for values
    for (int32 i = 0; i < STATE_COUNT; ++i)
    {
        eButtonDrawState drawState = (eButtonDrawState)i;
        const String &statePostfix = DrawStatePostfix(drawState);
        const UIControlBackground *baseStateBackground = baseControl->GetActualBackground(drawState);

        UIControlBackground *stateBackground = GetBackground(drawState);
        if (stateBackground)
        {
            //Get sprite and frame for state
            Sprite *stateSprite = stateBackground->GetSprite();
            if (stateSprite)
            {
                //Create array yamlnode and add it to map
                YamlNode *spriteNode = new YamlNode(YamlNode::TYPE_ARRAY);

                spriteNode->AddValueToArray(GetSpriteFrameworkPath(stateSprite));
                spriteNode->AddValueToArray(stateBackground->GetFrame());
                spriteNode->AddValueToArray(stateBackground->GetModification());
                node->AddNodeToMap(Format("stateSprite%s", statePostfix.c_str()), spriteNode);
            }

            //StateDrawType
            UIControlBackground::eDrawType drawType = stateBackground->GetDrawType();
            if (baseStateBackground->GetDrawType() != drawType)
            {
                node->Set(Format("stateDrawType%s", statePostfix.c_str()), loader->GetDrawTypeNodeValue(drawType));
            }
            //leftRightStretchCap
            float32 leftStretchCap = stateBackground->GetLeftRightStretchCap();
            if (baseStateBackground->GetLeftRightStretchCap() != leftStretchCap)
            {
                node->Set(Format("leftRightStretchCap%s", statePostfix.c_str()), leftStretchCap);
            }
            //topBottomStretchCap
            float32 topBottomStretchCap = stateBackground->GetTopBottomStretchCap();
            if (baseStateBackground->GetTopBottomStretchCap() != topBottomStretchCap)
            {
                node->Set(Format("topBottomStretchCap%s", statePostfix.c_str()), topBottomStretchCap);
            }
            //State align
            int32 stateAlign = stateBackground->GetAlign();
            if (baseStateBackground->GetAlign() != stateAlign)
            {
                node->AddNodeToMap(Format("stateAlign%s", statePostfix.c_str()), loader->GetAlignNodeValue(stateAlign));
            }

            // State background color
            const Color &color = stateBackground->GetColor();
            if (baseStateBackground->GetColor() != color)
            {
                VariantType colorVariant(color);
                node->Set(Format("stateColor%s", statePostfix.c_str()), &colorVariant);
            }

            // State color inherittype
            UIControlBackground::eColorInheritType colorInheritType = stateBackground->GetColorInheritType();
            if (baseStateBackground->GetColorInheritType() != colorInheritType)
            {
                node->Set(Format("stateColorInherit%s", statePostfix.c_str()), loader->GetColorInheritTypeNodeValue(colorInheritType));
            }
        }

        //State font, state text, text color, shadow color and shadow offset
        const UIStaticText *stateTextBlock = GetTextBlock(drawState);
        if (stateTextBlock)
        {
            ScopedPtr<UIStaticText> baseStaticText( new UIStaticText() );
            Font *stateFont = stateTextBlock->GetFont();
            node->Set(Format("stateFont%s", statePostfix.c_str()), FontManager::Instance()->GetFontName(stateFont));

            const WideString &text = stateTextBlock->GetText();
            if (baseStaticText->GetText() != text)
            {
                node->Set(Format("stateText%s", statePostfix.c_str()), text);
            }

            bool multiline = stateTextBlock->GetMultiline();
            if (baseStaticText->GetMultiline() != multiline)
            {
                node->Set(Format("stateMultiline%s", statePostfix.c_str()), multiline);
            }

            bool multilineBySymbol = stateTextBlock->GetMultilineBySymbol();
            if (baseStaticText->GetMultilineBySymbol() != multilineBySymbol)
            {
                node->Set(Format("stateMultilineBySymbol%s", statePostfix.c_str()), multilineBySymbol);
            }

            const Color &textColor = stateTextBlock->GetTextColor();
            if (baseStaticText->GetTextColor() != textColor)
            {
                VariantType colorVariant(textColor);
                node->Set(Format("stateTextcolor%s", statePostfix.c_str()), &colorVariant);
            }

            const Color &shadowColor = stateTextBlock->GetShadowColor();
            if( baseStaticText->GetShadowColor() != shadowColor )
            {
                VariantType colorVariant(shadowColor);
                node->Set(Format("stateShadowcolor%s", statePostfix.c_str()), &colorVariant);
            }

            const Vector2 &shadowOffset = stateTextBlock->GetShadowOffset();
            if (baseStaticText->GetShadowOffset() != shadowOffset)
            {
                node->Set(Format("stateShadowoffset%s", statePostfix.c_str()), shadowOffset);
            }

            int32 fittingOption = stateTextBlock->GetFittingOption();
            if (baseStaticText->GetFittingOption() != fittingOption)
            {
                node->Set(Format("stateFittingOption%s", statePostfix.c_str()), fittingOption);
            }

            int32 textAlign = stateTextBlock->GetTextAlign();
            if (baseStaticText->GetTextAlign() != textAlign)
            {
                node->SetNodeToMap(Format("stateTextAlign%s", statePostfix.c_str()), loader->GetAlignNodeValue(textAlign));
            }
        }
    }

    return node;
}

void UIButton::SetBackground(eButtonDrawState drawState, UIControlBackground * newBackground)
{
    if (drawState == DRAW_STATE_UNPRESSED)
    {
        SafeRelease(background);
        background = SafeRetain(newBackground);
    }

    SafeRelease(stateBacks[drawState]);
    stateBacks[drawState] = SafeRetain(newBackground);
    selectedBackground = GetActualBackgroundForState(controlState);
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
};