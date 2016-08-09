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
#include "Utils/StringUtils.h"
#include "Utils/TextBox.h"

#include <numeric>

#define ENABLE_CLIPBOARD 1
#ifdef ENABLE_CLIPBOARD 
#include "Clipboard/Clipboard.h"
#endif // USE_CLIPBOARD

namespace DAVA
{
static float32 DEFAULT_CURSOR_WIDTH = 1.f;

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
    staticText->GetTextBlock()->SetMeasureEnable(true);
    staticText->SetForceBiDiSupportEnabled(true);
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
    if (!isEditing)
    {
        SetCursorPos(GetTextLength());
        stb->SetSelectionStart(0);
        stb->SetSelectionEnd(0);
        isEditing = true;
        control->OnKeyboardShown(Rect());
    }
}

void TextFieldStbImpl::CloseKeyboard()
{
    if (isEditing)
    {
        isEditing = false;
        control->OnKeyboardHidden();
        selectionRects.clear();
    }
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
    // Size getting from the Font
}

void TextFieldStbImpl::SetText(const WideString& newText)
{
    WideString prevText = text;
    if (prevText != newText)
    {
        uint32 cursorPos = stb->GetCursorPosition();

        SelectAll();
        ignoreKeyPressedDelegate = true;
        if (newText.empty())
        {
            stb->Cut();
        }
        else
        {
            stb->Paste(newText);
        }
        stb->ClearUndoStack();
        ignoreKeyPressedDelegate = false;

        cursorPos = std::min(cursorPos, uint32(newText.length()));
        SetCursorPos(cursorPos);

        UITextFieldDelegate* delegate = control->GetDelegate();
        if (delegate)
        {
            delegate->TextFieldOnTextChanged(control, text, prevText);
        }

        staticText->SetText(control->GetVisibleText(), UIStaticText::NO_REQUIRED_SIZE);

        needRedraw = true;
    }
}

void TextFieldStbImpl::UpdateRect(const Rect&)
{
    // see comment for TextFieldPlatformImpl class above

    if (control == UIControlSystem::Instance()->GetFocusedControl() && isEditing)
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
    }

    if (!needRedraw)
    {
        return;
    }

    staticText->SetText(control->GetVisibleText(), UIStaticText::NO_REQUIRED_SIZE);
    needRedraw = false;

    if (isEditing)
    {
        if (lastCursorPos != stb->GetCursorPosition())
        {
            lastCursorPos = stb->GetCursorPosition();

            UpdateCursor(lastCursorPos, stb->IsInsertMode());
            UpdateOffset(cursorRect + staticTextOffset);
            // Fix cursor position for multiline if end of some line contains many
            // spaces over control size (same behavior in MS Word)
            if (!stb->IsSingleLineMode())
            {
                const Vector2& controlSize = control->GetSize();
                if (cursorRect.x + DEFAULT_CURSOR_WIDTH > controlSize.x)
                {
                    cursorRect.dx = DEFAULT_CURSOR_WIDTH;
                    cursorRect.x = controlSize.x - cursorRect.dx - 1.f;
                }
                else if (cursorRect.x + cursorRect.dx > controlSize.x)
                {
                    cursorRect.dx = controlSize.x - cursorRect.x - 1.f;
                }
            }

            // Show cursor again
            cursorTime = 0.f;
            showCursor = true;
        }

        if (lastSelStart != stb->GetSelectionStart() || lastSelEnd != stb->GetSelectionEnd())
        {
            lastSelStart = stb->GetSelectionStart();
            lastSelEnd = stb->GetSelectionEnd();
            UpdateSelection(lastSelStart, lastSelEnd);
        }
    }
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

void TextFieldStbImpl::GetText(WideString& output)
{
    output = text;
}

void TextFieldStbImpl::SetInputEnabled(bool, bool hierarchic /*= true*/)
{
}

void TextFieldStbImpl::SetVisible(bool v)
{
}

void TextFieldStbImpl::SetFont(Font* f)
{
    DropLastCursorAndSelection();
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
    DropLastCursorAndSelection();
    staticText->SetTextAlign(align);
}

TextBlock::eUseRtlAlign TextFieldStbImpl::GetTextUseRtlAlign()
{
    return staticText->GetTextUseRtlAlign();
}

void TextFieldStbImpl::SetTextUseRtlAlign(TextBlock::eUseRtlAlign align)
{
    DropLastCursorAndSelection();
    staticText->SetTextUseRtlAlign(align);
}

void TextFieldStbImpl::SetSize(const Vector2 vector2)
{
    staticText->SetSize(vector2);
}

void TextFieldStbImpl::SetMultiline(bool is_multiline)
{
    DropLastCursorAndSelection();
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
    return staticText->GetTextVisualAlign();
}

void TextFieldStbImpl::SetRect(const Rect& rect)
{
    DropLastCursorAndSelection();
    staticText->SetSize(rect.GetSize());
}

void TextFieldStbImpl::SystemDraw(const UIGeometricData& d)
{
    Rect clipRect = d.GetUnrotatedRect();
    RenderSystem2D::Instance()->PushClip();
    RenderSystem2D::Instance()->IntersectClipRect(clipRect);

    const Vector2& scale = d.scale;
    const Vector2& offset = d.GetUnrotatedRect().GetPosition();

    for (const Rect& r : selectionRects)
    {
        Rect sr = r;
        sr.x += staticTextOffset.x;
        sr.y += staticTextOffset.y;
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
        sr.x += staticTextOffset.x;
        sr.y += staticTextOffset.y;
        sr.x *= scale.x;
        sr.y *= scale.y;
        sr.dx *= scale.x;
        sr.dy *= scale.y;
        RenderSystem2D::Instance()->FillRect(sr + offset, staticText->GetTextColor());
    }

    RenderSystem2D::Instance()->PopClip();
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
    WideString insertText(str, length);
    UITextFieldDelegate* delegate = control->GetDelegate();
    bool apply = true;
    if (!ignoreKeyPressedDelegate && delegate)
    {
        apply = delegate->TextFieldKeyPressed(control, position, 0, insertText);
    }
    if (apply)
    {
        // Additional check if somebody in delegate TextFieldKeyPressed change text manually
        if (position > uint32(text.length()))
        {
            position = uint32(text.length());
        }

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
        DropLastCursorAndSelection();
        return length;
    }
    return 0;
}

uint32 TextFieldStbImpl::DeleteText(uint32 position, uint32 length)
{
    UITextFieldDelegate* delegate = control->GetDelegate();
    bool apply = true;
    if (!ignoreKeyPressedDelegate && delegate)
    {
        WideString str;
        apply = delegate->TextFieldKeyPressed(control, position, length, str);
    }
    if (apply)
    {
        text.erase(position, length);
        DropLastCursorAndSelection();
        return length;
    }
    return 0;
}

const TextBox* TextFieldStbImpl::GetTextBox()
{
    return staticText->GetTextBlock()->GetTextBox();
}

uint32 TextFieldStbImpl::GetTextLength()
{
    return uint32(text.length());
}

WideString::value_type TextFieldStbImpl::GetCharAt(uint32 i)
{
    return text.at(i);
}

void TextFieldStbImpl::CorrectPos(const TextBox* tb, uint32& pos, bool& cursorRight)
{
#if 1 // Simple correcting with RTL check
    const uint32 charsCount = tb->GetCharactersCount();
    if (pos < charsCount)
    {
        while (pos > 0 && tb->GetCharacter(pos).skip)
        {
            pos--;
        }
        cursorRight = tb->GetCharacter(pos).rtl;
    }
    else
    {
        pos = charsCount - 1;
        cursorRight = !tb->GetCharacter(pos).rtl;
    }
#else // Correct cursor like notepad (more complex)
    int32 charsCount = tb->GetCharactersCount();
    if (pos < charsCount)
    {
        while (pos > 0 && tb->GetCharacter(pos).skip)
        {
            pos--;
        }
        cursorRight = tb->GetCharacter(pos).rtl;
        if ((pos > 0) && (tb->GetCharacter(pos - 1).rtl != tb->GetCharacter(pos).rtl))
        {
            pos--;
            cursorRight = !tb->GetCharacter(pos).rtl;
        }
    }
    else
    {
        pos = charsCount - 1;
        cursorRight = !tb->GetCharacter(pos).rtl;
    }
#endif
}

void TextFieldStbImpl::UpdateSelection(uint32 start, uint32 end)
{
    selectionRects.clear();
    uint32 selStart = std::min(start, end);
    uint32 selEnd = std::max(start, end);
    if (selStart < selEnd)
    {
        const TextBox* tb = staticText->GetTextBlock()->GetTextBox();
        for (uint32 i = selStart; i < selEnd; ++i)
        {
            const TextBox::Character& c = tb->GetCharacter(i);
            if (c.skip)
            {
                continue;
            }
            const TextBox::Line& line = tb->GetLine(c.lineIndex);
            if (line.skip)
            {
                continue;
            }

            Rect r;
            r.x = c.xoffset + line.xoffset;
            r.y = line.yoffset;
            r.dx = c.xadvance;
            r.dy = line.yadvance;

            selectionRects.push_back(r);
        }
    }
}

void TextFieldStbImpl::UpdateCursor(uint32 cursorPos, bool insertMode)
{
    const TextBox* tb = staticText->GetTextBlock()->GetTextBox();
    const Vector<int32> linesSizes = staticText->GetTextBlock()->GetStringSizes();

    Rect r;
    r.dx = DEFAULT_CURSOR_WIDTH;

    int32 charsCount = tb->GetCharactersCount();
    if (charsCount > 0)
    {
        bool atEnd = false;
        CorrectPos(tb, cursorPos, atEnd);

        const TextBox::Character& c = tb->GetCharacter(cursorPos);
        const TextBox::Line& line = tb->GetLine(c.lineIndex);

        r.x += c.xoffset + line.xoffset;
        if (atEnd)
        {
            r.x += c.xadvance;
        }
        r.y += line.yoffset;
        r.dy = line.yadvance;
    }
    else
    {
        r.dy = GetFont() ? GetFont()->GetFontHeight() : 0.f;

        int32 ctrlAlign = control->GetTextAlign();
        if (ctrlAlign & ALIGN_RIGHT)
        {
            r.x += control->GetSize().x - r.dx;
        }
        else if (ctrlAlign & ALIGN_HCENTER)
        {
            r.x += (control->GetSize().x - r.dx) * 0.5f;
        }

        if (ctrlAlign & ALIGN_BOTTOM)
        {
            r.y += control->GetSize().y - r.dy;
        }
        else if (ctrlAlign & ALIGN_VCENTER)
        {
            r.y += (control->GetSize().y - r.dy) * 0.5f;
        }
    }

    cursorRect = r;
}

void TextFieldStbImpl::UpdateOffset(const Rect& visibleRect)
{
    const Vector2& controlSize = control->GetSize();
    const Vector2& textSize = staticText->GetTextSize();
    const Vector2 offsetMoveDelta = controlSize * 0.25f;

    if (controlSize.dx < textSize.dx && stb->IsSingleLineMode())
    {
        if (visibleRect.x < 0.f)
        {
            staticTextOffset.x += -visibleRect.x + offsetMoveDelta.x;
        }
        else if (visibleRect.x + visibleRect.dx > controlSize.dx)
        {
            staticTextOffset.x += controlSize.dx - visibleRect.x - visibleRect.dx - offsetMoveDelta.x;
        }
        staticTextOffset.x = std::max(std::min(0.f, staticTextOffset.x), controlSize.dx - textSize.dx - visibleRect.dx - 1.f);
    }
    else
    {
        staticTextOffset.x = 0.f;
    }

    if (controlSize.dy < textSize.dy && !stb->IsSingleLineMode())
    {
        if (visibleRect.y < 0.f)
        {
            staticTextOffset.y += -visibleRect.y + offsetMoveDelta.y;
        }
        else if (visibleRect.y + visibleRect.dy > controlSize.dy)
        {
            staticTextOffset.y += controlSize.dy - visibleRect.y - visibleRect.dy - offsetMoveDelta.y;
        }
        staticTextOffset.y = std::max(std::min(0.f, staticTextOffset.y), controlSize.dy - textSize.dy - 1.f);
    }
    else
    {
        staticTextOffset.y = 0.f;
    }
}

void TextFieldStbImpl::Input(UIEvent* currentInput)
{
    if (control != UIControlSystem::Instance()->GetFocusedControl())
        return;

    if (currentInput->phase == UIEvent::Phase::ENDED)
    {
        if (control->GetStartEditPolicy() == UITextField::START_EDIT_BY_USER_REQUEST)
        {
            control->StartEdit();
        }
    }

    if (!control->IsEditing())
        return;

    bool textChanged = false;
    WideString prevText(text);

    if (currentInput->phase == UIEvent::Phase::KEY_DOWN ||
        currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        const KeyboardDevice& kDevice = InputSystem::Instance()->GetKeyboard();
        bool isShift = kDevice.IsKeyPressed(Key::LSHIFT) || kDevice.IsKeyPressed(Key::RSHIFT);
        bool isCtrl = kDevice.IsKeyPressed(Key::LCTRL) || kDevice.IsKeyPressed(Key::RCTRL);
        bool isAlt = kDevice.IsKeyPressed(Key::LALT) || kDevice.IsKeyPressed(Key::RALT);

        if ((currentInput->key == Key::ENTER || currentInput->key == Key::NUMPADENTER) && !isAlt)
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
                stb->SendKey(StbTextEditBridge::KEY_WORDLEFT | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
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
                stb->SendKey(StbTextEditBridge::KEY_WORDRIGHT | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
            else
            {
                stb->SendKey(StbTextEditBridge::KEY_RIGHT | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
            }
        }
        else if (currentInput->key == Key::UP && !stb->IsSingleLineMode()) // Only in multiline text
        {
            stb->SendKey(StbTextEditBridge::KEY_UP | (isShift ? StbTextEditBridge::KEY_SHIFT_MASK : 0));
        }
        else if (currentInput->key == Key::DOWN && !stb->IsSingleLineMode()) // Only in multiline text
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
            textChanged = stb->SendKey(StbTextEditBridge::KEY_DELETE); // Can modify text
        }
        else if (currentInput->key == Key::INSERT)
        {
            stb->SendKey(StbTextEditBridge::KEY_INSERT);
        }
        else if (isCtrl && currentInput->key == Key::KEY_Y)
        {
            textChanged = stb->SendKey(StbTextEditBridge::KEY_REDO); // Can modify text
        }
        else if (isCtrl && currentInput->key == Key::KEY_Z)
        {
            textChanged = stb->SendKey(StbTextEditBridge::KEY_UNDO); // Can modify text
        }
        else if (isCtrl && currentInput->key == Key::KEY_A)
        {
            SelectAll();
        }
#if ENABLE_CLIPBOARD
        else if (currentInput->key == Key::KEY_X && isCtrl)
        {
            textChanged = CutToClipboard(); // Can modify text
        }
        else if (currentInput->key == Key::KEY_C && isCtrl)
        {
            CopyToClipboard();
        }
        else if (currentInput->key == Key::KEY_V && isCtrl)
        {
            textChanged = PasteFromClipboard(); // Can modify text
        }
#endif
    }
    else if (currentInput->phase == UIEvent::Phase::CHAR ||
             currentInput->phase == UIEvent::Phase::CHAR_REPEAT)
    {
        // Send Enter if it allowed
        if ((currentInput->keyChar == '\r' || currentInput->keyChar == '\n') && control->IsMultiline())
        {
            textChanged = stb->SendKey('\n'); // Can modify text
        }
        // Send backspace
        else if (currentInput->keyChar == '\b')
        {
            textChanged = stb->SendKey(StbTextEditBridge::KEY_BACKSPACE); // Can modify text
        }
#if 0 // Disable TAB for input now
        // Send TAB
        else if (currentInput->keyChar == '\t')
        {
            textChanged = stb->SendKey('\t'); // or SendKey(' '); // Can modify text
        }
#endif
        // Send printable characters (include Font check)
        else if (iswprint(static_cast<wint_t>(currentInput->keyChar))
                 && (control->GetFont() != nullptr && control->GetFont()->IsCharAvaliable(static_cast<char16>(currentInput->keyChar)))
                 )
        {
            textChanged = stb->SendKey(currentInput->keyChar); // Can modify text
        }
    }
    else if (currentInput->phase == UIEvent::Phase::BEGAN)
    {
        Vector2 localPoint = TransformInputPoint(currentInput->point, control->GetAbsolutePosition(), control->GetGeometricData().scale);
        stb->Click(localPoint - staticTextOffset);
        if (currentInput->tapCount == 2)
        {
            stb->SendKey(StbTextEditBridge::KEY_WORDLEFT);
            stb->SendKey(StbTextEditBridge::KEY_WORDRIGHT | StbTextEditBridge::KEY_SHIFT_MASK);
        }
        else if (currentInput->tapCount > 2)
        {
            uint32 length = GetTextLength();
            stb->SetSelectionStart(0);
            stb->SetSelectionEnd(length);
            SetCursorPos(length);
        }
    }
    else if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        Vector2 localPoint = TransformInputPoint(currentInput->point, control->GetAbsolutePosition(), control->GetGeometricData().scale);
        stb->Drag(localPoint - staticTextOffset);
    }

    if (textChanged)
    {
        UITextFieldDelegate* delegate = control->GetDelegate();
        if (delegate)
        {
            delegate->TextFieldOnTextChanged(control, text, prevText);
        }

        staticText->SetText(control->GetVisibleText(), UIStaticText::NO_REQUIRED_SIZE);
        needRedraw = true;
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

void TextFieldStbImpl::SelectAll()
{
    stb->SetSelectionStart(0);
    stb->SetSelectionEnd(GetTextLength());
    stb->SetCursorPosition(GetTextLength());
}

bool TextFieldStbImpl::CutToClipboard()
{
#if ENABLE_CLIPBOARD
    uint32 selStart = std::min(stb->GetSelectionStart(), stb->GetSelectionEnd());
    uint32 selEnd = std::max(stb->GetSelectionStart(), stb->GetSelectionEnd());
    if (selStart < selEnd)
    {
        WideString selectedText = text.substr(selStart, selEnd - selStart);
        if (Clipboard().SetText(selectedText))
        {
            WideString prevText(text);
            if (stb->Cut())
            {
                UITextFieldDelegate* delegate = control->GetDelegate();
                if (delegate)
                {
                    delegate->TextFieldOnTextChanged(control, text, prevText);
                }
                return true;
            }
        }
    }
#endif
    return false;
}

bool TextFieldStbImpl::CopyToClipboard()
{
#if ENABLE_CLIPBOARD
    uint32 selStart = std::min(stb->GetSelectionStart(), stb->GetSelectionEnd());
    uint32 selEnd = std::max(stb->GetSelectionStart(), stb->GetSelectionEnd());
    if (selStart < selEnd)
    {
        WideString selectedText = text.substr(selStart, selEnd - selStart);
        return Clipboard().SetText(selectedText);
    }
#endif
    return false;
}

bool TextFieldStbImpl::PasteFromClipboard()
{
#if ENABLE_CLIPBOARD
    Font* font = control->GetFont();
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
                WideString prevText(text);
                if (stb->Paste(clipText))
                {
                    UITextFieldDelegate* delegate = control->GetDelegate();
                    if (delegate)
                    {
                        delegate->TextFieldOnTextChanged(control, text, prevText);
                    }
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

void TextFieldStbImpl::DropLastCursorAndSelection()
{
    lastCursorPos = INVALID_POS;
    lastSelStart = INVALID_POS;
    lastSelEnd = INVALID_POS;
}
}
