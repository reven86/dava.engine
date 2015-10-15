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
#include "Math/Color.h"
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
    struct TextFieldProperties
    {
        TextFieldProperties();
        void ClearChangedFlags();

        bool createNew : 1;
        bool focus : 1;
        bool focusChanged : 1;

        Rect rect;
        Rect rectInWindowSpace;
        WideString text;
        Color textColor;
        bool visible = false;
        bool password = false;
        bool multiline = false;
        bool inputEnabled = false;
        bool spellCheckingEnabled = false;
        bool textRtlAlignment = false;
        int32 textAlignment = 0;
        int32 maxTextLength = 0;
        int32 keyboardType = 0;
        int32 caretPosition = 0;
        float32 fontSize = 0.0f;

        bool anyPropertyChanged : 1;
        bool rectChanged : 1;
        bool textChanged : 1;
        bool textColorChanged : 1;
        bool visibleChanged : 1;
        bool passwordChanged : 1;
        bool multilineChanged : 1;
        bool inputEnabledChanged : 1;
        bool spellCheckingEnabledChanged : 1;
        bool textRtlAlignmentChanged : 1;
        bool textAlignmentChanged : 1;
        bool maxTextLengthChanged : 1;
        bool keyboardTypeChanged : 1;
        bool caretPositionChanged : 1;
        bool fontSizeChanged : 1;

        bool rectAssigned : 1;
        bool textAssigned : 1;
        bool textColorAssigned : 1;
        bool visibleAssigned : 1;
        bool passwordAssigned : 1;
        bool multilineAssigned : 1;
        bool inputEnabledAssigned : 1;
        bool spellCheckingEnabledAssigned : 1;
        bool textRtlAlignmentAssigned : 1;
        bool textAlignmentAssigned : 1;
        bool maxTextLengthAssigned : 1;
        bool keyboardTypeAssigned : 1;
        bool caretPositionAssigned : 1;
        bool fontSizeAssigned : 1;
    };

public:
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
    void CreateNativeControl(bool textControl);
    void DeleteNativeControl();
    void InstallCommonEventHandlers();
    void InstallTextEventHandlers();
    void InstallPasswordEventHandlers();
    void InstallKeyboardEventHandlers();

    void ProcessProperties(const TextFieldProperties& props);
    void ApplyChangedProperties(const TextFieldProperties& props);
    void ApplyAssignedProperties(const TextFieldProperties& props);

    void SetNativePositionAndSize(const Rect& rect, bool offScreen);
    void SetNativeVisible(bool visible);
    void SetNativeMultiline(bool multiline);
    void SetNativeInputEnabled(bool enabled);
    void SetNativeText(const WideString& text);
    void SetNativeMaxTextLength(int32 maxLength);
    void SetNativeCaretPosition(int32 caretPosition);
    void SetNativeFontSize(float32 fontSize);
    void SetNativeTextColor(const Color& textColor);
    void SetNativeTextAlignment(int32 textAlignment, bool textRtlAlignment);
    void SetNativeKeyboardType(int32 type);
    void SetNativeSpellChecking(bool enabled);

    bool HasFocus() const;
    Platform::String ^ GetNativeText() const;
    int32 GetNativeCaretPosition() const;

    bool IsPassword() const;
    bool IsMultiline() const;

    Rect VirtualToWindow(const Rect& srcRect) const;
    Rect WindowToVirtual(const Rect& srcRect) const;
    void RenderToTexture(bool moveOffScreenOnCompletion);
    Sprite* CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const;

private:    // Event handlers
    void OnKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ args);
    void OnKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ args);
    void OnGotFocus();
    void OnLostFocus();
    void OnTextChanged();

    // TextBox specific events
    void OnSelectionChanged();

    // Onscreen keyboard events
    void OnKeyboardHiding(Windows::UI::ViewManagement::InputPaneVisibilityEventArgs ^ args);
    void OnKeyboardShowing(Windows::UI::ViewManagement::InputPaneVisibilityEventArgs ^ args);

private:
    CorePlatformWinUAP* core;
    UITextField* uiTextField = nullptr;
    UITextFieldDelegate* textFieldDelegate = nullptr;
    // Windows UAP has two different controls for text input and password input
    // So we should switch internal implementation depending on user's wishes
    Windows::UI::Xaml::Controls::TextBox^ nativeText = nullptr;
    Windows::UI::Xaml::Controls::PasswordBox^ nativePassword = nullptr;
    Windows::UI::Xaml::Controls::Control^ nativeControl = nullptr;      // Points either to nativeText or nativePassword
    Windows::UI::Xaml::Controls::Border^ nativeControlHolder = nullptr;

    // Tokens to unsubscribe from touch keyboard event handlers
    Windows::Foundation::EventRegistrationToken tokenKeyboardShowing;
    Windows::Foundation::EventRegistrationToken tokenKeyboardHiding;

    bool ignoreTextChange = false;
    bool waitRenderToTextureComplete = false;   // If flag is set do not move native control offscreen to get rid of some flickering

    int32 caretPosition = 0;                // Current caret position
    int32 savedCaretPosition = 0;           // Saved caret position to restore it when delegate declines text changing

    Rect rectInWindowSpace;

    WideString curText;
    TextFieldProperties properties;
    bool programmaticTextChange = false;
};

//////////////////////////////////////////////////////////////////////////
inline int32 PrivateTextFieldWinUAP::GetTextAlign() const
{
    return properties.textAlignment;
}

inline bool PrivateTextFieldWinUAP::GetTextUseRtlAlign() const
{
    return properties.textRtlAlignment;
}

inline void PrivateTextFieldWinUAP::SetRenderToTexture(bool /*value*/)
{
    // Do nothing as single line tex field always is painted into texture
    // Multiline text field is never rendered to texture
}

inline bool PrivateTextFieldWinUAP::IsRenderToTexture() const
{
    return !properties.multiline;
}

inline uint32 PrivateTextFieldWinUAP::GetCursorPos() const
{
    return caretPosition;
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
#endif  // __DAVAENGINE_PRIVATETEXTFIELD_WINUAP_H__
