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
#include "Concurrency/Mutex.h"

namespace DAVA
{

class Color;
class Sprite;
class UITextField;
class UITextFieldDelegate;
class CorePlatformWinUAP;

class PrivateTextFieldWinUAP : public std::enable_shared_from_this<PrivateTextFieldWinUAP>
{
public:
    static std::shared_ptr<PrivateTextFieldWinUAP> Create(UITextField* uiTextField);

    // Though static Create is used to create instance constructor is declared public to
    // use std::make_shared without tricks
    PrivateTextFieldWinUAP(UITextField* uiTextField);
    ~PrivateTextFieldWinUAP();

    // UITextFieldWinUAP should invoke it in its destructor to tell this class instance
    // to fly away on its own (finish pending jobs if any, and delete when all references are lost)
    void OwnerAtPremortem();

    void SetVisible(bool isVisible);
    void SetIsPassword(bool isPassword);
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

    void SetFontSize(float32 virtualFontSize);

    void SetDelegate(UITextFieldDelegate* textFieldDelegate);

    void SetMultiline(bool enable);

    void SetInputEnabled(bool enable);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    void SetAutoCapitalizationType(int32 value);
    void SetAutoCorrectionType(int32 value);
    void SetSpellCheckingType(int32 value);
    void SetKeyboardAppearanceType(int32 value);
    void SetKeyboardType(int32 value);
    void SetReturnKeyType(int32 value);
    void SetEnableReturnKeyAutomatically(bool value);

    uint32 GetCursorPos() const;
    void SetCursorPos(uint32 pos);

private:
    void Init();
    void CreateNativeText();
    void CreateNativePassword();
    void DeleteNativeControl();
    void InstallTextEventHandlers();
    void InstallPasswordEventHandlers();
    void SetVisibilityNative(bool show);
    void SetAlignmentNative(int32 alignment);
    void InvertTextAlignmentDependingOnRtlAlignment();
    void PositionNative(const Rect& rectInVirtualCoordinates, bool offScreen);

    void RenderToTexture();
    Sprite* CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const;

private:    // Event handlers
    void OnKeyDown(Windows::System::VirtualKey virtualKey);
    void OnGotFocus();
    void OnLostFocus();

    // TextBox specific events
    void OnSelectionChanged();
    void OnTextChanged();
    // PasswordBox specific events
    void OnPasswordChanged();

    bool HasFocus() const;
    bool ProcessTextChanged(const WideString& curText, const WideString& newText);

    void OnKeyboardHiding();
    void OnKeyboardShowing();

private:
    CorePlatformWinUAP* core;
    UITextField* uiTextField = nullptr;
    UITextFieldDelegate* textFieldDelegate = nullptr;
    // Windows UAP has two different controls for text input and password input
    // So we should switch internal implementation depending on user's wishes
    Windows::UI::Xaml::Controls::TextBox^ nativeText = nullptr;
    Windows::UI::Xaml::Controls::PasswordBox^ nativePassword = nullptr;
    Windows::UI::Xaml::Controls::Control^ nativeControl = nullptr;      // Points either to nativeText or nativePassword
    Rect originalRect = Rect(0.0f, 0.0f, 100.0f, 20.0f);
    bool visible = false;
    int32 textAlignment = ALIGN_LEFT | ALIGN_TOP;
    bool rtlTextAlignment = false;
    bool password = false;
    bool multiline = false;
    bool pendingTextureUpdate = false;      // Flag indicating that texture image should be recreated

    WideString curText;
    mutable Mutex textMutex;

    bool ignoreTextChange = false;
    int32 caretPosition = 0;                // Current caret position
    int32 savedCaretPosition = 0;           // Saved caret position to restore it when delegate declines text changing
};

//////////////////////////////////////////////////////////////////////////
inline int32 PrivateTextFieldWinUAP::GetTextAlign() const
{
    return textAlignment;
}

inline bool PrivateTextFieldWinUAP::GetTextUseRtlAlign() const
{
    return rtlTextAlignment;
}

inline void PrivateTextFieldWinUAP::SetRenderToTexture(bool /*value*/)
{
    // Do nothing as single line tex field always is painted into texture
    // Multiline text field is never rendered to texture
}

inline bool PrivateTextFieldWinUAP::IsRenderToTexture() const
{
    return !multiline;
}

inline uint32 PrivateTextFieldWinUAP::GetCursorPos() const
{
    return caretPosition;
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_PRIVATETEXTFIELD_WINUAP_H__
