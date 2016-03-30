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

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* control)
    : staticText(new UIStaticText(Rect(Vector2::Zero, control->GetSize())))
    , control(control)
{
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
}

TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    SafeRelease(staticText);
    control = nullptr;
}

void TextFieldPlatformImpl::CopyDataFrom(TextFieldPlatformImpl* t)
{
    CopyStbStateFrom(*t);
    staticText->CopyDataFrom(t->staticText);
    cursorTime = t->cursorTime;
    showCursor = t->showCursor;
}

void TextFieldPlatformImpl::OpenKeyboard()
{
}

void TextFieldPlatformImpl::CloseKeyboard()
{
}

void TextFieldPlatformImpl::SetRenderToTexture(bool)
{
}

void TextFieldPlatformImpl::SetIsPassword(bool)
{
    needRedraw = true;
}

void TextFieldPlatformImpl::SetFontSize(float32)
{
    // TODO: implement in staticText_->SetFontSize(float32);
}

void TextFieldPlatformImpl::SetText(const WideString& text)
{
    const WideString& prevText = control->GetText();
    if (control->GetDelegate() && prevText != text)
    {
        control->GetDelegate()->TextFieldOnTextChanged(control, text, prevText);
    }
    needRedraw = true;
}

void TextFieldPlatformImpl::UpdateRect(const Rect&)
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
            needRedraw = true;
        }

        auto selStart = std::min(GetSelectionStart(), GetSelectionEnd());
        auto selEnd = std::max(GetSelectionStart(), GetSelectionEnd());
        UpdateSelection(selStart, selEnd);
        UpdateCursor(GetCursor(), IsInsertMode());
    }
    else if (showCursor)
    {
        cursorTime = 0;
        showCursor = false;
        needRedraw = true;
    }

    if (!needRedraw)
    {
        return;
    }

    const WideString& txt = control->GetVisibleText();
    staticText->SetText(txt, UIStaticText::NO_REQUIRED_SIZE);
    needRedraw = false;
}

void TextFieldPlatformImpl::SetAutoCapitalizationType(int32)
{
}

void TextFieldPlatformImpl::SetAutoCorrectionType(int32)
{
}

void TextFieldPlatformImpl::SetSpellCheckingType(int32)
{
}

void TextFieldPlatformImpl::SetKeyboardAppearanceType(int32)
{
}

void TextFieldPlatformImpl::SetKeyboardType(int32)
{
}

void TextFieldPlatformImpl::SetReturnKeyType(int32)
{
}

void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(int32)
{
}

bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return false;
}

uint32 TextFieldPlatformImpl::GetCursorPos() const
{
    return GetCursor();
}

void TextFieldPlatformImpl::SetCursorPos(int32)
{
}

void TextFieldPlatformImpl::SetMaxLength(int32)
{
}

void TextFieldPlatformImpl::GetText(WideString&)
{
}

void TextFieldPlatformImpl::SetInputEnabled(bool, bool hierarchic /*= true*/)
{
}

void TextFieldPlatformImpl::SetVisible(bool v)
{
    staticText->SetVisibilityFlag(v);
}

void TextFieldPlatformImpl::SetFont(Font* f)
{
    staticText->SetFont(f);
}

Font* TextFieldPlatformImpl::GetFont() const
{
    return staticText->GetFont();
}

void TextFieldPlatformImpl::SetTextColor(const Color& c)
{
    staticText->SetTextColor(c);
}

void TextFieldPlatformImpl::SetShadowOffset(const Vector2& v)
{
    staticText->SetShadowOffset(v);
}

void TextFieldPlatformImpl::SetShadowColor(const Color& c)
{
    staticText->SetShadowColor(c);
}

void TextFieldPlatformImpl::SetTextAlign(int32 align)
{
    staticText->SetTextAlign(align);
}

TextBlock::eUseRtlAlign TextFieldPlatformImpl::GetTextUseRtlAlign()
{
    return staticText->GetTextUseRtlAlign();
}

void TextFieldPlatformImpl::SetTextUseRtlAlign(TextBlock::eUseRtlAlign align)
{
    staticText->SetTextUseRtlAlign(align);
}

void TextFieldPlatformImpl::SetSize(const Vector2 vector2)
{
    staticText->SetSize(vector2);
}

void TextFieldPlatformImpl::SetMultiline(bool is_multiline)
{
    staticText->SetMultiline(is_multiline);
}

Color TextFieldPlatformImpl::GetTextColor()
{
    return staticText->GetTextColor();
}

DAVA::Vector2 TextFieldPlatformImpl::GetShadowOffset()
{
    return staticText->GetShadowOffset();
}

Color TextFieldPlatformImpl::GetShadowColor()
{
    return staticText->GetShadowColor();
}

rhi::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return staticText->GetTextAlign();
}

void TextFieldPlatformImpl::SetRect(const Rect& rect)
{
    staticText->SetSize(rect.GetSize());
}

void TextFieldPlatformImpl::SystemDraw(const UIGeometricData& d)
{
    // see comment for TextFieldPlatformImpl class above
    staticText->SystemDraw(d);
}

void TextFieldPlatformImpl::InsertText(uint32 position, const WideString::value_type* str, uint32 length)
{
    WideString t = control->GetText();
    t.insert(position, str, length);
    control->SetText(t);
}

void TextFieldPlatformImpl::DeleteText(uint32 position, uint32 length)
{
    WideString t = control->GetText();
    t.erase(position, length);
    control->SetText(t);
}

const Vector<TextBlock::Line>& TextFieldPlatformImpl::GetMultilineInfo()
{
    return staticText->GetTextBlock()->GetMultilineInfo();
}

const Vector<float32>& TextFieldPlatformImpl::GetCharactersSizes()
{
    return staticText->GetTextBlock()->GetCharactersSize();
}

uint32 TextFieldPlatformImpl::GetLength()
{
    return control->GetText().length();
}

WideString::value_type TextFieldPlatformImpl::GetChar(uint32 i)
{
    return control->GetText()[i];
}

void TextFieldPlatformImpl::SendKey(uint32 codePoint)
{
    if (codePoint == '\r')
    {
        codePoint = control->IsMultiline() ? '\n' : '\0';
    }
    StbTextEditBridge::SendKey(codePoint);
}

void TextFieldPlatformImpl::UpdateSelection(uint32 start, uint32 end)
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
            r.y = line.yoffset;
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

void TextFieldPlatformImpl::UpdateCursor(uint32 cursorPos, bool insertMode)
{
    const auto& linesInfo = staticText->GetTextBlock()->GetMultilineInfo();
    const auto& charsSizes = staticText->GetTextBlock()->GetCharactersSize();

    Rect r;
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

            if (insertMode != 0)
            {
                r.dx = charsSizes[cursorPos];
            }
        }
        else
        {
            auto line = *linesInfo.rbegin();
            r.y = line.yoffset;
            r.dy = line.yadvance;
            r.x = std::accumulate(charsSizes.begin() + line.offset, charsSizes.begin() + line.offset + line.length, 0.f);
            r.x += line.xoffset;
        }

        if (r.x + 1.0f > control->GetSize().x)
        {
            r.dx = 1.0f;
            r.x = control->GetSize().x - r.dx;
        }
        else if (r.x + r.dx > control->GetSize().x)
        {
            r.dx = control->GetSize().x - r.x;
        }
    }
    cursorRect = r;
}

void TextFieldPlatformImpl::DrawSelection(const UIGeometricData& geometricData)
{
    for (const auto& r : selectionRects)
    {
        RenderSystem2D::Instance()->FillRect(r + geometricData.GetUnrotatedRect().GetPosition(), selectionColor);
    }
}

void TextFieldPlatformImpl::DrawCursor(const UIGeometricData& geometricData)
{
    if (showCursor)
    {
        RenderSystem2D::Instance()->DrawRect(cursorRect + geometricData.GetUnrotatedRect().GetPosition(), cursorColor);
    }
}

void TextFieldPlatformImpl::Input(UIEvent* currentInput)
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
        //         else if (currentInput->key == Key::KEY_X && isCtrl)
        //         {
        //             Cut();
        //         }
        //         else if (currentInput->key == Key::KEY_C && isCtrl)
        //         {
        //         }
        //         else if (currentInput->key == Key::KEY_V && isCtrl)
        //         {
        //             Paste(L"test");
        //         }
    }
    else if (currentInput->phase == UIEvent::Phase::CHAR ||
             currentInput->phase == UIEvent::Phase::CHAR_REPEAT)
    {
        SendKey(currentInput->keyChar);
    }
    else if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        Vector2 localPoint = currentInput->point - control->GetPosition();
        Click(localPoint);
    }
    else if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        Vector2 localPoint = currentInput->point - control->GetPosition();
        Drag(localPoint);
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}
}