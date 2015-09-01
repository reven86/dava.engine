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

#include "Platform/TemplateWin32/PrivateTextFieldWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "FileSystem/Logger.h"

#include "Math/Color.h"

#include "Concurrency/LockGuard.h"

#include "UI/UITextField.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/ImageConvert.h"

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

#include <ppltasks.h>

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Web;
using namespace concurrency;

namespace DAVA
{

namespace
{

struct StringDiffResult
{
    StringDiffResult() = default;

    enum eDiffType {
        NO_CHANGE = 0,      // No changes between original string and new string
        INSERTION,          // Character range was inserted into new string
        DELETION,           // Character range was deleted from original string
        REPLACEMENT         // Character range in original string was replaced by another character range in new string
    };

    eDiffType diffType = NO_CHANGE;
    int32 originalStringDiffPosition = 0;   // Position in original string where difference starts
    int32 newStringDiffPosition = 0;        // Position in new string where difference starts
    WideString originalStringDiff;          // What was changed in original string
    WideString newStringDiff;               // What was changed in new string

    // Explanation of originalStringDiff, newStringDiff and diff positions:
    // diffType is INSERTION:
    //      original            'text'
    //      new                 'te123xt'
    //      original diff pos   2
    //      original diff       ''      empty
    //      new diff pos        2
    //      new diff            '123'
    // diffType is DELETION:
    //      original            'text'
    //      new                 'tt'
    //      original diff pos   1
    //      original diff       'ex'
    //      new diff pos        1
    //      new diff            ''      empty
    // diffType is REPLACEMENT:
    //      original            'text'
    //      new                 't1234t'
    //      original diff pos   1
    //      original diff       'ex'
    //      new diff pos        1
    //      new diff            '1234'
};

void StringDiff(const WideString& originalString, const WideString& newString, StringDiffResult& result)
{
    // TODO: compare strings as UTF-32 to not bother with surrogate pairs

    int32 origLength = static_cast<int32>(originalString.size());
    int32 newLength = static_cast<int32>(newString.size());

    int32 origDiffBegin = 0;
    int32 newDiffBegin = 0;
    // Skip same characters from the beginning of both strings
    while (origDiffBegin < origLength && newDiffBegin < newLength && originalString[origDiffBegin] == newString[newDiffBegin])
    {
        origDiffBegin += 1;
        newDiffBegin += 1;
    }

    // No changes between original string and new string
    if (origDiffBegin == origLength && newDiffBegin == newLength)
    {
        result = StringDiffResult();
        return;
    }

    int32 origDiffEnd = origLength - 1;
    int32 newDiffEnd = newLength - 1;
    // Skip same characters from the end of both strings
    while (origDiffEnd >= origDiffBegin && newDiffEnd >= newDiffBegin && originalString[origDiffEnd] == newString[newDiffEnd])
    {
        origDiffEnd -= 1;
        newDiffEnd -= 1;
    }

    if (origDiffEnd < origDiffBegin)    // Insertion took place
    {
        result.diffType = StringDiffResult::INSERTION;
        result.originalStringDiffPosition = origDiffBegin;
        result.originalStringDiff = WideString();
        result.newStringDiffPosition = newDiffBegin;
        result.newStringDiff = WideString(newString, newDiffBegin, newDiffEnd - newDiffBegin + 1);
    }
    else if (newDiffEnd < origDiffBegin)    // Deletion took place
    {
        result.diffType = StringDiffResult::DELETION;
        result.originalStringDiffPosition = origDiffBegin;
        result.originalStringDiff = WideString(originalString, origDiffBegin, origDiffEnd - origDiffBegin + 1);
        result.originalStringDiffPosition = newDiffBegin;
        result.newStringDiff = WideString();
    }
    else    // Replacement took place
    {
        result.diffType = StringDiffResult::REPLACEMENT;
        result.originalStringDiffPosition = origDiffBegin;
        result.originalStringDiff = WideString(originalString, origDiffBegin, origDiffEnd - origDiffBegin + 1);
        result.newStringDiffPosition = newDiffBegin;
        result.newStringDiff = WideString(newString, newDiffBegin, newDiffEnd - newDiffBegin + 1);
    }
}

}   // unnamed namespace

std::shared_ptr<PrivateTextFieldWinUAP> PrivateTextFieldWinUAP::Create(UITextField* uiTextField)
{
    // It is highly not recommended to call shared_from_this() from constructor so introduce static Create method 
    std::shared_ptr<PrivateTextFieldWinUAP> obj = std::make_shared<PrivateTextFieldWinUAP>(uiTextField);
    obj->Init();
    return obj;
}

void PrivateTextFieldWinUAP::Init()
{
    core->RunOnUIThreadBlocked([this]()
    {
        CreateNativeText();

        std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
        auto keyboardHiding = ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>([this, self_weak](InputPane^, InputPaneVisibilityEventArgs^) {
            auto self = self_weak.lock();
            if (self != nullptr)
            {
                OnKeyboardHiding();
            }
        });
        auto keyboardShowing = ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>([this, self_weak](InputPane^, InputPaneVisibilityEventArgs^) {
            auto self = self_weak.lock();
            if (self != nullptr)
            {
                OnKeyboardShowing();
            }
        });
        InputPane::GetForCurrentView()->Showing += keyboardShowing;
        InputPane::GetForCurrentView()->Hiding += keyboardHiding;
    });
}

PrivateTextFieldWinUAP::PrivateTextFieldWinUAP(UITextField* uiTextField_)
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , uiTextField(uiTextField_)
{
}

PrivateTextFieldWinUAP::~PrivateTextFieldWinUAP()
{
    if (nativeControl != nullptr)
    {
        Control^ p = nativeControl;
        core->RunOnUIThread([p]() { // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
        nativeControl = nullptr;
        nativeText = nullptr;
        nativePassword = nullptr;
    }
}

void PrivateTextFieldWinUAP::OwnerAtPremortem()
{
    uiTextField = nullptr;
    textFieldDelegate = nullptr;
}

void PrivateTextFieldWinUAP::SetVisible(bool isVisible)
{
    if (visible != isVisible)
    {
        visible = isVisible;
        // Single line native text field is always rendered to texture and placed offscreen
        // Multiline native text field is always onscreen according to visibiliy flag
        if (multiline)
        {
            auto self{shared_from_this()};
            core->RunOnUIThread([this, self]() {
                SetVisibilityNative(visible);
            });
        }
    }
}

void PrivateTextFieldWinUAP::SetIsPassword(bool isPassword)
{
    // Do not allow multiline password fields
    DVASSERT((!isPassword || !multiline) && "Password multiline text fields are not allowed");
    if (isPassword != password)
    {
        password = isPassword;
        auto self{shared_from_this()};
        core->RunOnUIThreadBlocked([this, self]()
        {
            curText.clear();
            DeleteNativeControl();
            if (password)
                CreateNativePassword();
            else
                CreateNativeText();
            pendingTextureUpdate = true;
        });
    }
}

void PrivateTextFieldWinUAP::SetMaxLength(int32 value)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, value]()
    {
        // Native controls expect zero for unlimited input length
        int length = std::max(0, value);
        if (nativeText != nullptr)
            nativeText->MaxLength = length;
        else // if (nativePassword != nullptr)
            nativePassword->MaxLength = length;
    });
}

void PrivateTextFieldWinUAP::OpenKeyboard()
{
    // Focus native control as if mouse has been pressed to show keyboard
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self]()
    {
        nativeControl->Focus(FocusState::Pointer);
    });
}

void PrivateTextFieldWinUAP::CloseKeyboard()
{
    // Hide keyboard through unfocusing native control
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self]()
    {
        if (HasFocus())
        {
            core->XamlApplication()->UnfocusUIElement();
        }
    });
}

void PrivateTextFieldWinUAP::UpdateRect(const Rect& rect)
{
    bool sizeChanged = false;
    if (rect != originalRect)
    {
        originalRect = rect;
        pendingTextureUpdate = true;
        sizeChanged = true;
    }

    if (pendingTextureUpdate)
    {
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, sizeChanged]()
        {
            if (sizeChanged)
                PositionNative(originalRect, !multiline && !HasFocus());
            if (pendingTextureUpdate && !multiline && !HasFocus())
                RenderToTexture();
            pendingTextureUpdate = false;
        });
    }
}

void PrivateTextFieldWinUAP::SetText(const WideString& text)
{
    Platform::String^ str = ref new Platform::String(text.c_str());
    {
        LockGuard<Mutex> guard(textMutex);
        curText = text;
    }

    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, str]()
    {
        if (nativeText != nullptr)
            nativeText->Text = str;
        else if (nativePassword != nullptr)
            nativePassword->Password = str;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::GetText(WideString& text) const
{
    LockGuard<Mutex> guard(textMutex);
    text = curText;
}

void PrivateTextFieldWinUAP::SetTextColor(const Color& color)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, color]()
    {
        Windows::UI::Color nativeColor;
        nativeColor.R = static_cast<unsigned char>(color.r * 255.0f);
        nativeColor.G = static_cast<unsigned char>(color.g * 255.0f);
        nativeColor.B = static_cast<unsigned char>(color.b * 255.0f);
        nativeColor.A = 255;
        nativeControl->Foreground = ref new SolidColorBrush(nativeColor);
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::SetTextAlign(int32 align)
{
    if (!password)
    {
        textAlignment = 0;
        if (align & ALIGN_LEFT)
            textAlignment |= ALIGN_LEFT;
        else if (align & ALIGN_HCENTER)
            textAlignment |= ALIGN_HCENTER;
        else if (align & ALIGN_RIGHT)
            textAlignment |= ALIGN_RIGHT;

        if (0 == textAlignment)
            textAlignment = ALIGN_LEFT;
        textAlignment |= ALIGN_TOP; // Native TextBox doesn't support vertical text alignment
        InvertTextAlignmentDependingOnRtlAlignment();

        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]()
        {
            SetAlignmentNative(textAlignment);
            pendingTextureUpdate = true;
        });
    }
}

void PrivateTextFieldWinUAP::SetTextUseRtlAlign(bool useRtlAlign)
{
    if (rtlTextAlignment != useRtlAlign)
    {
        rtlTextAlignment = useRtlAlign;
        InvertTextAlignmentDependingOnRtlAlignment();

        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]()
        {
            SetAlignmentNative(textAlignment);
            pendingTextureUpdate = true;
        });
    }
}

void PrivateTextFieldWinUAP::SetFontSize(float32 virtualFontSize)
{
    const float32 scaleFactor = core->GetScreenScaleFactor();
    float32 fontSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(virtualFontSize);
    fontSize /= scaleFactor;

    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, fontSize]()
    {
        if (nativeText != nullptr)
            nativeText->FontSize = fontSize;
        else if (nativePassword != nullptr)
            nativePassword->FontSize = fontSize;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::SetDelegate(UITextFieldDelegate* textFieldDelegate_)
{
    textFieldDelegate = textFieldDelegate_;
}

void PrivateTextFieldWinUAP::SetMultiline(bool enable)
{
    // Do not allow multiline password fields
    DVASSERT((!enable || !password) && "Password multiline text fields are not allowed");
    if (multiline != enable)
    {
        multiline = enable;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]()
        {
            nativeText->AcceptsReturn = multiline;
            PositionNative(originalRect, !multiline);
        });
    }
}

void PrivateTextFieldWinUAP::SetInputEnabled(bool enable)
{
    if (!password)
    {
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, enable]()
        {
            nativeText->IsReadOnly = enable;
            pendingTextureUpdate = true;
        });
    }
}

void PrivateTextFieldWinUAP::SetSpellCheckingType(int32 value)
{
    if (!password)
    {
        bool enabled = UITextField::SPELL_CHECKING_TYPE_YES == value;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, enabled]()
        {
            nativeText->IsSpellCheckEnabled = enabled;
            pendingTextureUpdate = true;
        });
    }
}

void PrivateTextFieldWinUAP::SetAutoCapitalizationType(int32 value)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetAutoCorrectionType(int32 value)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetKeyboardAppearanceType(int32 value)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetKeyboardType(int32 value)
{
    InputScopeNameValue nativeValue = InputScopeNameValue::Default;
    switch (value)
    {
    case DAVA::UITextField::KEYBOARD_TYPE_URL:
        nativeValue = InputScopeNameValue::Url;
        break;
    case DAVA::UITextField::KEYBOARD_TYPE_NUMBER_PAD:
    case DAVA::UITextField::KEYBOARD_TYPE_DECIMAL_PAD:
    case DAVA::UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION:
        nativeValue = InputScopeNameValue::Number;
        break;
    case DAVA::UITextField::KEYBOARD_TYPE_PHONE_PAD:
        nativeValue = InputScopeNameValue::TelephoneNumber;
        break;
    case DAVA::UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD:
        nativeValue = InputScopeNameValue::NameOrPhoneNumber;
        break;
    case DAVA::UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS:
        nativeValue = InputScopeNameValue::EmailSmtpAddress;
        break;
    default:
        nativeValue = InputScopeNameValue::Default;
        break;
    }

    // PasswordBox supports only Password and NumericPin from InputScopeNameValue enum
    if (password)
        nativeValue = InputScopeNameValue::Password;
    
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, nativeValue]() {
        InputScope^ inputScope = ref new InputScope();
        inputScope->Names->Append(ref new InputScopeName(nativeValue));
        if (nativeText != nullptr)
            nativeText->InputScope = inputScope;
        else if (nativePassword != nullptr)
            nativePassword->InputScope = inputScope;
    });
}

void PrivateTextFieldWinUAP::SetReturnKeyType(int32 /*value*/)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetEnableReturnKeyAutomatically(bool /*value*/)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetCursorPos(uint32 pos)
{
    if (!password && static_cast<int32>(pos) >= 0)
    {
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, pos]()
        {
            nativeText->SelectionStart = pos;
        });
    }
}

void PrivateTextFieldWinUAP::CreateNativeText()
{
    nativeText = ref new TextBox();
    core->XamlApplication()->SetTextBoxCustomStyle(nativeText);

    nativeControl = nativeText;
    nativeControl->Visibility = Visibility::Collapsed;
    nativeControl->BorderThickness = Thickness(0.0);
    nativeControl->Background = ref new SolidColorBrush(Colors::Transparent);
    nativeControl->Foreground = ref new SolidColorBrush(Colors::White);
    nativeControl->BorderBrush = ref new SolidColorBrush(Colors::Transparent);
    nativeControl->Padding = Thickness(0.0);

    nativeText->TextAlignment = TextAlignment::Left;

    core->XamlApplication()->AddUIElement(nativeControl);
    PositionNative(originalRect, true);

    InstallTextEventHandlers();
    nativeControl->Visibility = Visibility::Visible;
}

void PrivateTextFieldWinUAP::CreateNativePassword()
{
    nativePassword = ref new PasswordBox();
    core->XamlApplication()->SetPasswordBoxCustomStyle(nativePassword);

    nativeControl = nativePassword;
    nativeControl->Visibility = Visibility::Collapsed;
    nativeControl->BorderThickness = Thickness(0.0);
    nativeControl->Background = ref new SolidColorBrush(Colors::Transparent);
    nativeControl->Foreground = ref new SolidColorBrush(Colors::White);
    nativeControl->BorderBrush = ref new SolidColorBrush(Colors::Transparent);
    nativeControl->Padding = Thickness(0.0);

    core->XamlApplication()->AddUIElement(nativeControl);
    PositionNative(originalRect, true);

    InstallPasswordEventHandlers();
    nativeControl->Visibility = Visibility::Visible;
}

void PrivateTextFieldWinUAP::DeleteNativeControl()
{
    static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(nativeControl);
    nativeControl = nullptr;
    nativeText = nullptr;
    nativePassword = nullptr;
}

void PrivateTextFieldWinUAP::InstallTextEventHandlers()
{
    using namespace Platform;

    std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
    auto keyDown = ref new KeyEventHandler([this, self_weak](Object^, KeyRoutedEventArgs^ args) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnKeyDown(args->Key);
        }
    });
    auto gotFocus = ref new RoutedEventHandler([this, self_weak](Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnGotFocus();
        }
    });
    auto lostFocus = ref new RoutedEventHandler([this, self_weak](Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnLostFocus();
        }
    });
    auto selectionChanged = ref new RoutedEventHandler([this, self_weak](Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnSelectionChanged();
        }
    });
    auto textChanged = ref new TextChangedEventHandler([this, self_weak](Object^, TextChangedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnTextChanged();
        }
    });
    nativeText->KeyDown += keyDown;
    nativeText->GotFocus += gotFocus;
    nativeText->LostFocus += lostFocus;
    nativeText->SelectionChanged += selectionChanged;
    nativeText->TextChanged += textChanged;
}

void PrivateTextFieldWinUAP::InstallPasswordEventHandlers()
{
    using namespace Platform;
    std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
    auto keyDown = ref new KeyEventHandler([this, self_weak](Object^, KeyRoutedEventArgs^ args) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnKeyDown(args->Key);
        }
    });
    auto gotFocus = ref new RoutedEventHandler([this, self_weak](Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnGotFocus();
        }
    });
    auto lostFocus = ref new RoutedEventHandler([this, self_weak](Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnLostFocus();
        }
    });
    auto passwordChanged = ref new RoutedEventHandler([this, self_weak](Object^, RoutedEventArgs^) {
        auto self = self_weak.lock();
        if (self != nullptr)
        {
            OnPasswordChanged();
        }
    });
    nativePassword->KeyDown += keyDown;
    nativePassword->GotFocus += gotFocus;
    nativePassword->LostFocus += lostFocus;
    nativePassword->PasswordChanged += passwordChanged;
}

void PrivateTextFieldWinUAP::SetVisibilityNative(bool show)
{
    nativeControl->Visibility = show ? Visibility::Visible : Visibility::Collapsed;
}

void PrivateTextFieldWinUAP::SetAlignmentNative(int32 alignment)
{
    TextAlignment nativeAlignment = TextAlignment::Left;
    if (textAlignment & ALIGN_LEFT)
        nativeAlignment = TextAlignment::Left;
    else if (textAlignment & ALIGN_HCENTER)
        nativeAlignment = TextAlignment::Center;
    else if (textAlignment & ALIGN_RIGHT)
        nativeAlignment = TextAlignment::Right;

    // Only TextBox has TextAlignment property, not PasswordBox
    if (nativeText != nullptr)
        nativeText->TextAlignment = nativeAlignment;
}

void PrivateTextFieldWinUAP::InvertTextAlignmentDependingOnRtlAlignment()
{
    // As far as I understood RTL text alignment affects only text alignment inside control rect
    // If RTL text alignment flag is set then invert text alignment from left to right and vice versa
    if (rtlTextAlignment)
    {
        if (textAlignment & ALIGN_LEFT)
        {
            textAlignment &= ~ALIGN_LEFT;
            textAlignment |= ALIGN_RIGHT;
        }
        else if (textAlignment & ALIGN_RIGHT)
        {
            textAlignment &= ~ALIGN_RIGHT;
            textAlignment |= ALIGN_LEFT;
        }
    }
}

void PrivateTextFieldWinUAP::PositionNative(const Rect& rectInVirtualCoordinates, bool offScreen)
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect controlRect = coordSystem->ConvertVirtualToPhysical(rectInVirtualCoordinates);
    controlRect += coordSystem->GetPhysicalDrawOffset();

    // 2. map physical to window
    const float32 scaleFactor = core->GetScreenScaleFactor();
    controlRect.x /= scaleFactor;
    controlRect.y /= scaleFactor;
    controlRect.dx /= scaleFactor;
    controlRect.dy /= scaleFactor;

    if (offScreen)
    {
        controlRect.x = -controlRect.dx;
        controlRect.y = -controlRect.dy;
    }

    // 3. set control's position and size
    nativeControl->MinHeight = 0.0;     // Force minimum control sizes to zero to
    nativeControl->MinWidth = 0.0;      // allow setting any control sizes
    nativeControl->Width = controlRect.dx;
    nativeControl->Height = controlRect.dy;
    core->XamlApplication()->PositionUIElement(nativeControl, controlRect.x, controlRect.y);
}

void PrivateTextFieldWinUAP::OnKeyDown(Windows::System::VirtualKey virtualKey)
{
    if (nativeText != nullptr)
        savedCaretPosition = nativeText->SelectionStart;

    switch (virtualKey)
    {
    case VirtualKey::Back:
        savedCaretPosition += 1;
        break;
    case VirtualKey::Enter:
        if (!multiline)
        {
            auto self{shared_from_this()};
            core->RunOnMainThread([this, self]()
            {
                if (textFieldDelegate != nullptr)
                    textFieldDelegate->TextFieldShouldReturn(uiTextField);
            });
        }
        break;
    case VirtualKey::Escape:
        {
            auto self{shared_from_this()};
            core->RunOnMainThread([this, self]()
            {
                if (textFieldDelegate != nullptr)
                    textFieldDelegate->TextFieldShouldCancel(uiTextField);
            });
        }
        break;
    default:
        break;
    }
}

void PrivateTextFieldWinUAP::OnGotFocus()
{
    if (nativeText != nullptr)
    {
        nativeText->SelectionStart = nativeText->Text->Length();
    }
    if (!multiline)
    {
        PositionNative(originalRect, false);
        auto self{shared_from_this()};
        core->RunOnMainThread([this, self]()
        {
            if (uiTextField != nullptr)
            {
                uiTextField->SetSprite(nullptr, 0);
            }
        });
    }
}

void PrivateTextFieldWinUAP::OnLostFocus()
{
    if (!multiline)
    {
        PositionNative(originalRect, true);
        RenderToTexture();
    }
}

void PrivateTextFieldWinUAP::OnSelectionChanged()
{
    caretPosition = nativeText->SelectionStart;
}

void PrivateTextFieldWinUAP::OnTextChanged()
{
    if (ignoreTextChange || nullptr == nativeText)
    {
        ignoreTextChange = false;
        return;
    }

    WideString curTextCopy;
    {
        LockGuard<Mutex> guard(textMutex);
        curTextCopy = curText;
    }
    WideString newText(nativeText->Text->Data());
    bool textDeclined = ProcessTextChanged(curTextCopy, newText);
    if (textDeclined)
    {
        nativeText->Text = ref new Platform::String(curTextCopy.c_str());
        // Restore caret position to position before text has been changed
        nativeText->SelectionStart = savedCaretPosition;
        ignoreTextChange = true;
    }
    else
    {
        LockGuard<Mutex> guard(textMutex);
        curText = std::move(newText);
    }
}

void PrivateTextFieldWinUAP::OnPasswordChanged()
{
    if (ignoreTextChange || nullptr == nativePassword)
    {
        ignoreTextChange = false;
        return;
    }

    WideString curTextCopy;
    {
        LockGuard<Mutex> guard(textMutex);
        curTextCopy = curText;
    }
    WideString newText(nativePassword->Password->Data());
    bool textDeclined = ProcessTextChanged(curTextCopy, newText);
    if (textDeclined)
    {
        nativePassword->Password = ref new Platform::String(curTextCopy.c_str());
        ignoreTextChange = true;
    }
    else
    {
        LockGuard<Mutex> guard(textMutex);
        curText = std::move(newText);
    }
}

bool PrivateTextFieldWinUAP::HasFocus() const
{
    return FocusState::Unfocused != nativeControl->FocusState;
}

bool PrivateTextFieldWinUAP::ProcessTextChanged(const WideString& curText, const WideString& newText)
{
    StringDiffResult diffR;
    StringDiff(curText, newText, diffR);
    if (StringDiffResult::NO_CHANGE == diffR.diffType)
        return false;

    // Ask delegate whether to accept changes
    bool accept = true;
    auto self{shared_from_this()};
    core->RunOnMainThreadBlocked([this, self, &diffR, &accept]()
    {
        if (uiTextField != nullptr && textFieldDelegate != nullptr)
        {
            accept = textFieldDelegate->TextFieldKeyPressed(uiTextField,
                                                            diffR.originalStringDiffPosition,
                                                            static_cast<int32>(diffR.originalStringDiff.length()),
                                                            diffR.newStringDiff);
        }
    });

    if (accept)
    {
        core->RunOnMainThread([this, self, curText, newText]()
        {
            if (textFieldDelegate != nullptr)
                textFieldDelegate->TextFieldOnTextChanged(uiTextField, curText, newText);
        });
    }
    return !accept;
}

void PrivateTextFieldWinUAP::OnKeyboardHiding()
{
    if (nativeControl->FocusState != FocusState::Unfocused)
    {
        auto self{shared_from_this()};
        core->RunOnMainThread([this, self]()
        {
            if (textFieldDelegate != nullptr)
                textFieldDelegate->OnKeyboardHidden();
        });
    }
}

void PrivateTextFieldWinUAP::OnKeyboardShowing()
{
    if (nativeControl->FocusState != FocusState::Unfocused)
    {
        Windows::Foundation::Rect srcRect = InputPane::GetForCurrentView()->OccludedRect;
        DAVA::Rect keyboardRect(srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height);
        auto self{shared_from_this()};
        core->RunOnMainThread([this, self, keyboardRect]()
        {
            if (textFieldDelegate != nullptr)
                textFieldDelegate->OnKeyboardShown(keyboardRect);
        });
    }
}

void PrivateTextFieldWinUAP::RenderToTexture()
{
    auto self{shared_from_this()};
    RenderTargetBitmap^ renderTarget = ref new RenderTargetBitmap;

    auto renderTask = create_task(renderTarget->RenderAsync(nativeControl)).then([this, self, renderTarget]()
    {
        return renderTarget->GetPixelsAsync();
    }).then([this, self, renderTarget](IBuffer^ renderBuffer)
    {
        int32 imageWidth = renderTarget->PixelWidth;
        int32 imageHeight = renderTarget->PixelHeight;
        size_t streamSize = static_cast<size_t>(renderBuffer->Length);
        DataReader^ reader = DataReader::FromBuffer(renderBuffer);

        size_t index = 0;
        Vector<uint8> buf(streamSize, 0);
        while (reader->UnconsumedBufferLength > 0)
        {
            buf[index] = reader->ReadByte();
            index += 1;
        }

        RefPtr<Sprite> sprite(CreateSpriteFromPreviewData(&buf[0], imageWidth, imageHeight));
        if (sprite.Valid())
        {
            core->RunOnMainThread([this, self, sprite]() {
                if (uiTextField != nullptr)
                {
                    uiTextField->SetSprite(sprite.Get(), 0);
                }
            });
        }
    }).then([this, self](task<void> t)
    {
        try {
            t.get();
        }
        catch (Platform::COMException^ e) {
            HRESULT hr = e->HResult;
            Logger::Error("[TextField] RenderToTexture failed: 0x%08X", hr);
        }
    });
}

Sprite* PrivateTextFieldWinUAP::CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const
{
    const uint32 pitch = 4 * width;
    ImageConvert::ConvertImageDirect(FORMAT_BGRA8888, FORMAT_RGBA8888, imageData, width, height, pitch, imageData, width, height, pitch);
    RefPtr<Image> imgSrc(Image::CreateFromData(width, height, FORMAT_RGBA8888, imageData));
    return Sprite::CreateFromImage(imgSrc.Get(), true, false);
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
