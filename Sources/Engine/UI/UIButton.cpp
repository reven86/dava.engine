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
#include "Base/ObjectFactory.h"
#include "UI/UIYamlLoader.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/VariantType.h"
#include "Render/2D/FontManager.h"

namespace DAVA
{

    const int32 stateArray[] = {UIControl::STATE_NORMAL, UIControl::STATE_PRESSED_INSIDE, UIControl::STATE_PRESSED_OUTSIDE, UIControl::STATE_DISABLED, UIControl::STATE_SELECTED, UIControl::STATE_HOVER};
    const String statePostfix[] = {"Normal", "PressedInside", "PressedOutside", "Disabled", "Selected", "Hover"};

    UIButton::UIButton(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
    : UIControl(rect, rectInAbsoluteCoordinates)
    {
        oldState = 0;
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            stateBacks[i] = NULL;
            stateTexts[i] = NULL;
        }
        stateBacks[DRAW_STATE_UNPRESSED] = background;
        selectedBackground = background;
        selectedText = NULL;
        exclusiveInput = TRUE;
        SetInputEnabled(true, false);

    }


    UIButton::~UIButton()
    {
        background = stateBacks[DRAW_STATE_UNPRESSED];
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            SafeRelease(stateTexts[i]);
        }
        for(int i = 1; i < DRAW_STATE_COUNT; i++)
        {
            SafeRelease(stateBacks[i]);
        }
        SafeRelease(selectedText);
    }

    UIButton *UIButton::CloneButton()
    {
        return (UIButton *)Clone();
    }


    UIControl *UIButton::Clone()
    {
        UIButton *b = new UIButton(GetRect());
        b->CopyDataFrom(this);
        return b;
    }
    void UIButton::CopyDataFrom(UIControl *srcControl)
    {
        background = stateBacks[DRAW_STATE_UNPRESSED];
        SafeRelease(selectedText);
        for(int i = 1; i < DRAW_STATE_COUNT; i++)
        {
            SafeRelease(stateBacks[i]);
            if(stateTexts[i])
            {
                RemoveControl(stateTexts[i]);
                SafeRelease(stateTexts[i]);
            }
        }

        UIControl::CopyDataFrom(srcControl);
        UIButton *srcButton = (UIButton *)srcControl;
        for(int32 i = 1; i < DRAW_STATE_COUNT; i++)
        {
            if(srcButton->stateBacks[i])
            {
                stateBacks[i] = srcButton->stateBacks[i]->Clone();
            }
        }

        for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(srcButton->stateTexts[i])
            {
                stateTexts[i] = srcButton->stateTexts[i]->CloneStaticText();
            }
        }
        selectedBackground = background;
        oldState = 0;
        stateBacks[DRAW_STATE_UNPRESSED] = background;
    }


    void UIButton::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
    {
        UIControl::SetRect(rect, rectInAbsoluteCoordinates);

        // Have to update all the stateTexts here to update the position of the text for all states.
        // Start loop from zero.
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(stateTexts[i])
            {
                stateTexts[i]->SetRect(Rect(0, 0, rect.dx, rect.dy));
            }
        }
    }

    void UIButton::SetStateSprite(int32 state, const FilePath &spriteName, int32 spriteFrame/* = 0*/)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetSprite(spriteName, spriteFrame);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateSprite(int32 state, Sprite *newSprite, int32 spriteFrame/* = 0*/)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetSprite(newSprite, spriteFrame);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateFrame(int32 state, int32 spriteFrame)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetFrame(spriteFrame);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateDrawType(int32 state, UIControlBackground::eDrawType drawType)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetDrawType(drawType);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateAlign(int32 state, int32 align)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetAlign(align);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateModification(int32 state, int32 modification)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetModification(modification);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateColor(int32 state, Color color)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetColor(color);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::SetStateColorInheritType(int32 state, UIControlBackground::eColorInheritType value)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i)->SetColorInheritType(value);
            }
            state >>= 1;
        }

        selectedBackground = GetActualBackground(controlState);
    }

    void UIButton::CreateBackgroundForState(int32 state)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateBackForState((eButtonDrawState)i);
            }

            state >>= 1;
        }
    }

    Sprite* UIButton::GetStateSprite(int32 state)
    {
        return GetActualBackground(state)->GetSprite();
    }
    int32 UIButton::GetStateFrame(int32 state)
    {
        return GetActualBackground(state)->GetFrame();
    }
    UIControlBackground::eDrawType UIButton::GetStateDrawType(int32 state)
    {
        return GetActualBackground(state)->GetDrawType();
    }
    int32 UIButton::GetStateAlign(int32 state)
    {
        return GetActualBackground(state)->GetAlign();
    }

    UIControlBackground *UIButton::GetStateBackground(int32 state)
    {
        return GetActualBackground(state);
    }


    void UIButton::SetStateBackground(int32 state, UIControlBackground *newBackground)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                SafeRelease(stateBacks[i]);
                stateBacks[i] = newBackground->Clone();
            }
            state >>= 1;
        }
        oldState = 0;
    }


    void UIButton::SetStateFont(int32 state, Font *font)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetFont(font);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateFontColor(int32 state, const Color& fontColor)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetTextColor(fontColor);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateShadowColor(int32 state, const Color& shadowColor)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetShadowColor(shadowColor);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateShadowOffset(int32 state, const Vector2& offset)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetShadowOffset(offset);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateFittingOption(int32 state, int32 fittingOption)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetFittingOption(fittingOption);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateText(int32 state, const WideString &text, const Vector2 &requestedTextRectSize/* = Vector2(0,0)*/)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetText(text, requestedTextRectSize);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateTextAlign(int32 state, int32 align)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                CreateTextForState((eButtonDrawState)i)->SetTextAlign(align);
            }
            state >>= 1;
        }
    }

    void UIButton::SetStateTextControl(int32 state, UIStaticText *textControl)
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(state & 0x01)
            {
                if(stateTexts[i])
                {
                    RemoveControl(stateTexts[i]);
                    SafeRelease(stateTexts[i]);
                }
                stateTexts[i] = textControl->CloneStaticText();
            }
            state >>= 1;
        }
        oldState = 0;
    }

//	FTFont *UIButton::GetStateFont(int32 state)
//	{
//		UIStaticText *tx = GetActualText(state);
//		if(tx)
//		{
//			return tx->GetFont();
//		}
//		return NULL;
//	}
//
//	const WideString &UIButton::GetStateText(int32 state)
//	{
//
//	}

    UIStaticText *UIButton::GetStateTextControl(int32 state)
    {
        return GetActualText(state);
    }



    void UIButton::SystemUpdate(float32 timeElapsed)
    {
        UIControl::SystemUpdate(timeElapsed);

        if(oldState != controlState)
        {
            if(selectedText)
            {
                RemoveControl(selectedText);
                SafeRelease(selectedText);
            }
            selectedText = SafeRetain(GetActualText(controlState));
            if(selectedText)
            {
                AddControl(selectedText);
                BringChildBack(selectedText);
            }

            selectedBackground = GetActualBackground(controlState);

            oldState = controlState;
        }
    }

    void UIButton::SetVisible(bool isVisible, bool hierarchic)
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
        UIControl::SetVisible(isVisible, hierarchic);
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(stateTexts[i])
            {
                stateTexts[i]->SetVisible(isVisible);
            }
        }
        if(selectedText)
        {
            AddControl(selectedText);
            BringChildBack(selectedText);
        }
    }

    void UIButton::SetInputEnabled(bool isEnabled, bool hierarchic)
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
        UIControl::SetInputEnabled(isEnabled, hierarchic);
        if(selectedText)
        {
            AddControl(selectedText);
            BringChildBack(selectedText);
        }
    }
    void UIButton::SetDisabled(bool isDisabled, bool hierarchic)
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
        UIControl::SetDisabled(isDisabled, hierarchic);
        if(selectedText)
        {
            AddControl(selectedText);
            BringChildBack(selectedText);
        }
    }

    void UIButton::SetSelected(bool isSelected, bool hierarchic)
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
        UIControl::SetSelected(isSelected, hierarchic);
        if(selectedText)
        {
            AddControl(selectedText);
            BringChildBack(selectedText);
        }
    }

    void UIButton::SetExclusiveInput(bool isExclusiveInput, bool hierarchic)
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
        UIControl::SetExclusiveInput(isExclusiveInput, hierarchic);
        if(selectedText)
        {
            AddControl(selectedText);
            BringChildBack(selectedText);
        }
    }

    void UIButton::SetMultiInput(bool isMultiInput, bool hierarchic)
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
        UIControl::SetMultiInput(isMultiInput, hierarchic);
        if(selectedText)
        {
            AddControl(selectedText);
            BringChildBack(selectedText);
        }
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


    void UIButton::SetLeftAlign(int32 align)
    {
        UIControl::SetLeftAlign(align);
        UpdateStateTextControlSize();
    }

    void UIButton::SetHCenterAlign(int32 align)
    {
        UIControl::SetHCenterAlign(align);
        UpdateStateTextControlSize();
    }

    void UIButton::SetRightAlign(int32 align)
    {
        UIControl::SetRightAlign(align);
        UpdateStateTextControlSize();
    }

    void UIButton::SetTopAlign(int32 align)
    {
        UIControl::SetTopAlign(align);
        UpdateStateTextControlSize();
    }

    void UIButton::SetVCenterAlign(int32 align)
    {
        UIControl::SetVCenterAlign(align);
        UpdateStateTextControlSize();
    }

    void UIButton::SetBottomAlign(int32 align)
    {
        UIControl::SetBottomAlign(align);
        UpdateStateTextControlSize();
    }

    void UIButton::SystemDraw(const UIGeometricData &geometricData)
    {
        background = selectedBackground;
        UIControl::SystemDraw(geometricData);
        background = stateBacks[DRAW_STATE_UNPRESSED];
    }

    UIControlBackground *UIButton::GetActualBackground(int32 state)
    {
        return stateBacks[BackgroundIndexForState(GetDrawStateForControlState(state))];
    }

    UIStaticText *UIButton::GetActualText(int32 state)
    {
        return stateTexts[TextIndexForState(GetDrawStateForControlState(state))];
    }

    UIControlBackground *UIButton::CreateBackForState(eButtonDrawState buttonState)
    {
        if(stateBacks[buttonState])
        {
            return stateBacks[buttonState];
        }

        UIControlBackground* targetBack = stateBacks[BackgroundIndexForState(buttonState)];
        if(!targetBack)
        {
            stateBacks[buttonState] = new UIControlBackground();
        }
        else
        {
            stateBacks[buttonState] = targetBack->Clone();
        }
        return stateBacks[buttonState];
    }

    UIStaticText *UIButton::CreateTextForState(eButtonDrawState buttonState)
    {
        if(stateTexts[buttonState])
        {
            return stateTexts[buttonState];
        }

        stateTexts[buttonState] = new UIStaticText(Rect(0, 0, size.x, size.y));
        if(!GetVisible())
        {
            stateTexts[buttonState]->SetVisible(false, false);
        }
        return stateTexts[buttonState];
    }


    UIButton::eButtonDrawState UIButton::GetDrawStateForControlState(int32 state)
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

    int32 UIButton::BackgroundIndexForState(eButtonDrawState buttonState)
    {
        if(stateBacks[buttonState])
        {//return current state if dada for state is present
            return buttonState;
        }
        switch (buttonState)
        {//find other state if data for the requested state is absent
            case DRAW_STATE_PRESSED_INSIDE:
            {
                if(stateBacks[DRAW_STATE_PRESSED_OUTSIDE])
                {
                    return DRAW_STATE_PRESSED_OUTSIDE;
                }
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_PRESSED_OUTSIDE:
            {
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_SELECTED:
            {
                if(stateBacks[DRAW_STATE_PRESSED_INSIDE])
                {
                    return DRAW_STATE_PRESSED_INSIDE;
                }
                if(stateBacks[DRAW_STATE_PRESSED_OUTSIDE])
                {
                    return DRAW_STATE_PRESSED_OUTSIDE;
                }
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_DISABLED:
            {
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_HOVERED:
            {
                return DRAW_STATE_UNPRESSED;
            }
                break;
            default:
                break;
        }
        return DRAW_STATE_UNPRESSED;
    }

    int32 UIButton::TextIndexForState(eButtonDrawState buttonState)
    {
        if(stateTexts[buttonState])
        {
            return buttonState;
        }
        switch (buttonState)
        {//find other state if data for the requested state is absent
            case DRAW_STATE_PRESSED_INSIDE:
            {
                if(stateTexts[DRAW_STATE_PRESSED_OUTSIDE])
                {
                    return DRAW_STATE_PRESSED_OUTSIDE;
                }
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_PRESSED_OUTSIDE:
            {
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_SELECTED:
            {
                if(stateTexts[DRAW_STATE_PRESSED_INSIDE])
                {
                    return DRAW_STATE_PRESSED_INSIDE;
                }
                if(stateTexts[DRAW_STATE_PRESSED_OUTSIDE])
                {
                    return DRAW_STATE_PRESSED_OUTSIDE;
                }
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_DISABLED:
            {
                return DRAW_STATE_UNPRESSED;
            }
                break;
            case DRAW_STATE_HOVERED:
            {
                return DRAW_STATE_UNPRESSED;
            }
                break;
            default:
                break;
        }
        return DRAW_STATE_UNPRESSED;
    }

    void UIButton::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
    {
        UIControl::LoadFromYamlNode(node, loader);

        //int32 stateArray[] = {STATE_NORMAL, STATE_PRESSED_INSIDE, STATE_PRESSED_OUTSIDE, STATE_DISABLED, STATE_SELECTED, STATE_HOVER};
        //String statePostfix[] = {"Normal", "PressedInside", "PressedOutside", "Disabled", "Selected", "Hover"};

        for (int k = 0; k < STATE_COUNT; ++k)
        {
            const YamlNode * stateSpriteNode = node->Get(Format("stateSprite%s", statePostfix[k].c_str()));
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
                if (frameNode)frame = frameNode->AsInt();
                if (spriteNode)
                {
                    SetStateSprite(stateArray[k], spriteNode->AsString(), frame);
                }
                if (backgroundModificationNode)
                {
                    stateBacks[k]->SetModification(backgroundModificationNode->AsInt());
                }
            }

            const YamlNode * stateDrawTypeNode = node->Get(Format("stateDrawType%s", statePostfix[k].c_str()));
            if (stateDrawTypeNode)
            {
                UIControlBackground::eDrawType type = (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(stateDrawTypeNode);
                SetStateDrawType(stateArray[k],type);

                const YamlNode * leftRightStretchCapNode = node->Get(Format("leftRightStretchCap%s", statePostfix[k].c_str()));
                const YamlNode * topBottomStretchCapNode = node->Get(Format("topBottomStretchCap%s", statePostfix[k].c_str()));

                if(leftRightStretchCapNode)
                {
                    float32 leftStretchCap = leftRightStretchCapNode->AsFloat();
                    GetActualBackground(stateArray[k])->SetLeftRightStretchCap(leftStretchCap);
                }

                if(topBottomStretchCapNode)
                {
                    float32 topStretchCap = topBottomStretchCapNode->AsFloat();
                    GetActualBackground(stateArray[k])->SetTopBottomStretchCap(topStretchCap);
                }
            }
            else
            {
                SetStateDrawType(stateArray[k],UIControlBackground::DRAW_ALIGNED);
            }

            const YamlNode * stateAlignNode = node->Get(Format("stateAlign%s", statePostfix[k].c_str()));
            if (stateAlignNode)
            {
                int32 align = loader->GetAlignFromYamlNode(stateAlignNode);
                SetStateAlign(stateArray[k],align);
            }

            const YamlNode * stateFontNode = node->Get(Format("stateFont%s", statePostfix[k].c_str()));
            if (stateFontNode)
            {
                Font * font = loader->GetFontByName(stateFontNode->AsString());
                if (font)SetStateFont(stateArray[k], font);
            }

            const YamlNode * stateTextAlignNode = node->Get(Format("stateTextAlign%s", statePostfix[k].c_str()));
            if (stateTextAlignNode)
            {
                SetStateTextAlign(stateArray[k], loader->GetAlignFromYamlNode(stateTextAlignNode));
            }

            const YamlNode * stateTextColorNode = node->Get(Format("stateTextcolor%s", statePostfix[k].c_str()));
            if (stateTextColorNode)
            {
                SetStateFontColor(stateArray[k], stateTextColorNode->AsColor());
            }

            const YamlNode * stateShadowColorNode = node->Get(Format("stateShadowcolor%s", statePostfix[k].c_str()));
            if (stateShadowColorNode)
            {
                SetStateShadowColor(stateArray[k], stateShadowColorNode->AsColor());
            }

            const YamlNode * stateShadowOffsetNode = node->Get(Format("stateShadowoffset%s", statePostfix[k].c_str()));
            if (stateShadowOffsetNode)
            {
                SetStateShadowOffset(stateArray[k], stateShadowOffsetNode->AsVector2());
            }

            const YamlNode * stateFittingOptionNode = node->Get(Format("stateFittingOption%s", statePostfix[k].c_str()));
            if (stateFittingOptionNode)
            {
                SetStateFittingOption(stateArray[k], stateFittingOptionNode->AsInt32());
            }

            const YamlNode * colorInheritNode = node->Get(Format("stateColorInherit%s", statePostfix[k].c_str()));
            if(colorInheritNode)
            {
                UIControlBackground::eColorInheritType type = (UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode);
                GetActualBackground(stateArray[k])->SetColorInheritType(type);
            }

            const YamlNode * colorNode = node->Get(Format("stateColor%s", statePostfix[k].c_str()));
            if(colorNode)
            {
                Color color = loader->GetColorFromYamlNode(colorNode);
                GetActualBackground(stateArray[k])->SetColor(color);
            }

            const YamlNode * stateTextNode = node->Get(Format("stateText%s", statePostfix[k].c_str()));
            if (stateTextNode)
            {
                SetStateText(stateArray[k], LocalizedString(stateTextNode->AsWString()));
            }
        }
    }

    YamlNode * UIButton::SaveToYamlNode(UIYamlLoader * loader)
    {
        YamlNode *node = UIControl::SaveToYamlNode(loader);
        //Temp variable
        VariantType *nodeValue = new VariantType();
        String stringValue;

        UIButton *baseControl = new UIButton();
        UIStaticText *baseStaticText = new UIStaticText();

        //Remove values of UIControl
        //UIButton has state specific properties
        const YamlNode *colorNode = node->Get("color");
        const YamlNode *spriteNode = node->Get("sprite");
        const YamlNode *drawTypeNode = node->Get("drawType");
        const YamlNode *colorInheritNode = node->Get("colorInherit");
        const YamlNode *alignNode = node->Get("align");
        const YamlNode *leftRightStretchCapNode = node->Get("leftRightStretchCap");
        const YamlNode *topBottomStretchCapNode = node->Get("topBottomStretchCap");
        const YamlNode *spriteModificationNode = node->Get("spriteModification");

        if (colorNode)
        {
            node->RemoveNodeFromMap("color");
        }
        if (spriteNode)
        {
            node->RemoveNodeFromMap("sprite");
        }
        if (drawTypeNode)
        {
            node->RemoveNodeFromMap("drawType");
        }
        if (colorInheritNode)
        {
            node->RemoveNodeFromMap("colorInherit");
        }
        if (alignNode)
        {
            node->RemoveNodeFromMap("align");
        }
        if (leftRightStretchCapNode)
        {
            node->RemoveNodeFromMap("leftRightStretchCap");
        }
        if (topBottomStretchCapNode)
        {
            node->RemoveNodeFromMap("topBottomStretchCap");
        }
        if (spriteModificationNode)
        {
            node->RemoveNodeFromMap("spriteModification");
        }

        //States cycle for values
        for (int i = 0; i < STATE_COUNT; ++i)
        {
            //Get sprite and frame for state
            Sprite *stateSprite = this->GetStateSprite(stateArray[i]);
            int32 stateFrame = this->GetStateFrame(stateArray[i]);
            if (stateSprite)
            {
                //Create array yamlnode and add it to map
                YamlNode *spriteNode = new YamlNode(YamlNode::TYPE_ARRAY);

                spriteNode->AddValueToArray(GetSpriteFrameworkPath(stateSprite));
                spriteNode->AddValueToArray(stateFrame);

                int32 modification = stateBacks[BackgroundIndexForState((eButtonDrawState)i)]->GetModification();
                spriteNode->AddValueToArray(modification);
                node->AddNodeToMap(Format("stateSprite%s", statePostfix[i].c_str()), spriteNode);
            }

            //StateDrawType
            UIControlBackground::eDrawType drawType = this->GetStateDrawType(stateArray[i]);
            if (baseControl->GetStateDrawType(stateArray[i]) != drawType)
            {
                node->Set(Format("stateDrawType%s", statePostfix[i].c_str()), loader->GetDrawTypeNodeValue(drawType));
            }
            //leftRightStretchCap
            float32 leftStretchCap = this->GetActualBackground(stateArray[i])->GetLeftRightStretchCap();
            float32 baseLeftStretchCap = baseControl->GetActualBackground(stateArray[i])->GetLeftRightStretchCap();
            if (baseLeftStretchCap != leftStretchCap)
            {
                node->Set(Format("leftRightStretchCap%s", statePostfix[i].c_str()), leftStretchCap);
            }
            //topBottomStretchCap
            float32 topBottomStretchCap = this->GetActualBackground(stateArray[i])->GetTopBottomStretchCap();
            float32 baseTopBottomStretchCap = baseControl->GetActualBackground(stateArray[i])->GetTopBottomStretchCap();
            if (baseTopBottomStretchCap != topBottomStretchCap)
            {
                node->Set(Format("topBottomStretchCap%s", statePostfix[i].c_str()), topBottomStretchCap);
            }
            //State align
            int32 stateAlign = this->GetStateAlign(stateArray[i]);
            int32 baseStateAlign = baseControl->GetStateAlign(stateArray[i]);
            if (baseStateAlign != stateAlign)
            {
                node->AddNodeToMap(Format("stateAlign%s", statePostfix[i].c_str()), loader->GetAlignNodeValue(stateAlign));
            }
            //State font, state text, text color, shadow color and shadow offset
            if (this->GetStateTextControl(stateArray[i]))
            {
                Font *stateFont = this->GetStateTextControl(stateArray[i])->GetFont();
                node->Set(Format("stateFont%s", statePostfix[i].c_str()), FontManager::Instance()->GetFontName(stateFont));

                nodeValue->SetWideString(GetStateTextControl(stateArray[i])->GetText());
                node->Set(Format("stateText%s", statePostfix[i].c_str()), nodeValue);

                const Color &textColor = GetStateTextControl(stateArray[i])->GetTextColor();
                if (baseStaticText->GetTextColor() != textColor)
                {
                    nodeValue->SetColor(textColor);
                    node->Set(Format("stateTextcolor%s", statePostfix[i].c_str()), nodeValue);
                }

                const Color &shadowColor = GetStateTextControl(stateArray[i])->GetShadowColor();
                if( baseStaticText->GetShadowColor() != shadowColor )
                {
                    nodeValue->SetColor(shadowColor);
                    node->Set(Format("stateShadowcolor%s", statePostfix[i].c_str()), nodeValue);
                }

                const Vector2 &shadowOffset = GetStateTextControl(stateArray[i])->GetShadowOffset();
                if (baseStaticText->GetShadowOffset() != shadowOffset)
                {
                    nodeValue->SetVector2(shadowOffset);
                    node->Set(Format("stateShadowoffset%s", statePostfix[i].c_str()), nodeValue);
                }

                int32 fittingOption = GetStateTextControl(stateArray[i])->GetFittingOption();
                if (baseStaticText->GetFittingOption() != fittingOption)
                {
                    nodeValue->SetInt32(fittingOption);
                    node->Set(Format("stateFittingOption%s", statePostfix[i].c_str()), nodeValue);
                }

                int32 textAlign = GetStateTextControl(stateArray[i])->GetTextAlign();
                if (baseStaticText->GetTextAlign() != textAlign)
                {
                    node->SetNodeToMap(Format("stateTextAlign%s", statePostfix[i].c_str()), loader->GetAlignNodeValue(textAlign));
                }
            }

            // State background color
            const Color &color = this->GetActualBackground(stateArray[i])->GetColor();
            const Color &baseColor =  baseControl->GetActualBackground(stateArray[i])->GetColor();
            if (baseColor != color)
            {
                nodeValue->SetColor(color);
                node->Set(Format("stateColor%s", statePostfix[i].c_str()), nodeValue);
            }

            // State color inherittype
            UIControlBackground::eColorInheritType colorInheritType = this->GetActualBackground(stateArray[i])->GetColorInheritType();
            UIControlBackground::eColorInheritType baseColorInheritType = baseControl->GetActualBackground(stateArray[i])->GetColorInheritType();
            if (baseColorInheritType != colorInheritType)
            {
                node->Set(Format("stateColorInherit%s", statePostfix[i].c_str()), loader->GetColorInheritTypeNodeValue(colorInheritType));
            }
        }

        SafeDelete(nodeValue);
        SafeRelease(baseControl);
        SafeRelease(baseStaticText);

        return node;
    }

    List<UIControl* >& UIButton::GetRealChildren()
    {
        List<UIControl* >& realChildren = UIControl::GetRealChildren();
        for (uint32 i = 0; i < DRAW_STATE_COUNT; ++i)
        {
            realChildren.remove(stateTexts[i]);
        }

        return realChildren;
    }

    void UIButton::UpdateStateTextControlSize()
    {
        // Current control rect
        const Rect rect = this->GetRect();
        // Update size of texcontrol for each state
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if(stateTexts[i])
            {
                stateTexts[i]->SetRect(Rect(0, 0, rect.dx, rect.dy));
            }
        }
    }

    void UIButton::SetVisibleForUIEditor(bool value, bool hierarchic/* = true*/)
    {
        UIControl::SetVisibleForUIEditor(value, hierarchic);
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
        {
            if (stateTexts[i])
            {
                stateTexts[i]->SetVisibleForUIEditor(value, hierarchic);
            }
        }
    }
};