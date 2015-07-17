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

#include "UI/UITextField.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

#include <ppltasks.h>

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
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

PrivateTextFieldWinUAP::PrivateTextFieldWinUAP(UITextField* uiTextField_)
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , uiTextField(uiTextField_)
{
    core->RunOnUIThreadBlocked([this]() {
        nativeControl = ref new TextBox();
        nativeControl->Visibility = Visibility::Collapsed;
        nativeControl->TextAlignment = TextAlignment::Left;
        nativeControl->FlowDirection = FlowDirection::LeftToRight;

        nativeControl->Background = ref new SolidColorBrush(Colors::Transparent);
        nativeControl->BorderBrush = ref new SolidColorBrush(Colors::Transparent);

        core->XamlApplication()->AddUIElement(nativeControl);

        InstallEventHandlers();
    });
}

PrivateTextFieldWinUAP::~PrivateTextFieldWinUAP()
{
    if (nativeControl != nullptr)
    {
        // Compiler complains of capturing nativeWebView data member in lambda
        TextBox^ p = nativeControl;
        core->RunOnUIThread([p]() { // We don't need blocking call here
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
        nativeControl = nullptr;
    }
}

void PrivateTextFieldWinUAP::FlyToSunIcarus()
{
    uiTextField = nullptr;
}

void PrivateTextFieldWinUAP::SetVisible(bool isVisible)
{
    if (visible != isVisible)
    {
        visible = isVisible;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self](){
            SetVisibilityNative(visible);
        });
    }
}

void PrivateTextFieldWinUAP::SetMaxLength(int32 value)
{
    // Native control expects zero for unlimited input length
    value = std::max(0, value);
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, value]() {
        nativeControl->MaxLength = value;
    });
}

void PrivateTextFieldWinUAP::OpenKeyboard()
{
    core->RunOnUIThread([]() {
        InputPane::GetForCurrentView()->TryShow();
    });
}

void PrivateTextFieldWinUAP::CloseKeyboard()
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self]() {
        InputPane::GetForCurrentView()->TryHide();
    });
}

void PrivateTextFieldWinUAP::UpdateRect(const Rect& rect)
{
    if (rect != originalRect)
    {
        originalRect = rect;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]() {
            PositionNative(originalRect, false);
        });
    }
}

void PrivateTextFieldWinUAP::SetText(const WideString& text)
{
    Platform::String^ str = ref new Platform::String(text.c_str());
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, str]() {
        nativeControl->Text = str;
        RenderToTexture();
    });
}

void PrivateTextFieldWinUAP::GetText(WideString& text) const
{
    core->RunOnUIThreadBlocked([this, &text]() {
        text = nativeControl->Text->Data();
    });
}

void PrivateTextFieldWinUAP::SetTextColor(const Color& color)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, color]() {
        Windows::UI::Color nativeColor;
        nativeColor.R = static_cast<unsigned char>(color.r * 255.0f);
        nativeColor.G = static_cast<unsigned char>(color.g * 255.0f);
        nativeColor.B = static_cast<unsigned char>(color.b * 255.0f);
        nativeColor.A = 255;
        nativeControl->Foreground = ref new SolidColorBrush(nativeColor);
    });
}

void PrivateTextFieldWinUAP::SetTextAlign(int32 align)
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
    textAlignment |= ALIGN_TOP; // Native text field doesn't support vertical text alignment

    auto self{shared_from_this()};
    core->RunOnUIThread([this, self]() {
        if (textAlignment & ALIGN_LEFT)
            nativeControl->TextAlignment = TextAlignment::Left;
        else if (textAlignment & ALIGN_HCENTER)
            nativeControl->TextAlignment = TextAlignment::Center;
        else if (textAlignment & ALIGN_RIGHT)
            nativeControl->TextAlignment = TextAlignment::Right;
    });
}

int32 PrivateTextFieldWinUAP::GetTextAlign() const
{
    return textAlignment;
}

void PrivateTextFieldWinUAP::SetTextUseRtlAlign(bool useRtlAlign)
{
    if (flowDirectionRTL != useRtlAlign)
    {
        flowDirectionRTL = useRtlAlign;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self](){
            nativeControl->FlowDirection = flowDirectionRTL ? FlowDirection::RightToLeft : FlowDirection::LeftToRight;
        });
    }
}

bool PrivateTextFieldWinUAP::GetTextUseRtlAlign() const
{
    return flowDirectionRTL;
}

void PrivateTextFieldWinUAP::SetFontSize(float32 size)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, size]() {
        nativeControl->FontSize = size;
    });
}

void PrivateTextFieldWinUAP::SetMultiline(bool enable)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, enable]() {
        nativeControl->AcceptsReturn = enable;
    });
}

void PrivateTextFieldWinUAP::SetInputEnabled(bool enable)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, enable]() {
        nativeControl->IsReadOnly = enable;
    });
}

bool PrivateTextFieldWinUAP::IsRenderToTexture() const
{
    return renderToTexture;
}

void PrivateTextFieldWinUAP::SetSpellCheckingType(int32 value)
{
    bool enabled = UITextField::SPELL_CHECKING_TYPE_YES == value;
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, enabled]() {
        nativeControl->IsSpellCheckEnabled = enabled;
    });
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
    
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, nativeValue]() {
        InputScope^ inputScope = ref new InputScope();
        inputScope->Names->Append(ref new InputScopeName(nativeValue));
        nativeControl->InputScope = inputScope;
    });
}

void PrivateTextFieldWinUAP::InstallEventHandlers()
{
    auto keyDown = ref new KeyEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args) {
        OnKeyDown(sender, args);
    });
    auto keyUp = ref new KeyEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args) {
        OnKeyUp(sender, args);
    });
    auto gotFocus = ref new RoutedEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args) {
        OnGotFocus(sender, args);
    });
    auto lostFocus = ref new RoutedEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args) {
        OnLostFocus(sender, args);
    });
    nativeControl->KeyDown += keyDown;
    nativeControl->KeyUp += keyUp;
    nativeControl->GotFocus += gotFocus;
    nativeControl->LostFocus += lostFocus;
}

void PrivateTextFieldWinUAP::SetVisibilityNative(bool show)
{
    nativeControl->Visibility = show ? Visibility::Visible : Visibility::Collapsed;
}

void PrivateTextFieldWinUAP::PositionNative(const Rect& rect, bool offScreen)
{
    VirtualCoordinatesSystem* coordSys = VirtualCoordinatesSystem::Instance();

    Rect physRect = coordSys->ConvertVirtualToPhysical(rect);
    const Vector2 physOffset = coordSys->GetPhysicalDrawOffset();

    float32 width = physRect.dx + physOffset.x;
    float32 height = physRect.dy + physOffset.y;

    if (offScreen)
    {
        physRect.x = -width;
        physRect.y = -height;
    }
    nativeControl->Width = width;
    nativeControl->Height = height;
    core->XamlApplication()->PositionUIElement(nativeControl, physRect.x, physRect.y);
}

void PrivateTextFieldWinUAP::OnKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args)
{
    VirtualKey vk = args->Key;
    Logger::Debug("************* PrivateTextFieldWinUAP::OnKeyDown: %d (0x%X", (int)vk, (int)vk);
    /*if (VirtualKey::Number0 <= vk && vk <= VirtualKey::Number9)
    {
        int h = 0;
    }
    else
        args->Handled = true;*/
}

void PrivateTextFieldWinUAP::OnKeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args)
{
    //VirtualKey vk = args->Key;
    //Logger::Debug("************* PrivateTextFieldWinUAP::OnKeyUp: %d (0x%X", (int)vk, (int)vk);

    //if (vk == VirtualKey::Enter)
    //{
    //    CloseKeyboard();
    //}
    /*if (VirtualKey::Number0 <= vk && vk <= VirtualKey::Number9)
    {
        int h = 0;
    }
    else
        args->Handled = true;*/
}

void PrivateTextFieldWinUAP::OnGotFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args)
{
    Logger::Debug("********************* PrivateTextFieldWinUAP::OnGotFocus");
}

void PrivateTextFieldWinUAP::OnLostFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args)
{
    Logger::Debug("********************* PrivateTextFieldWinUAP::OnLostFocus");
}

void PrivateTextFieldWinUAP::RenderToTexture()
{
    static bool f = false;

    if (f)
        return;
    f = true;

    RenderTargetBitmap^ rtb = ref new RenderTargetBitmap;
    auto r1 = rtb->RenderAsync(nativeControl);

    create_task(r1).then([this, rtb]() {

        int h = 0;

        auto x = rtb->GetPixelsAsync();

    });
#if 0
    auto t1 = create_task(r1).then([this, rtb, r1]()
    {
        int st = (int)r1->Status;
        while (st == (int)AsyncStatus::Started)
        {
            st = (int)r1->Status;
        }

        return rtb->GetPixelsAsync();
    }).then([this, rtb](IBuffer^ buf)
    {
        //DataReader^ reader = ref new DataReader(buf);
        int cap = (int)buf->Capacity;
        int sz = (int)buf->Length;
        sz = sz;
    });

    try {
        t1.get();
    }
    catch (Platform::COMException^ e)
    {
        HRESULT hr = e->HResult;
        Platform::String^ s = e->Message;
        Logger::Error("[WebView] RenderToTexture failed: 0x%08X", hr);
    }
#endif
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
