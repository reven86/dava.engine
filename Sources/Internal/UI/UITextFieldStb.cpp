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
#include "Utils/UTF8Utils.h"

#include <numeric>

#define ENABLE_CLIPBOARD 1
#ifdef ENABLE_CLIPBOARD 
#include "Clipboard/Clipboard.h"
#endif // USE_CLIPBOARD

namespace DAVA
{
static Vector2 TransformInputPoint(const Vector2& inputPoint, const Vector2& controlAbsPosition, const Vector2& controlScale)
{
    return (inputPoint - controlAbsPosition) / controlScale;
}

TextFieldStbImpl::TextFieldStbImpl(UITextField* control)
    : staticText(new UIStaticText(Rect(Vector2::Zero, control->GetSize())))
    , stb(new StbTextEditBridge(this))
    , control(control)
{
    stb->SetSingleLineMode(true); // Set default because UITextField is single line by default
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
    staticText->SetName("TextFieldStaticText");
}

TextFieldStbImpl::~TextFieldStbImpl()
{
    SafeRelease(staticText);
    SafeDelete(stb);
    control = nullptr;
}

void TextFieldStbImpl::CopyDataFrom(TextFieldStbImpl* t)
{
    stb->CopyStbStateFrom(*t->stb);
    staticText->CopyDataFrom(t->staticText);
    cursorTime = t->cursorTime;
    showCursor = t->showCursor;
}

void TextFieldStbImpl::OpenKeyboard()
{
    // On focus text field
    SetCursorPos(GetTextLength());
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

    UpdateSelection(stb->GetSelectionStart(), stb->GetSelectionEnd());
    UpdateCursor(stb->GetCursorPosition(), stb->IsInsertMode());
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
    return stb->GetCursorPosition();
}

void TextFieldStbImpl::SetCursorPos(int32 position)
{
    stb->SetCursorPosition(position);
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
    stb->SetSingleLineMode(!is_multiline);
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

    const auto& scale = d.scale;
    const auto& offset = d.GetUnrotatedRect().GetPosition();

    for (const auto& r : selectionRects)
    {
        Rect sr = r;
        sr.x *= scale.x;
        sr.y *= scale.y;
        sr.dx *= scale.x;
        sr.dy *= scale.y;
        RenderSystem2D::Instance()->FillRect(sr + offset, selectionColor);
    }

    UIGeometricData staticGeometric = staticText->GetLocalGeometricData();
    staticGeometric.AddGeometricData(d);
    staticGeometric.position += staticTextOffset * scale;
    staticText->SystemDraw(staticGeometric);

    if (showCursor)
    {
        Rect sr = cursorRect;
        sr.x *= scale.x;
        sr.y *= scale.y;
        sr.dx *= scale.x;
        sr.dy *= scale.y;
        RenderSystem2D::Instance()->FillRect(sr + offset, staticText->GetTextColor());
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
    auto delegate = control->GetDelegate();
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
            int32 outOfBounds = int32(text.length()) - control->GetMaxLength() + int32(length);
            if (outOfBounds < 0)
                outOfBounds = 0;
            if (outOfBounds > int32(length))
                outOfBounds = int32(length);
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
    auto delegate = control->GetDelegate();
    bool apply = true;
    if (delegate)
    {
        WideString str;
        apply = delegate->TextFieldKeyPressed(control, position, length, str);
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

uint32 TextFieldStbImpl::GetTextLength()
{
    return uint32(control->GetText().length());
}

WideString::value_type TextFieldStbImpl::GetCharAt(uint32 i)
{
    return control->GetText()[i];
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

            r.x += staticTextOffset.x;
            r.y += staticTextOffset.y;

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
        if (ctrlAlign & ALIGN_LEFT /*|| align & ALIGN_HJUSTIFY*/)
        {
            r.x = 0;
        }
        else if (ctrlAlign & ALIGN_RIGHT)
        {
            r.x = control->GetSize().x - r.dx;
        }
        else //if (ctrlAlign & ALIGN_HCENTER)
        {
            r.x = (control->GetSize().x - r.dx) * 0.5f;
        }

        if (ctrlAlign & ALIGN_TOP)
        {
            r.y = 0;
        }
        else if (ctrlAlign & ALIGN_BOTTOM)
        {
            r.y = control->GetSize().y - r.dy;
        }
        else //if (ctrlAlign & ALIGN_VCENTER)
        {
            r.y = (control->GetSize().y - r.dy) * 0.5f;
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
    if (control != UIControlSystem::Instance()->GetFocusedControl())
        return;

    if (currentInput->phase == UIEvent::Phase::KEY_DOWN ||
        currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        const auto& kDevice = InputSystem::Instance()->GetKeyboard();
        auto isShift = kDevice.IsKeyPressed(Key::LSHIFT) || kDevice.IsKeyPressed(Key::RSHIFT);
        auto isCtrl = kDevice.IsKeyPressed(Key::LCTRL) || kDevice.IsKeyPressed(Key::RCTRL);
        auto isAlt = kDevice.IsKeyPressed(Key::LALT) || kDevice.IsKeyPressed(Key::RALT);
        auto isWin = kDevice.IsKeyPressed(Key::LWIN) || kDevice.IsKeyPressed(Key::RWIN);

        if (currentInput->key == Key::ENTER)
        {
            if (control->GetDelegate())
            {
                control->GetDelegate()->TextFieldShouldReturn(control);
            }
        }
        else if (currentInput->key == Key::ESCAPE)
        {
            if (control->GetDelegate())
            {
                control->GetDelegate()->TextFieldShouldCancel(control);
            }
        }
        else if (currentInput->key == Key::LEFT)
        {
            if (isCtrl)
            {
                stb->SendKey(StbTextEditBridge::KEY_LEFT);
            }
            else
            {
                stb->SendKey(StbTextEditBridge::KEY_LEFT | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
        }
        else if (currentInput->key == Key::RIGHT)
        {
            if (isCtrl)
            {
                stb->SendKey(StbTextEditBridge::KEY_WORDRIGHT);
            }
            else
            {
                stb->SendKey(StbTextEditBridge::KEY_RIGHT | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
        }
        else if (currentInput->key == Key::UP)
        {
            stb->SendKey(StbTextEditBridge::KEY_UP | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
        }
        else if (currentInput->key == Key::DOWN)
        {
            stb->SendKey(StbTextEditBridge::KEY_DOWN | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
        }
        else if (currentInput->key == Key::HOME)
        {
            if (isCtrl)
            {
                stb->SendKey(StbTextEditBridge::KEY_TEXTSTART | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
            else
            {
                stb->SendKey(StbTextEditBridge::KEY_LINESTART | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
        }
        else if (currentInput->key == Key::END)
        {
            if (isCtrl)
            {
                stb->SendKey(StbTextEditBridge::KEY_TEXTEND | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
            else
            {
                stb->SendKey(StbTextEditBridge::KEY_LINEEND | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
        }
        else if (currentInput->key == Key::DELETE)
        {
            stb->SendKey(StbTextEditBridge::KEY_DELETE);
        }
        else if (currentInput->key == Key::INSERT)
        {
            stb->SendKey(StbTextEditBridge::KEY_INSERT);
        }
        else if (isCtrl && currentInput->key == Key::KEY_Y)
        {
            stb->SendKey(StbTextEditBridge::KEY_REDO);
        }
        else if (isCtrl && currentInput->key == Key::KEY_Z)
        {
            stb->SendKey(StbTextEditBridge::KEY_UNDO);
        }
        else if (isCtrl && currentInput->key == Key::KEY_A)
        {
        }
#if ENABLE_CLIPBOARD
        else if (currentInput->key == Key::KEY_X && isCtrl)
        {
            CutToClipboard();
        }
        else if (currentInput->key == Key::KEY_C && isCtrl)
        {
            CopyToClipboard();
        }
        else if (currentInput->key == Key::KEY_V && isCtrl)
        {
            PasteFromClipboard();
        }
#endif
    }
    else if (currentInput->phase == UIEvent::Phase::CHAR ||
             currentInput->phase == UIEvent::Phase::CHAR_REPEAT)
    {
        // Send Enter if it allowed
        if ((currentInput->keyChar == '\r' || currentInput->keyChar == '\n') && control->IsMultiline())
        {
            stb->SendKey('\n');
        }
        // Send backspace
        else if (currentInput->keyChar == '\b')
        {
            stb->SendKey(StbTextEditBridge::KEY_BACKSPACE);
        }
#if 0 // Disable TAB for input now
        // Send TAB
        else if (currentInput->keyChar == '\t')
        {
            stb->SendKey('\t'); // or SendKey(' '); 
        }
#endif
        // Send printable characters (include Font check)
        else if (iswprint(static_cast<wint_t>(currentInput->keyChar))
                 && (control->GetFont() != nullptr && control->GetFont()->IsCharAvaliable(static_cast<char16>(currentInput->keyChar)))
                 )
        {
            stb->SendKey(currentInput->keyChar);
        }
    }
    else if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        auto localPoint = TransformInputPoint(currentInput->point, control->GetAbsolutePosition(), control->GetGeometricData().scale);
        stb->Click(localPoint - staticTextOffset);
    }
    else if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        auto localPoint = TransformInputPoint(currentInput->point, control->GetAbsolutePosition(), control->GetGeometricData().scale);
        stb->Drag(localPoint - staticTextOffset);
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

bool TextFieldStbImpl::CutToClipboard()
{
#if ENABLE_CLIPBOARD
    auto selStart = std::min(stb->GetSelectionStart(), stb->GetSelectionEnd());
    auto selEnd = std::max(stb->GetSelectionStart(), stb->GetSelectionEnd());
    auto selectedText = control->GetText().substr(selStart, selEnd - selStart);
    if (Clipboard().SetText(selectedText))
    {
        stb->Cut();
        return true;
    }
#endif
    return false;
}

bool TextFieldStbImpl::CopyToClipboard()
{
#if ENABLE_CLIPBOARD
    auto selStart = std::min(stb->GetSelectionStart(), stb->GetSelectionEnd());
    auto selEnd = std::max(stb->GetSelectionStart(), stb->GetSelectionEnd());
    auto selectedText = control->GetText().substr(selStart, selEnd - selStart);
#endif
    return Clipboard().SetText(selectedText);
}

bool TextFieldStbImpl::PasteFromClipboard()
{
#if ENABLE_CLIPBOARD
    auto font = control->GetFont();
    // Can't paste any text without font
    if (font != nullptr)
    {
        WideString clipText;
        Clipboard clip;
        if (clip.HasText())
        {
            clipText = clip.GetText();
            // Remove not valid characters (include Font check)
            clipText = StringUtils::RemoveNonPrintable(clipText);
            StringUtils::RemoveEmoji(clipText);
            clipText.erase(std::remove_if(clipText.begin(), clipText.end(), [font](WideString::value_type& ch)
                                          {
                                              return !font->IsCharAvaliable(static_cast<char16>(ch));
                                          }),
                           clipText.end());

            if (!clipText.empty())
            {
                stb->Paste(clipText);
                return true;
            }
        }
    }
#endif
    return false;
}
}
