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
#include "UI/UIEvent.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/VariantType.h"
#include "Render/2D/FontManager.h"

namespace DAVA
{
static const UIControl::eControlState stateArray[] = { UIControl::STATE_NORMAL, UIControl::STATE_PRESSED_OUTSIDE, UIControl::STATE_PRESSED_INSIDE, UIControl::STATE_DISABLED, UIControl::STATE_SELECTED, UIControl::STATE_HOVER };
static const String statePostfix[] = { "Normal", "PressedOutside", "PressedInside", "Disabled", "Selected", "Hover" };

UIButton::UIButton(const Rect& rect)
    : UIControl(rect)
    , selectedBackground(NULL)
    , selectedTextBlock(NULL)
    , oldControlState(0)
{
    for (int32 i = 0; i < DRAW_STATE_COUNT; i++)
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
    for (int32 i = 0; i < DRAW_STATE_COUNT; i++)
    {
        SafeRelease(stateBacks[i]);
        SafeRelease(stateTexts[i]);
    }
}

UIButton* UIButton::Clone()
{
    UIButton* b = new UIButton(GetRect());
    b->CopyDataFrom(this);
    return b;
}

void UIButton::CopyDataFrom(UIControl* srcControl)
{
    selectedBackground = NULL;
    selectedTextBlock = NULL;

    UIControl::CopyDataFrom(srcControl);
    UIButton* srcButton = static_cast<UIButton*>(srcControl);
    for (int32 i = 0; i < DRAW_STATE_COUNT; i++)
    {
        eButtonDrawState drawState = static_cast<eButtonDrawState>(i);
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

void UIButton::SetRect(const Rect& rect)
{
    UIControl::SetRect(rect);

    UpdateStateTextControlSize();
}

void UIButton::SetSize(const Vector2& newSize)
{
    UIControl::SetSize(newSize);

    UpdateStateTextControlSize();
}

void UIButton::SetStateSprite(int32 state, const FilePath& spriteName, int32 spriteFrame /* = 0*/)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetSprite(spriteName, spriteFrame);
        }
        state >>= 1;
    }
}

void UIButton::SetStateSprite(int32 state, Sprite* newSprite, int32 spriteFrame /* = 0*/)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetSprite(newSprite, spriteFrame);
        }
        state >>= 1;
    }
}

void UIButton::SetStateFrame(int32 state, int32 spriteFrame)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetFrame(spriteFrame);
        }
        state >>= 1;
    }
}

void UIButton::SetStateDrawType(int32 state, UIControlBackground::eDrawType drawType)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetDrawType(drawType);
        }
        state >>= 1;
    }
}

void UIButton::SetStateAlign(int32 state, int32 align)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetAlign(align);
        }
        state >>= 1;
    }
}

void UIButton::SetStateModification(int32 state, int32 modification)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetModification(modification);
        }
        state >>= 1;
    }
}

void UIButton::SetStateColor(int32 state, Color color)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetColor(color);
        }
        state >>= 1;
    }
}

void UIButton::SetStateColorInheritType(int32 state, UIControlBackground::eColorInheritType value)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetColorInheritType(value);
        }
        state >>= 1;
    }
}

void UIButton::SetStatePerPixelAccuracyType(int32 state, UIControlBackground::ePerPixelAccuracyType value)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetPerPixelAccuracyType(value);
        }
        state >>= 1;
    }
}

void UIButton::CreateBackgroundForState(int32 state)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i));
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

UIControlBackground* UIButton::GetStateBackground(int32 state)
{
    return GetActualBackgroundForState(state);
}

void UIButton::SetStateBackground(int32 state, UIControlBackground* newBackground)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            SetBackground(static_cast<eButtonDrawState>(i), ScopedPtr<UIControlBackground>(newBackground->Clone()));
        }
        state >>= 1;
    }
}

void UIButton::SetStateFont(int32 state, Font* font)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetFont(font);
        }
        state >>= 1;
    }
}

void UIButton::SetStateFontColor(int32 state, const Color& fontColor)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetTextColor(fontColor);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextColorInheritType(int32 state, UIControlBackground::eColorInheritType colorInheritType)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            UIStaticText* staticText = GetOrCreateTextBlock(static_cast<eButtonDrawState>(i));
            staticText->GetTextBackground()->SetColorInheritType(colorInheritType);
            staticText->GetShadowBackground()->SetColorInheritType(colorInheritType);
        }

        state >>= 1;
    }
}

void UIButton::SetStateTextPerPixelAccuracyType(int32 state, UIControlBackground::ePerPixelAccuracyType pixelAccuracyType)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            UIStaticText* staticText = GetOrCreateTextBlock(static_cast<eButtonDrawState>(i));
            staticText->GetTextBackground()->SetPerPixelAccuracyType(pixelAccuracyType);
            staticText->GetShadowBackground()->SetPerPixelAccuracyType(pixelAccuracyType);
        }

        state >>= 1;
    }
}

void UIButton::SetStateShadowColor(int32 state, const Color& shadowColor)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetShadowColor(shadowColor);
        }
        state >>= 1;
    }
}

void UIButton::SetStateShadowOffset(int32 state, const Vector2& offset)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetShadowOffset(offset);
        }
        state >>= 1;
    }
}

void UIButton::SetStateFittingOption(int32 state, int32 fittingOption)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetFittingOption(fittingOption);
        }
        state >>= 1;
    }
}

void UIButton::SetStateText(int32 state, const WideString& text, const Vector2& requestedTextRectSize /* = Vector2(0,0)*/)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetText(text, requestedTextRectSize);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextAlign(int32 state, int32 align)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetTextAlign(align);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextUseRtlAlign(int32 state, TextBlock::eUseRtlAlign value)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetTextUseRtlAlign(value);
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
            UIStaticText* text = GetOrCreateTextBlock(static_cast<eButtonDrawState>(i));
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
            UIStaticText* text = GetOrCreateTextBlock(static_cast<eButtonDrawState>(i));
            text->SetMultiline(text->GetMultiline(), value);
        }
        state >>= 1;
    }
}

void UIButton::SetStateMargins(int32 state, const UIControlBackground::UIMargins* margins)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateBackground(static_cast<eButtonDrawState>(i))->SetMargins(margins);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextMargins(int32 state, const UIControlBackground::UIMargins* margins)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            GetOrCreateTextBlock(static_cast<eButtonDrawState>(i))->SetMargins(margins);
        }
        state >>= 1;
    }
}

void UIButton::SetStateTextControl(int32 state, UIStaticText* textControl)
{
    for (int i = 0; i < DRAW_STATE_COUNT && state; i++)
    {
        if (state & 0x01)
        {
            SetTextBlock(static_cast<eButtonDrawState>(i), ScopedPtr<UIStaticText>(textControl->Clone()));
        }
        state >>= 1;
    }
}

UIStaticText* UIButton::GetStateTextControl(int32 state)
{
    return GetActualTextBlockForState(state);
}

void UIButton::Input(UIEvent* currentInput)
{
    UIControl::Input(currentInput);
    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

void UIButton::SetBackground(UIControlBackground* newBg)
{
    DVASSERT(false);
}

UIControlBackground* UIButton::GetBackground() const
{
    return selectedBackground;
}

void UIButton::SystemDraw(const UIGeometricData& geometricData)
{
    if (oldControlState != controlState)
    {
        oldControlState = controlState;
        selectedTextBlock = GetActualTextBlockForState(controlState);
        selectedBackground = GetActualBackgroundForState(controlState);
    }

    UIControl::SystemDraw(geometricData);
}

void UIButton::Draw(const UIGeometricData& geometricData)
{
    DVASSERT(selectedBackground);
    selectedBackground->Draw(geometricData);

    if (selectedTextBlock)
    {
        selectedTextBlock->Draw(geometricData);
    }
}

void UIButton::SetParentColor(const Color& parentColor)
{
    UIControl::SetParentColor(parentColor);
    DVASSERT(selectedBackground);
    selectedBackground->SetParentColor(parentColor);
    if (selectedTextBlock)
        selectedTextBlock->SetParentColor(selectedBackground->GetDrawColor());
}

UIControlBackground* UIButton::GetActualBackgroundForState(int32 state) const
{
    return GetActualBackground(ControlStateToDrawState(state));
}

UIStaticText* UIButton::GetActualTextBlockForState(int32 state) const
{
    return stateTexts[GetActualTextBlockState(ControlStateToDrawState(state))];
}

UIControlBackground* UIButton::GetOrCreateBackground(eButtonDrawState drawState)
{
    if (!GetBackground(drawState))
    {
        UIControlBackground* targetBack = GetActualBackground(drawState);
        ScopedPtr<UIControlBackground> stateBackground(targetBack ? targetBack->Clone() : CreateDefaultBackground());
        SetBackground(drawState, stateBackground);
    }

    return GetBackground(drawState);
}

void UIButton::SetBackground(eButtonDrawState drawState, UIControlBackground* newBackground)
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

UIStaticText* UIButton::GetOrCreateTextBlock(eButtonDrawState drawState)
{
    if (!GetTextBlock(drawState))
    {
        UIStaticText* targetTextBlock = GetActualTextBlock(drawState);
        ScopedPtr<UIStaticText> stateTextBlock(targetTextBlock ? targetTextBlock->Clone() : CreateDefaultTextBlock());
        SetTextBlock(drawState, stateTextBlock);
    }
    return GetTextBlock(drawState);
}

UIButton::eButtonDrawState UIButton::ControlStateToDrawState(int32 state)
{
    if (state & UIControl::STATE_DISABLED)
    {
        return DRAW_STATE_DISABLED;
    }
    else if (state & UIControl::STATE_SELECTED)
    {
        return DRAW_STATE_SELECTED;
    }
    else if (state & UIControl::STATE_PRESSED_INSIDE)
    {
        return DRAW_STATE_PRESSED_INSIDE;
    }
    else if (state & UIControl::STATE_PRESSED_OUTSIDE)
    {
        return DRAW_STATE_PRESSED_OUTSIDE;
    }
    else if (state & UIControl::STATE_HOVER)
    {
        return DRAW_STATE_HOVERED;
    }

    return DRAW_STATE_UNPRESSED;
}

UIControl::eControlState UIButton::DrawStateToControlState(eButtonDrawState state)
{
    return stateArray[state];
}

const String& UIButton::DrawStatePostfix(eButtonDrawState state)
{
    return statePostfix[state];
}

UIButton::eButtonDrawState UIButton::GetStateReplacer(eButtonDrawState drawState)
{
    eButtonDrawState stateReplacer = DRAW_STATE_UNPRESSED;
    switch (drawState)
    {
    case DRAW_STATE_PRESSED_INSIDE:
        stateReplacer = DRAW_STATE_PRESSED_OUTSIDE;
        break;
    case DRAW_STATE_SELECTED:
        stateReplacer = DRAW_STATE_PRESSED_INSIDE;
        break;
    default:
        break;
    }

    return stateReplacer;
}

UIButton::eButtonDrawState UIButton::GetActualBackgroundState(eButtonDrawState drawState) const
{
    while (!GetBackground(drawState) && drawState != DRAW_STATE_UNPRESSED)
    {
        drawState = GetStateReplacer(drawState);
    }

    return drawState;
}

UIButton::eButtonDrawState UIButton::GetActualTextBlockState(eButtonDrawState drawState) const
{
    while (!GetTextBlock(drawState) && drawState != DRAW_STATE_UNPRESSED)
    {
        drawState = GetStateReplacer(drawState);
    }

    return drawState;
}

void UIButton::SetTextBlock(eButtonDrawState drawState, UIStaticText* newTextBlock)
{
    SafeRelease(stateTexts[drawState]);
    stateTexts[drawState] = SafeRetain(newTextBlock);
    selectedTextBlock = GetActualTextBlockForState(controlState);
}

void UIButton::UpdateStateTextControlSize()
{
    // Current control rect
    const Rect& rect = GetRect();
    // Update size of texcontrol for each state
    for (int i = 0; i < DRAW_STATE_COUNT; i++)
    {
        if (stateTexts[i])
        {
            stateTexts[i]->SetRect(Rect(0, 0, rect.dx, rect.dy));
        }
    }
}

UIStaticText* UIButton::CreateDefaultTextBlock() const
{
    return new UIStaticText(Rect(Vector2(), GetSize()));
}

int32 UIButton::GetBackgroundComponentsCount() const
{
    return DRAW_STATE_COUNT;
}

UIControlBackground* UIButton::GetBackgroundComponent(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    return stateBacks[index];
}

UIControlBackground* UIButton::CreateBackgroundComponent(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    UIControlBackground* bg = GetActualBackground(static_cast<eButtonDrawState>(index));
    return bg ? bg->Clone() : CreateDefaultBackground();
}

void UIButton::SetBackgroundComponent(int32 drawState, UIControlBackground* newBackground)
{
    DVASSERT(0 <= drawState && drawState < DRAW_STATE_COUNT);
    SetBackground(static_cast<eButtonDrawState>(drawState), newBackground);
}

String UIButton::GetBackgroundComponentName(int32 index) const
{
    return statePostfix[index];
}

int32 UIButton::GetInternalControlsCount() const
{
    return DRAW_STATE_COUNT;
}

UIControl* UIButton::GetInternalControl(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    return stateTexts[index];
}

UIControl* UIButton::CreateInternalControl(int32 index) const
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    UIStaticText* targetTextBlock = GetActualTextBlock(static_cast<eButtonDrawState>(index));
    return targetTextBlock ? targetTextBlock->Clone() : CreateDefaultTextBlock();
}

void UIButton::SetInternalControl(int32 index, UIControl* control)
{
    DVASSERT(0 <= index && index < DRAW_STATE_COUNT);
    SetTextBlock(static_cast<eButtonDrawState>(index), DynamicTypeCheck<UIStaticText*>(control));
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
