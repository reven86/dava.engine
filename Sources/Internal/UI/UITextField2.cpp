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


#include "UI/UITextField2.h"
#include "UI/UIControlSystem.h"
#include "UI/UIStaticText.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

// Use NO_REQUIRED_SIZE to notify textFieldImpl->SetText that we don't want
// to enable of any kind of static text fitting
static const DAVA::Vector2 NO_REQUIRED_SIZE = DAVA::Vector2(-1, -1);

////////////////////////////////////////////////////////////////////////////////

#define STB_TEXTEDIT_CHARTYPE DAVA::WideString::value_type
#define STB_TEXTEDIT_STRING StbTextStruct
//#define STB_TEXTEDIT_UNDOSTATECOUNT   99
//#define STB_TEXTEDIT_UNDOCHARCOUNT   999
//#define STB_TEXTEDIT_POSITIONTYPE    int

#include <stb/stb_textedit.h>

struct StbTextStruct
{
    DAVA::UITextField2* field;
    STB_TexteditState state;
};

//    STB_TEXTEDIT_LAYOUTROW(&r,obj,n)  returns the results of laying out a line of characters
//                                        starting from character #n (see discussion below)
inline void layout_func(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i)
{
    int remaining_chars = str->field->GetText().length() - start_i;
    row->num_chars = remaining_chars > 20 ? 20 : remaining_chars; // should do real word wrap here
    row->x0 = 0;
    row->x1 = 20; // need to account for actual size of characters
    row->baseline_y_delta = 1.25;
    row->ymin = -1;
    row->ymax = 0;
}

//    STB_TEXTEDIT_DELETECHARS(obj,i,n)      delete n characters starting at i
inline int delete_chars(STB_TEXTEDIT_STRING* str, int pos, int num)
{
    str->field->DeleteText(pos, num);
    return 1; // always succeeds
}

//    STB_TEXTEDIT_INSERTCHARS(obj,i,c*,n)   insert n characters at i (pointed to by STB_TEXTEDIT_CHARTYPE*)
inline int insert_chars(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num)
{
    str->field->InsertText(pos, newtext, num);
    return 1; // always succeeds
}

//    STB_TEXTEDIT_STRINGLEN(obj)       the length of the string (ideally O(1))
inline int get_length(STB_TEXTEDIT_STRING* str)
{
    return str->field->GetText().length();
}

//    STB_TEXTEDIT_GETWIDTH(obj,n,i)    returns the pixel delta from the xpos of the i'th character
//                                        to the xpos of the i+1'th char for a line of characters
//                                        starting at character #n (i.e. accounts for kerning
//                                        with previous char)
inline int get_width(STB_TEXTEDIT_STRING* str, int n, int i)
{
    return 1;
}

//    STB_TEXTEDIT_KEYTOTEXT(k)         maps a keyboard input to an insertable character
//                                        (return type is int, -1 means not valid to insert)
inline int key_to_text(int key)
{
    return key;
}

//    STB_TEXTEDIT_GETCHAR(obj,i)       returns the i'th character of obj, 0-based
inline int get_char(STB_TEXTEDIT_STRING* str, int i)
{
    return str->field->GetText()[i];
}

// define all the #defines needed 
#define STB_TEXTEDIT_STRINGLEN get_length
#define STB_TEXTEDIT_LAYOUTROW layout_func
#define STB_TEXTEDIT_GETWIDTH get_width
#define STB_TEXTEDIT_KEYTOTEXT key_to_text
#define STB_TEXTEDIT_GETCHAR get_char
#define STB_TEXTEDIT_DELETECHARS delete_chars
#define STB_TEXTEDIT_INSERTCHARS insert_chars
#define STB_TEXTEDIT_IS_SPACE isspace
#define STB_TEXTEDIT_NEWLINE L'\n'

//#define STB_TEXTEDIT_K_CONTROL         0x20000000 //Not required
#define STB_TEXTEDIT_K_SHIFT 0x40000000 //SHIFT MODIFICATOR
#define STB_TEXTEDIT_K_LEFT 0x00010000 //KEY_DOWN(VK_LEFT)
#define STB_TEXTEDIT_K_RIGHT 0x00010001 //KEY_DOWN(VK_RIGHT)
#define STB_TEXTEDIT_K_UP 0x00010002 //KEY_DOWN(VK_UP)
#define STB_TEXTEDIT_K_DOWN 0x00010004 //KEY_DOWN(VK_DOWN)
#define STB_TEXTEDIT_K_LINESTART 0x00010008 //KEY_DOWN(VK_HOME)
#define STB_TEXTEDIT_K_LINEEND 0x00010010 //KEY_DOWN(VK_END)
#define STB_TEXTEDIT_K_TEXTSTART 0x00010020 //KEY_DOWN(VK_HOME + VK_CTRL)
#define STB_TEXTEDIT_K_TEXTEND 0x00010040 //KEY_DOWN(VK_END + VK_CTRL)
#define STB_TEXTEDIT_K_DELETE 0x00010080 //KEY_DOWN(VK_DELETE)
#define STB_TEXTEDIT_K_BACKSPACE 8 //CHAR(8) or KEY_DOWN(VK_BACKSPACE)
#define STB_TEXTEDIT_K_UNDO 26 //CHAR(26) or KEY_DOWN(VK_Z + VK_CTRL)
#define STB_TEXTEDIT_K_REDO 25 //CHAR(25) or KEY_DOWN(VK_Y + VK_CTRL)
#define STB_TEXTEDIT_K_INSERT 0x00010800 //KEY_DOWN(VK_INSERT)
#define STB_TEXTEDIT_K_WORDLEFT 0x00011000 //KEY_DOWN(VK_LEFT + VK_CTRL)
#define STB_TEXTEDIT_K_WORDRIGHT 0x00012000 //KEY_DOWN(VK_RIGHT + VK_CTRL)
#define STB_TEXTEDIT_K_PGUP 0x00014000 //KEY_DOWN(VK_PGUP)
#define STB_TEXTEDIT_K_PGDOWN 0x00018000 //KEY_DOWN(VK_PGDN)

#define STB_TEXTEDIT_IMPLEMENTATION
#include <stb/stb_textedit.h>

////////////////////////////////////////////////////////////////////////////////

namespace DAVA
{
class PlatformKeyboard
{
public:
    void OpenKeyboard()
    {
    }
    void CloseKeyboard()
    {
    }
    void SetAutoCapitalizationType(int32 value)
    {
    }
    void SetAutoCorrectionType(int32 value)
    {
    }
    void SetSpellCheckingType(int32 value)
    {
    }
    void SetKeyboardAppearanceType(int32 value)
    {
    }
    void SetKeyboardType(int32 value)
    {
    }
    void SetReturnKeyType(int32 value)
    {
    }
    void SetEnableReturnKeyAutomatically(bool value)
    {
    }
};

UITextField2::UITextField2(const Rect& rect)
    : UIControl(rect)
    , staticText(new UIStaticText(Rect(Vector2(0, 0), GetSize())))
    , stb_struct(new StbTextStruct())
    , keyboardImpl(new PlatformKeyboard())
{
    stb_struct->field = this;
    stb_textedit_initialize_state(&stb_struct->state, 0);

    AddControl(staticText);
    SetupDefaults();
}

UITextField2::~UITextField2()
{
    SafeRelease(staticText);
    SafeDelete(stb_struct);
    SafeDelete(keyboardImpl);
    UIControl::RemoveAllControls();
}

void UITextField2::SetupDefaults()
{
    // Control props
    SetInputEnabled(true, false);
    // Keyboard props
    SetAutoCapitalizationType(UITextField::AUTO_CAPITALIZATION_TYPE_SENTENCES);
    SetAutoCorrectionType(UITextField::AUTO_CORRECTION_TYPE_DEFAULT);
    SetSpellCheckingType(UITextField::SPELL_CHECKING_TYPE_DEFAULT);
    SetKeyboardAppearanceType(UITextField::KEYBOARD_APPEARANCE_DEFAULT);
    SetKeyboardType(UITextField::KEYBOARD_TYPE_DEFAULT);
    SetReturnKeyType(UITextField::RETURN_KEY_DEFAULT);
    SetEnableReturnKeyAutomatically(false);
    SetTextUseRtlAlign(TextBlock::RTL_DONT_USE);
    // Static text props
    SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    SetFontSize(26);
    // Text field props
    SetMaxLength(-1);
    SetPassword(false);
    SetText(L"");
}

UITextField2* UITextField2::Clone()
{
    UITextField2* t = new UITextField2();
    t->CopyDataFrom(this);
    return t;
}

void UITextField2::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UITextField2* t = static_cast<UITextField2*>(srcControl);

    stb_struct = new StbTextStruct(*t->stb_struct);
    stb_struct->field = this;
    staticText->CopyDataFrom(t->staticText);

    cursorBlinkingTime = t->cursorBlinkingTime;
    cursorTime = t->cursorTime;
    needRedraw = t->needRedraw;
    showCursor = t->showCursor;
    SetPassword(t->GetPassword());
    SetMultiline(t->GetMultiline());
    SetText(t->text);
    SetRect(t->GetRect());

    SetAutoCapitalizationType(t->GetAutoCapitalizationType());
    SetAutoCorrectionType(t->GetAutoCorrectionType());
    SetSpellCheckingType(t->GetSpellCheckingType());
    SetKeyboardAppearanceType(t->GetKeyboardAppearanceType());
    SetKeyboardType(t->GetKeyboardType());
    SetReturnKeyType(t->GetReturnKeyType());
    SetEnableReturnKeyAutomatically(t->IsEnableReturnKeyAutomatically());
    SetTextUseRtlAlign(t->GetTextUseRtlAlign());
    SetMaxLength(t->GetMaxLength());
}

void UITextField2::InsertText(uint32 position, const WideString& str)
{
    WideString t = GetText();
    t.insert(position, str);
    SetText(t);
}

void UITextField2::InsertText(uint32 position, const WideString::value_type* str, uint32 length)
{
    WideString t = GetText();
    t.insert(position, str, length);
    SetText(t);
}

void UITextField2::DeleteText(uint32 position, uint32 length)
{
    WideString t = GetText();
    t.erase(position, length);
    SetText(t);
}

const WideString& UITextField2::GetText() const
{
    return text;
}

void UITextField2::SetText(const WideString& newText)
{
    if (text != newText)
    {
        if (delegate != nullptr)
        {
            delegate->OnTextChanged(this, text, newText);
        }
        text = newText;
        staticText->SetText(text, NO_REQUIRED_SIZE);
    }
}

WideString UITextField2::GetVisibleText() const
{
    return isPassword ? WideString(GetText().length(), L'*') : GetText();
}

bool UITextField2::GetMultiline() const
{
    return isMultiline;
}

void UITextField2::SetMultiline(bool value)
{
    if (value != isMultiline)
    {
        isMultiline = value;
        staticText->SetMultiline(isMultiline);
    }
}

bool UITextField2::GetPassword() const
{
    return isPassword;
}

void UITextField2::SetPassword(bool value)
{
    if (isPassword != value)
    {
        isPassword = value;
        needRedraw = true;
    }
}

UITextField2Delegate* UITextField2::GetDelegate()
{
    return delegate;
}

void UITextField2::SetDelegate(UITextField2Delegate* newDelegate)
{
    delegate = newDelegate;
}

uint32 UITextField2::GetCursorPos()
{
    return 0;
}

void UITextField2::SetCursorPos(uint32 pos)
{
}

int32 UITextField2::GetMaxLength() const
{
    return maxLength;
}

void UITextField2::SetMaxLength(int32 newMaxLength)
{
    maxLength = Max(-1, newMaxLength); //-1 valid value
}

int32 UITextField2::GetAutoCapitalizationType() const
{
    return autoCapitalizationType;
}

void UITextField2::SetAutoCapitalizationType(int32 value)
{
    autoCapitalizationType = static_cast<UITextField::eAutoCapitalizationType>(value);
    keyboardImpl->SetAutoCapitalizationType(autoCapitalizationType);
}

int32 UITextField2::GetAutoCorrectionType() const
{
    return autoCorrectionType;
}

void UITextField2::SetAutoCorrectionType(int32 value)
{
    autoCorrectionType = static_cast<UITextField::eAutoCorrectionType>(value);
    keyboardImpl->SetAutoCorrectionType(autoCorrectionType);
}

int32 UITextField2::GetSpellCheckingType() const
{
    return spellCheckingType;
}

void UITextField2::SetSpellCheckingType(int32 value)
{
    spellCheckingType = static_cast<UITextField::eSpellCheckingType>(value);
    keyboardImpl->SetSpellCheckingType(spellCheckingType);
}

int32 UITextField2::GetKeyboardAppearanceType() const
{
    return keyboardAppearanceType;
}

void UITextField2::SetKeyboardAppearanceType(int32 value)
{
    keyboardAppearanceType = static_cast<UITextField::eKeyboardAppearanceType>(value);
    keyboardImpl->SetKeyboardAppearanceType(keyboardAppearanceType);
}

int32 UITextField2::GetKeyboardType() const
{
    return keyboardType;
}

void UITextField2::SetKeyboardType(int32 value)
{
    keyboardType = static_cast<UITextField::eKeyboardType>(value);
    keyboardImpl->SetKeyboardType(keyboardType);
}

int32 UITextField2::GetReturnKeyType() const
{
    return returnKeyType;
}

void UITextField2::SetReturnKeyType(int32 value)
{
    returnKeyType = static_cast<UITextField::eReturnKeyType>(value);
    keyboardImpl->SetReturnKeyType(returnKeyType);
}

bool UITextField2::IsEnableReturnKeyAutomatically() const
{
    return enableReturnKeyAutomatically;
}

void UITextField2::SetEnableReturnKeyAutomatically(bool value)
{
    enableReturnKeyAutomatically = value;
    keyboardImpl->SetEnableReturnKeyAutomatically(enableReturnKeyAutomatically);
}

void UITextField2::OpenKeyboard()
{
    keyboardImpl->OpenKeyboard();
}

void UITextField2::CloseKeyboard()
{
    keyboardImpl->CloseKeyboard();
}

Font* UITextField2::GetFont() const
{
    return staticText->GetFont();
}

void UITextField2::SetFont(Font* font)
{
    staticText->SetFont(font);
}

String UITextField2::GetFontPresetName() const
{
    return staticText->GetFontPresetName();
}

void UITextField2::SetFontByPresetName(const String& presetName)
{
    staticText->SetFontByPresetName(presetName);
}

void UITextField2::SetFontSize(float32 size)
{
    //TODO: staticText->SetFontSize(size);
}

Color UITextField2::GetTextColor() const
{
    return staticText->GetTextColor();
}

void UITextField2::SetTextColor(const Color& fontColor)
{
    staticText->SetTextColor(fontColor);
}

Vector2 UITextField2::GetShadowOffset() const
{
    return staticText->GetShadowOffset();
}

void UITextField2::SetShadowOffset(const DAVA::Vector2& offset)
{
    staticText->SetShadowOffset(offset);
}

Color UITextField2::GetShadowColor() const
{
    return staticText->GetShadowColor();
}

void UITextField2::SetShadowColor(const Color& color)
{
    staticText->SetShadowColor(color);
}

int32 UITextField2::GetTextAlign() const
{
    return staticText->GetTextAlign();
}

void UITextField2::SetTextAlign(int32 align)
{
    staticText->SetTextAlign(align);
}

TextBlock::eUseRtlAlign UITextField2::GetTextUseRtlAlign() const
{
    return staticText->GetTextUseRtlAlign();
}

void UITextField2::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
    staticText->SetTextUseRtlAlign(useRtlAlign);
}

int32 UITextField2::GetTextUseRtlAlignAsInt() const
{
    return static_cast<TextBlock::eUseRtlAlign>(GetTextUseRtlAlign());
}

void UITextField2::SetTextUseRtlAlignFromInt(int32 value)
{
    SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(value));
}

void UITextField2::OnFocused()
{
    OpenKeyboard();
}

void UITextField2::OnFocusLost(UIControl* newFocus)
{
    CloseKeyboard();
    if (delegate != nullptr)
    {
        delegate->OnLostFocus(this);
    }
}

void UITextField2::Update(float32 timeElapsed)
{
    if (this == UIControlSystem::Instance()->GetFocusedControl())
    {
        //float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
        cursorTime += timeElapsed;

        if (cursorTime >= 0.5f)
        {
            cursorTime = 0;
            showCursor = !showCursor;
            needRedraw = true;
        }
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

    const WideString& txt = this->GetVisibleText();
    if (this == UIControlSystem::Instance()->GetFocusedControl())
    {
        WideString txtWithCursor = txt;
        auto pos = stb_struct->state.cursor;
        txtWithCursor.insert(pos, showCursor ? L"|" : L" ", 1);
        staticText->SetText(txtWithCursor, NO_REQUIRED_SIZE);
    }
    else
    {
        staticText->SetText(txt, NO_REQUIRED_SIZE);
    }
    needRedraw = false;
}

void UITextField2::Input(UIEvent* currentInput)
{
    if (nullptr == delegate)
    {
        return;
    }

    if (this != UIControlSystem::Instance()->GetFocusedControl())
        return;

    if (currentInput->phase == UIEvent::Phase::KEY_DOWN ||
        currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
    {
        if (currentInput->key == Key::ENTER)
        {
            delegate->OnSubmit(this);
        }
        else if (currentInput->key == Key::ESCAPE)
        {
            delegate->OnCancel(this);
        }
        else if (currentInput->key == Key::LEFT)
        {
            stb_textedit_key(stb_struct, &stb_struct->state, STB_TEXTEDIT_K_LEFT);
        }
        else if (currentInput->key == Key::DOWN)
        {
            stb_textedit_key(stb_struct, &stb_struct->state, STB_TEXTEDIT_K_DOWN);
        }
        else if (currentInput->key == Key::RIGHT)
        {
            stb_textedit_key(stb_struct, &stb_struct->state, STB_TEXTEDIT_K_RIGHT);
        }
        else if (currentInput->key == Key::UP)
        {
            stb_textedit_key(stb_struct, &stb_struct->state, STB_TEXTEDIT_K_UP);
        }
        Logger::Error("KEY_DOWN: %d", currentInput->keyChar);
    }
    else if (currentInput->phase == UIEvent::Phase::CHAR ||
             currentInput->phase == UIEvent::Phase::CHAR_REPEAT)
    {
        /*if ('\r' == currentInput->keyChar)
        {
            if (IsMultiline())
            {
                currentInput->keyChar = '\n';
            }
            else
            {
                currentInput->keyChar = '\0';
            }
        }
        if (currentInput->keyChar != 0 && currentInput->keyChar != '\b' && currentInput->keyChar != 0x7f // 0x7f del key (on mac backspace)
            && currentInput->keyChar != 0xf728) // on mac fn+backspace
        {
            WideString str;
            str += currentInput->keyChar;
            int32 length = static_cast<int32>(GetText().length());
            if (delegate->TextFieldKeyPressed(this, length, 0, str))
            {
                SetText(GetAppliedChanges(length, 0, str));
            }
        }*/
        stb_textedit_key(stb_struct, &stb_struct->state, currentInput->keyChar);
        Logger::Error("CHAR: %d", currentInput->keyChar);
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

void UITextField2::SetSize(const DAVA::Vector2& newSize)
{
    UIControl::SetSize(newSize);
    staticText->SetSize(newSize);
}

} // namespace DAVA
