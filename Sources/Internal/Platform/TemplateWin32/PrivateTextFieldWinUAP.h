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

#ifndef __DAVAENGINE_PRIVATETEXTFIELD_WINUAP_H__
#define __DAVAENGINE_PRIVATETEXTFIELD_WINUAP_H__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Base/BaseTypes.h"
#include "Math/Rect.h"
#include "Concurrency/Atomic.h"

namespace DAVA
{

class Color;
class UITextField;
class CorePlatformWinUAP;

class PrivateTextFieldWinUAP : public std::enable_shared_from_this<PrivateTextFieldWinUAP>
{
public:
    PrivateTextFieldWinUAP(UITextField* uiTextField);
    ~PrivateTextFieldWinUAP();

    // UITextFieldWinUAP should invoke it in its destructor to tell this class instance
    // to fly away on its own (finish pending jobs if any, and delete when all references are lost)
    void FlyToSunIcarus();

    void SetVisible(bool isVisible);
    void SetMaxLength(int32 value);

    void OpenKeyboard();
    void CloseKeyboard();

    void UpdateRect(const Rect& rect);

    void SetText(const WideString& text);
    void GetText(WideString& text) const;

    void SetTextColor(const Color& color);
    void SetTextAlign(int32 align);
    int32 GetTextAlign() const;
    void SetTextUseRtlAlign(bool useRtlAlign);
    bool GetTextUseRtlAlign() const;

    void SetFontSize(float32 size);

    void SetMultiline(bool enable);

    void SetInputEnabled(bool enable);

    bool IsRenderToTexture() const;

    void SetAutoCapitalizationType(int32 value);
    void SetAutoCorrectionType(int32 value);
    void SetSpellCheckingType(int32 value);
    void SetKeyboardAppearanceType(int32 value);
    void SetKeyboardType(int32 value);
    void SetReturnKeyType(int32 value);
    void SetEnableReturnKeyAutomatically(bool value);

private:
    void InstallEventHandlers();
    void SetVisibilityNative(bool show);
    void PositionNative(const Rect& rect, bool offScreen);

    void RenderToTexture();

private:    // Event handlers
    void OnKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args);
    void OnKeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args);

    void OnGotFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
    void OnLostFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);

private:
    CorePlatformWinUAP* core;
    Atomic<UITextField*> uiTextField;
    Windows::UI::Xaml::Controls::TextBox^ nativeControl = nullptr;
    Rect originalRect;
    bool visible = false;
    int32 textAlignment = ALIGN_LEFT | ALIGN_TOP;
    bool flowDirectionRTL = false;
    bool renderToTexture = false;
};

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_PRIVATETEXTFIELD_WINUAP_H__
