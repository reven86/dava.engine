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

#include "UITextFieldStb.h"

#include "UI/UITextField.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"
#include "Platform/SystemTimer.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include <numeric>

namespace DAVA
{
TextFieldStbImpl::TextFieldStbImpl(UITextField* control)
    : staticText(new UIStaticText(Rect(Vector2::Zero, control->GetSize())))
    , control(control)
{
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
    staticText->SetName("TextFieldStaticText");
}

TextFieldStbImpl::~TextFieldStbImpl()
{
    SafeRelease(staticText);
    control = nullptr;
}

void TextFieldStbImpl::CopyDataFrom(TextFieldStbImpl* t)
{
    CopyStbStateFrom(*t);
    staticText->CopyDataFrom(t->staticText);
    cursorTime = t->cursorTime;
    showCursor = t->showCursor;
}

void TextFieldStbImpl::OpenKeyboard()
{
    // On focus text field
    //SetCursorPos(GetLength());
}

void TextFieldStbImpl::CloseKeyboard()
{
}

void TextFieldStbImpl::SetRenderToTexture(bool)
{
}

void TextFieldStbImpl::SetIsPassword(bool)
{
    needRedraw = true;
}

void TextFieldStbImpl::SetFontSize(float32 size)
{
    staticText->SetFontSize(size);
}

void TextFieldStbImpl::SetText(const WideString& text)
{
    const WideString& prevText = control->GetText();
    if (control->GetDelegate() && prevText != text)
    {
        control->GetDelegate()->TextFieldOnTextChanged(control, text, prevText);
    }
    //SetCursorPos(text.length());
    needRedraw = true;
}

void TextFieldStbImpl::UpdateRect(const Rect&)
{
    // see comment for TextFieldPlatformImpl class above

    if (control == UIControlSystem::Instance()->GetFocusedControl())
    {
        float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
        cursorTime += timeElapsed;
        if (cursorTime >= 0.5f)
        {
            cursorTime = 0;
            showCursor = !showCursor;
        }
        showCursor = true;
        needRedraw = true;
    }
    else if (showCursor)
    {
        cursorTime = 0;
        showCursor = false;
        needRedraw = true;
        staticTextOffset = Vector2::Zero;
    }

    if (!needRedraw)
    {
        return;
    }

    const WideString& txt = control->GetVisibleText();
    staticText->SetText(txt, UIStaticText::NO_REQUIRED_SIZE);

    UpdateSelection(GetSelectionStart(), GetSelectionEnd());
    UpdateCursor(GetCursor(), IsInsertMode());
    if (!staticText->GetMultiline())
    {
        UpdateOffset(cursorRect);
    }
    else
    {
        staticTextOffset = Vector2::Zero;
    }

    needRedraw = false;
}

void TextFieldStbImpl::SetAutoCapitalizationType(int32)
{
}

void TextFieldStbImpl::SetAutoCorrectionType(int32)
{
}

void TextFieldStbImpl::SetSpellCheckingType(int32)
{
}

void TextFieldStbImpl::SetKeyboardAppearanceType(int32)
{
}

void TextFieldStbImpl::SetKeyboardType(int32)
{
}

void TextFieldStbImpl::SetReturnKeyType(int32)
{
}

void TextFieldStbImpl::SetEnableReturnKeyAutomatically(int32)
{
}

bool TextFieldStbImpl::IsRenderToTexture() const
{
    return false;
}

uint32 TextFieldStbImpl::GetCursorPos() const
{
    return GetCursor();
}

void TextFieldStbImpl::SetCursorPos(int32 position)
{
    SetCursor(position);
}

void TextFieldStbImpl::SetMaxLength(int32 _maxLength)
{
    maxLength = _maxLength;
}

void TextFieldStbImpl::GetText(WideString&)
{
}

void TextFieldStbImpl::SetInputEnabled(bool, bool hierarchic /*= true*/)
{
}

void TextFieldStbImpl::SetVisible(bool v)
{
    //staticText->SetVisibilityFlag(v);
}

void TextFieldStbImpl::SetFont(Font* f)
{
    staticText->SetFont(f);
}

Font* TextFieldStbImpl::GetFont() const
{
    return staticText->GetFont();
}

void TextFieldStbImpl::SetTextColor(const Color& c)
{
    staticText->SetTextColor(c);
}

void TextFieldStbImpl::SetShadowOffset(const Vector2& v)
{
    staticText->SetShadowOffset(v);
}

void TextFieldStbImpl::SetShadowColor(const Color& c)
{
    staticText->SetShadowColor(c);
}

void TextFieldStbImpl::SetTextAlign(int32 align)
{
    staticText->SetTextAlign(align);
}

TextBlock::eUseRtlAlign TextFieldStbImpl::GetTextUseRtlAlign()
{
    return staticText->GetTextUseRtlAlign();
}

void TextFieldStbImpl::SetTextUseRtlAlign(TextBlock::eUseRtlAlign align)
{
    staticText->SetTextUseRtlAlign(align);
}

void TextFieldStbImpl::SetSize(const Vector2 vector2)
{
    staticText->SetSize(vector2);
}

void TextFieldStbImpl::SetMultiline(bool is_multiline)
{
    staticText->SetMultiline(is_multiline);
}

Color TextFieldStbImpl::GetTextColor()
{
    return staticText->GetTextColor();
}

DAVA::Vector2 TextFieldStbImpl::GetShadowOffset()
{
    return staticText->GetShadowOffset();
}

Color TextFieldStbImpl::GetShadowColor()
{
    return staticText->GetShadowColor();
}

rhi::int32 TextFieldStbImpl::GetTextAlign()
{
    return staticText->GetTextAlign();
}

void TextFieldStbImpl::SetRect(const Rect& rect)
{
    staticText->SetSize(rect.GetSize());
}

void TextFieldStbImpl::SystemDraw(const UIGeometricData& d)
{
    Rect clipRect = d.GetUnrotatedRect();
    RenderSystem2D::Instance()->PushClip();
    RenderSystem2D::Instance()->IntersectClipRect(clipRect);

    auto& offset = d.GetUnrotatedRect().GetPosition();

    for (const auto& r : selectionRects)
    {
        RenderSystem2D::Instance()->FillRect(r + offset, selectionColor);
    }

    UIGeometricData staticGeometric = staticText->GetLocalGeometricData();
    staticGeometric.AddGeometricData(d);
    staticGeometric.position += staticTextOffset;
    staticText->SystemDraw(staticGeometric);

    if (showCursor)
    {
        RenderSystem2D::Instance()->FillRect(cursorRect + offset, staticText->GetTextColor());
    }

    RenderSystem2D::Instance()->PopClip();

    //     Rect r;
    //     r.x = 0;
    //     r.y = 0;
    //     r.dx = 3;
    //     r.dy = 3;
    //     RenderSystem2D::Instance()->FillRect(r, Color(1, 0, 1, 1));
    //
    //     r.x = 1;
    //     r.y = 1;
    //     r.dx = 3;
    //     r.dy = 3;
    //     RenderSystem2D::Instance()->DrawRect(r, Color(0, 1, 1, 0.5));
}

void TextFieldStbImpl::SetSelectionColor(const Color& _selectionColor)
{
    selectionColor = _selectionColor;
}

const Color& TextFieldStbImpl::GetSelectionColor() const
{
    return selectionColor;
}

uint32 TextFieldStbImpl::InsertText(uint32 position, const WideString::value_type* str, uint32 length)
{
    auto insertText = WideString(str, length);
    const auto& delegate = control->GetDelegate();
    bool apply = true;
    if (delegate)
    {
        apply = delegate->TextFieldKeyPressed(control, position, 0, insertText);
    }
    if (apply)
    {
        auto text = control->GetText();
        if (control->GetMaxLength() > 0)
        {
            int32 outOfBounds = int32(text.length()) - control->GetMaxLength() + length;
            if (outOfBounds < 0)
                outOfBounds = 0;
            if (outOfBounds > length)
                outOfBounds = length;
            length -= outOfBounds;
        }
        text.insert(position, str, length);
        control->SetText(text);
        return length;
    }
    return 0;
}

uint32 TextFieldStbImpl::DeleteText(uint32 position, uint32 length)
{
    const auto& delegate = control->GetDelegate();
    bool apply = true;
    if (delegate)
    {
        apply = delegate->TextFieldKeyPressed(control, position, length, WideString());
    }
    if (apply)
    {
        auto text = control->GetText();
        text.erase(position, length);
        control->SetText(text);
        return length;
    }
    return 0;
}

const Vector<TextBlock::Line>& TextFieldStbImpl::GetMultilineInfo()
{
    return staticText->GetTextBlock()->GetMultilineInfo();
}

const Vector<float32>& TextFieldStbImpl::GetCharactersSizes()
{
    return staticText->GetTextBlock()->GetCharactersSize();
}

uint32 TextFieldStbImpl::GetLength()
{
    return uint32(control->GetText().length());
}

WideString::value_type TextFieldStbImpl::GetChar(uint32 i)
{
    return control->GetText()[i];
}

void TextFieldStbImpl::SendKey(uint32 codePoint)
{
    if (codePoint == '\r')
    {
        codePoint = control->IsMultiline() ? '\n' : '\0';
    }
    StbTextEditBridge::SendKey(codePoint);
}

void TextFieldStbImpl::UpdateSelection(uint32 start, uint32 end)
{
    selectionRects.clear();
    auto selStart = std::min(start, end);
    auto selEnd = std::max(start, end);
    if (selStart < selEnd)
    {
        const auto& linesInfo = staticText->GetTextBlock()->GetMultilineInfo();
        const auto& charsSizes = staticText->GetTextBlock()->GetCharactersSize();
        for (const auto& line : linesInfo)
        {
            if (selStart >= line.offset + line.length || selEnd <= line.offset)
            {
                continue;
            }

            Rect r;
            r.x = staticTextOffset.x;
            r.y = staticTextOffset.y;
            r.y += line.yoffset;
            r.dy = line.yadvance;
            if (selStart > line.offset)
            {
                r.x += std::accumulate(charsSizes.begin() + line.offset, charsSizes.begin() + selStart, 0.f);
            }

            if (selEnd >= line.offset + line.length)
            {
                r.dx = line.xadvance;
            }
            else
            {
                r.dx = std::accumulate(charsSizes.begin() + line.offset, charsSizes.begin() + selEnd, 0.f);
            }
            r.dx -= r.x;
            r.x += line.xoffset;

            if (r.x > control->GetSize().x)
            {
                continue;
            }
            else if (r.x + r.dx > control->GetSize().x)
            {
                r.dx = control->GetSize().x - r.x;
            }

            selectionRects.push_back(r);
        }
    }
}

void TextFieldStbImpl::UpdateCursor(uint32 cursorPos, bool insertMode)
{
    const auto& linesInfo = staticText->GetTextBlock()->GetMultilineInfo();
    const auto& charsSizes = staticText->GetTextBlock()->GetCharactersSize();

    Rect r;
    r.x = staticTextOffset.x;
    r.y = staticTextOffset.y;
    r.dy = GetFont() ? GetFont()->GetFontHeight() : 0.f;
    r.dx = 1.f;

    if (!linesInfo.empty())
    {
        auto lineInfoIt = std::find_if(linesInfo.begin(), linesInfo.end(), [cursorPos](const DAVA::TextBlock::Line& l)
                                       {
                                           return l.offset <= cursorPos && cursorPos < l.offset + l.length;
                                       });
        if (lineInfoIt != linesInfo.end())
        {
            auto line = *lineInfoIt;
            r.y = line.yoffset;
            r.dy = line.yadvance;
            if (cursorPos != line.offset)
            {
                r.x += std::accumulate(charsSizes.begin() + line.offset, charsSizes.begin() + cursorPos, 0.f);
            }
            r.x += line.xoffset;

#if 0 // Set cursor width like current replacing character
            if (insertMode != 0)
            {
                r.dx = charsSizes[cursorPos];
            }
#endif
        }
        else
        {
            auto line = *linesInfo.rbegin();
            r.y = line.yoffset;
            r.dy = line.yadvance;
            r.x = std::accumulate(charsSizes.begin() + line.offset, charsSizes.begin() + line.offset + line.length, 0.f);
            r.x += line.xoffset;
        }

        if (r.x + 1.f > control->GetSize().x)
        {
            r.dx = 1.0f;
            r.x = control->GetSize().x - r.dx;
        }
        else if (r.x + r.dx > control->GetSize().x)
        {
            r.dx = control->GetSize().x - r.x;
        }
    }
    else
    {
        int32 ctrlAlign = control->GetTextAlign();
        if (ctrlAlign & ALIGN_HCENTER)
        {
            r.x = (control->GetSize().x - r.dx) * 0.5f;
        }
        else if (ctrlAlign & ALIGN_RIGHT)
        {
            r.x = control->GetSize().x - r.dx;
        }

        if (ctrlAlign & ALIGN_VCENTER)
        {
            r.y = (control->GetSize().y - r.dy) * 0.5f;
        }
        else if (ctrlAlign & ALIGN_BOTTOM)
        {
            r.y = control->GetSize().y - r.dy;
        }
    }

    cursorRect = r;
}

void TextFieldStbImpl::UpdateOffset(const Rect& visibleRect)
{
    static float32 checkDelta = 5.f;
    static float32 moveDelta = 30.f;

    const Vector2& controlSize = control->GetSize();
    const Vector2& textSize = staticText->GetTextSize();
    if (controlSize.dx < textSize.dx)
    {
        float32 delta = std::min(moveDelta, textSize.dx - controlSize.dx);
        if (visibleRect.x < checkDelta)
        {
            staticTextOffset.x = std::min(0.f, staticTextOffset.x + delta);
        }
        else if (visibleRect.x > controlSize.dx - checkDelta)
        {
            staticTextOffset.x = std::max(controlSize.dx - textSize.dx, staticTextOffset.x - delta);
        }
        else if (staticTextOffset.x + textSize.dx < controlSize.dx)
        {
            staticTextOffset.x = std::min(0.f, controlSize.x - textSize.dx);
            ;
        }
    }
    else
    {
        staticTextOffset = Vector2::Zero;
    }
}

void TextFieldStbImpl::Input(UIEvent* currentInput)
{
    if (nullptr == control->GetDelegate())
    {
        return;
    }

    if (control != UIControlSystem::Instance()->GetFocusedControl())
        return;

    if (currentInput->phase == UIEvent::Phase::KEY_DOWN ||
        currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        const auto& kDevice = InputSystem::Instance()->GetKeyboard();
        auto isShift = kDevice.IsKeyPressed(Key::LSHIFT) || kDevice.IsKeyPressed(Key::RSHIFT);
        auto isCtrl = kDevice.IsKeyPressed(Key::LCTRL) || kDevice.IsKeyPressed(Key::RCTRL);

        if (currentInput->key == Key::ENTER)
        {
            control->GetDelegate()->TextFieldShouldReturn(control);
        }
        else if (currentInput->key == Key::ESCAPE)
        {
            control->GetDelegate()->TextFieldShouldCancel(control);
        }
        else if (currentInput->key == Key::LEFT)
        {
            if (isCtrl)
            {
                SendKey(STB_TEXTEDIT_K_WORDLEFT);
            }
            else
            {
                SendKey(STB_TEXTEDIT_K_LEFT | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
            }
        }
        else if (currentInput->key == Key::RIGHT)
        {
            if (isCtrl)
            {
                SendKey(STB_TEXTEDIT_K_WORDRIGHT);
            }
            else
            {
                SendKey(STB_TEXTEDIT_K_RIGHT | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
            }
        }
        else if (currentInput->key == Key::UP)
        {
            SendKey(STB_TEXTEDIT_K_UP | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
        }
        else if (currentInput->key == Key::DOWN)
        {
            SendKey(STB_TEXTEDIT_K_DOWN | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
        }
        else if (currentInput->key == Key::HOME)
        {
            if (isCtrl)
            {
                SendKey(STB_TEXTEDIT_K_TEXTSTART | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
            }
            else
            {
                SendKey(STB_TEXTEDIT_K_LINESTART | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
            }
        }
        else if (currentInput->key == Key::END)
        {
            if (isCtrl)
            {
                SendKey(STB_TEXTEDIT_K_TEXTEND | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
            }
            else
            {
                SendKey(STB_TEXTEDIT_K_LINEEND | (isShift ? STB_TEXTEDIT_K_SHIFT : 0));
            }
        }
        else if (currentInput->key == Key::DELETE)
        {
            SendKey(STB_TEXTEDIT_K_DELETE);
        }
        else if (currentInput->key == Key::INSERT)
        {
            SendKey(STB_TEXTEDIT_K_INSERT);
        }
#if 0 // Disable cut/copy/paste keybinding for now
        else if (currentInput->key == Key::KEY_X && isCtrl)
        {
            Cut();
        }
        else if (currentInput->key == Key::KEY_C && isCtrl)
        {
        }
        else if (currentInput->key == Key::KEY_V && isCtrl)
        {
            Paste(L"test");
        }
#endif
    }
    else if (currentInput->phase == UIEvent::Phase::CHAR ||
             currentInput->phase == UIEvent::Phase::CHAR_REPEAT)
    {
        SendKey(currentInput->keyChar);
    }
    else if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        Vector2 localPoint = currentInput->point - control->GetAbsolutePosition() - staticTextOffset;
        Click(localPoint);
    }
    else if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        Vector2 localPoint = currentInput->point - control->GetAbsolutePosition() - staticTextOffset;
        Drag(localPoint);
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}
}
