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

PrivateTextFieldWinUAP::PrivateTextFieldWinUAP(UITextField* uiTextField_)
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , uiTextField(uiTextField_)
{
    core->RunOnUIThreadBlocked([this]() {
        nativeControl = ref new TextBox();
        nativeControl->TextAlignment = TextAlignment::Left;
        nativeControl->FlowDirection = FlowDirection::LeftToRight;

        nativeControl->Background = ref new SolidColorBrush(Colors::Transparent);
        nativeControl->BorderBrush = ref new SolidColorBrush(Colors::Transparent);

        core->XamlApplication()->AddUIElement(nativeControl);
        PositionNative(originalRect, true);

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

void PrivateTextFieldWinUAP::SetIsPassword(bool isPassword_)
{

}

void PrivateTextFieldWinUAP::SetMaxLength(int32 value)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, value]()
    {
        // Native control expects zero for unlimited input length
        nativeControl->MaxLength = std::max(0, value);
    });
}

void PrivateTextFieldWinUAP::OpenKeyboard()
{
    // Focus native control as if mouse has been pressed to show keyboard
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self]()
    {
        if (!multiline)
            PositionNative(originalRect, false);
        nativeControl->Focus(FocusState::Pointer);
    });
}

void PrivateTextFieldWinUAP::CloseKeyboard()
{
    // Hide keyboard through unfocusing native control
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self](){
        core->XamlApplication()->UnfocusUIElement();
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

    if (multiline || pendingTextureUpdate)
    {
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, sizeChanged]()
        {
            if (sizeChanged)
                PositionNative(originalRect, !multiline);
            if (pendingTextureUpdate && !multiline)
                RenderToTexture();
            pendingTextureUpdate = false;
        });
    }
}

void PrivateTextFieldWinUAP::SetText(const WideString& text)
{
    Platform::String^ str = ref new Platform::String(text.c_str());
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, str]()
    {
        curText = str->Data();
        nativeControl->Text = str;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::GetText(WideString& text) const
{
    core->RunOnUIThreadBlocked([this, &text]()
    {
        text = nativeControl->Text->Data();
    });
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
    core->RunOnUIThread([this, self]()
    {
        if (textAlignment & ALIGN_LEFT)
            nativeControl->TextAlignment = TextAlignment::Left;
        else if (textAlignment & ALIGN_HCENTER)
            nativeControl->TextAlignment = TextAlignment::Center;
        else if (textAlignment & ALIGN_RIGHT)
            nativeControl->TextAlignment = TextAlignment::Right;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::SetTextUseRtlAlign(bool useRtlAlign)
{
    if (flowDirectionRTL != useRtlAlign)
    {
        flowDirectionRTL = useRtlAlign;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]()
        {
            nativeControl->FlowDirection = flowDirectionRTL ? FlowDirection::RightToLeft : FlowDirection::LeftToRight;
            pendingTextureUpdate = true;
        });
    }
}

void PrivateTextFieldWinUAP::SetFontSize(float32 size)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, size]()
    {
        nativeControl->FontSize = size;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::SetDelegate(UITextFieldDelegate* textFieldDelegate_)
{
    textFieldDelegate = textFieldDelegate_;
}

void PrivateTextFieldWinUAP::SetMultiline(bool enable)
{
    if (multiline != enable)
    {
        multiline = enable;
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self]()
        {
            nativeControl->AcceptsReturn = multiline;
            PositionNative(originalRect, !multiline);
        });
    }
}

void PrivateTextFieldWinUAP::SetInputEnabled(bool enable)
{
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, enable]()
    {
        nativeControl->IsReadOnly = enable;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::SetSpellCheckingType(int32 value)
{
    bool enabled = UITextField::SPELL_CHECKING_TYPE_YES == value;
    auto self{shared_from_this()};
    core->RunOnUIThread([this, self, enabled]()
    {
        nativeControl->IsSpellCheckEnabled = enabled;
        pendingTextureUpdate = true;
    });
}

void PrivateTextFieldWinUAP::SetAutoCapitalizationType(int32 value)
{}

void PrivateTextFieldWinUAP::SetAutoCorrectionType(int32 value)
{}

void PrivateTextFieldWinUAP::SetKeyboardAppearanceType(int32 value)
{}

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

void PrivateTextFieldWinUAP::SetReturnKeyType(int32 /*value*/)
{}

void PrivateTextFieldWinUAP::SetEnableReturnKeyAutomatically(bool /*value*/)
{}

void PrivateTextFieldWinUAP::SetCursorPos(uint32 pos)
{
    if (static_cast<int32>(pos) >= 0)
    {
        auto self{shared_from_this()};
        core->RunOnUIThread([this, self, pos]()
        {
            nativeControl->SelectionStart = pos;
        });
    }
}

void PrivateTextFieldWinUAP::InstallEventHandlers()
{
    auto keyDown = ref new KeyEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args) {
        OnKeyDown(sender, args);
    });
    auto selectionChanged = ref new RoutedEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args) {
        OnSelectionChanged(sender, args);
    });
    auto textChanged = ref new TextChangedEventHandler([this](Platform::Object^ sender, TextChangedEventArgs^ args) {
        OnTextChanged(sender, args);
    });
    auto lostFocus = ref new RoutedEventHandler([this](Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args) {
        OnLostFocus(sender, args);
    });
    nativeControl->KeyDown += keyDown;
    nativeControl->SelectionChanged += selectionChanged;
    nativeControl->TextChanged += textChanged;
    nativeControl->LostFocus += lostFocus;

    auto keyboardHiding = ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>([this](InputPane^ sender, InputPaneVisibilityEventArgs^ args) {
        OnKeyboardHiding(sender, args);
    });
    auto keyboardShowing = ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>([this](InputPane^ sender, InputPaneVisibilityEventArgs^ args) {
        OnKeyboardShowing(sender, args);
    });
    InputPane::GetForCurrentView()->Showing += keyboardShowing;
    InputPane::GetForCurrentView()->Hiding += keyboardHiding;
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
    savedCaretPosition = nativeControl->SelectionStart;

    VirtualKey vk = args->Key;
    switch (vk)
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

void PrivateTextFieldWinUAP::OnSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args)
{
    caretPosition = nativeControl->SelectionStart;
}

void PrivateTextFieldWinUAP::OnTextChanged(Platform::Object^ sender, TextChangedEventArgs^ args)
{
    if (ignoreTextChange)
    {
        ignoreTextChange = false;
        return;
    }

    StringDiffResult diffR;
    WideString newText(nativeControl->Text->Data());
    StringDiff(curText, newText, diffR);
    if (StringDiffResult::NO_CHANGE == diffR.diffType)
        return;

    // Ask delegate whether to accept changes
    bool decline = false;
    auto self{shared_from_this()};
    core->RunOnMainThreadBlocked([this, self, &diffR, &decline]()
    {
        if (uiTextField != nullptr && textFieldDelegate != nullptr)
        {
            decline = !textFieldDelegate->TextFieldKeyPressed(uiTextField,
                                                              diffR.originalStringDiffPosition,
                                                              static_cast<int32>(diffR.originalStringDiff.length()),
                                                              diffR.newStringDiff);
        }
    });

    if (decline)
    {
        nativeControl->Text = ref new Platform::String(curText.c_str());
        // Restore caret position to position before text has been changed
        nativeControl->SelectionStart = savedCaretPosition;
        ignoreTextChange = true;
    }
    else
    {
        WideString curTextCopy = curText;
        WideString newTextCopy = newText;
        core->RunOnMainThread([this, self, curTextCopy, newTextCopy]()
        {
            if (textFieldDelegate != nullptr)
                textFieldDelegate->TextFieldOnTextChanged(uiTextField, newTextCopy, curTextCopy);
        });
        curText = newText;
    }
}

void PrivateTextFieldWinUAP::OnLostFocus(Platform::Object^ sender, RoutedEventArgs^ args)
{
    if (!multiline)
    {
        PositionNative(originalRect, true);
        RenderToTexture();
    }
}

void PrivateTextFieldWinUAP::OnKeyboardHiding(InputPane^ sender, InputPaneVisibilityEventArgs^ args)
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

void PrivateTextFieldWinUAP::OnKeyboardShowing(InputPane^ sender, InputPaneVisibilityEventArgs^ args)
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
        std::vector<uint8> buf(streamSize, 0);
        while (reader->UnconsumedBufferLength > 0)
        {
            buf[index] = reader->ReadByte();
            index += 1;
        }

        RefPtr<Sprite> sprite(CreateSpriteFromPreviewData(buf.data(), imageWidth, imageHeight));
        if (sprite.Valid())
        {
            core->RunOnMainThread([this, sprite]() {
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

Sprite* PrivateTextFieldWinUAP::CreateSpriteFromPreviewData(const uint8* imageData, int32 width, int32 height) const
{
    RefPtr<Image> imgSrc(Image::CreateFromData(width, height, FORMAT_RGBA8888, imageData));
    RefPtr<Image> imgDst(Image::Create(width, height, FORMAT_RGB888));
    if (imgSrc.Valid() && imgDst.Valid())
    {
        ImageConvert::ConvertImageDirect(imgSrc.Get(), imgDst.Get());
        return Sprite::CreateFromImage(imgDst.Get());
    }
    return nullptr;
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
