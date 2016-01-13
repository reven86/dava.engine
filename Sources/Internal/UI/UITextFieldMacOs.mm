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

#include "UI/UITextFieldMacOs.h"

#ifdef __DAVAENGINE_MACOS__

#include <AppKit/NSTextField.h>
#include "Utils/UTF8Utils.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
class SingleLineText;
} // end namespace DAVA forward declaration

// objective C declaration may appeared only in global scope
@interface SingleLineDelegate : NSObject<NSTextFieldDelegate>
{
@public
    DAVA::SingleLineText* text;
}
- (id)init;
- (void)dealloc;
@end
// end objective C declarations

namespace DAVA
{
enum class TextMode
{
    SingleLineText,
    MultylineText,
    PasswordText
};

class ITextCtrl
{
public:
    virtual ~ITextCtrl(){};

    virtual void OpenKeyboard() = 0;
    virtual void CloseKeyboard() = 0;
    virtual void GetText(WideString& string) const = 0;
    virtual void SetText(const WideString& string) = 0;
    virtual void UpdateRect(const Rect& rect) = 0;

    virtual void SetTextColor(const DAVA::Color& color) = 0;
    virtual void SetFontSize(float size) = 0;

    virtual void SetTextAlign(DAVA::int32 align) = 0;
    virtual DAVA::int32 GetTextAlign() = 0;
    virtual void SetTextUseRtlAlign(bool useRtlAlign) = 0;
    virtual bool GetTextUseRtlAlign() const = 0;

    virtual void SetVisible(bool value) = 0;
    virtual void ShowField() = 0;
    virtual void HideField() = 0;

    virtual void SetInputEnabled(bool value) = 0;

    // Keyboard traits.
    virtual void SetAutoCapitalizationType(DAVA::int32 value) = 0;
    virtual void SetAutoCorrectionType(DAVA::int32 value) = 0;
    virtual void SetSpellCheckingType(DAVA::int32 value) = 0;
    virtual void SetKeyboardAppearanceType(DAVA::int32 value) = 0;
    virtual void SetKeyboardType(DAVA::int32 value) = 0;
    virtual void SetReturnKeyType(DAVA::int32 value) = 0;
    virtual void SetEnableReturnKeyAutomatically(bool value) = 0;

    // Cursor pos.
    virtual uint32 GetCursorPos() = 0;
    virtual void SetCursorPos(uint32 pos) = 0;

    // Max text length.
    virtual void SetMaxLength(int maxLength) = 0;

    virtual void SetRenderToTexture(bool value) = 0;
    virtual bool IsRenderToTexture() const = 0;
};

class SingleLineText : public ITextCtrl
{
public:
    explicit SingleLineText(UITextField* davaText_)
    {
        davaText = davaText_;
        nsTextField = [[NSTextField alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];

        objcDelegate = [[SingleLineDelegate alloc] init];
        objcDelegate->text = this;

        [nsTextField setValue:objcDelegate forKey:@"delegate"];

        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [openGLView addSubview:nsTextField];
    }

    void OpenKeyboard() override
    {
        [nsTextField becomeFirstResponder];
    }
    void CloseKeyboard() override
    {
        [nsTextField resignFirstResponder];
    }

    void GetText(WideString& string) const override
    {
        NSString* currentText = [nsTextField stringValue];
        const char* cstr = [currentText cStringUsingEncoding:NSUTF8StringEncoding];
        size_t strSize = std::strlen(cstr);
        UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(cstr), strSize, string);
    }
    void SetText(const WideString& string) override
    {
        NSString* text = [[[NSString alloc] initWithBytes:(char*)string.data()
                                                   length:string.size() * sizeof(wchar_t)
                                                 encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)] autorelease];
        [nsTextField setValue:text forKey:@"text"];
    }
    void UpdateRect(const Rect& rect) override
    {
        DAVA::float32 divider = DAVA::Core::Instance()->GetScreenScaleFactor();
        DAVA::Rect physicalRect = DAVA::VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(rect);
        CGRect nativeRect = CGRectMake((physicalRect.x) / divider, (physicalRect.y) / divider, physicalRect.dx / divider, physicalRect.dy / divider);

        nativeRect = CGRectIntegral(nativeRect);
        [nsTextField setFrame:nativeRect];
    }

    void SetTextColor(const DAVA::Color& color) override
    {
        // TODO
    }
    void SetFontSize(float size) override
    {
    }

    void SetTextAlign(DAVA::int32 align) override
    {
        // TODO
    }
    DAVA::int32 GetTextAlign() override
    {
        // TODO
        return 0;
    }
    void SetTextUseRtlAlign(bool useRtlAlign) override
    {
        // TODO
    }
    bool GetTextUseRtlAlign() const override
    {
        // TODO
        return false;
    }

    void SetVisible(bool value) override
    {
        // TODO
    }
    void ShowField() override
    {
        // TODO
    }
    void HideField() override
    {
        // TODO
    }

    void SetInputEnabled(bool value) override
    {
        // TODO
    }

    // Keyboard traits.
    void SetAutoCapitalizationType(DAVA::int32 value) override
    {
        // TODO
    }
    void SetAutoCorrectionType(DAVA::int32 value) override
    {
    }
    void SetSpellCheckingType(DAVA::int32 value) override
    {
    }
    void SetKeyboardAppearanceType(DAVA::int32 value) override
    {
    }
    void SetKeyboardType(DAVA::int32 value) override
    {
    }
    void SetReturnKeyType(DAVA::int32 value) override
    {
    }
    void SetEnableReturnKeyAutomatically(bool value) override
    {
    }

    // Cursor pos.
    uint32 GetCursorPos() override
    {
        // TODO
        return 0;
    }
    void SetCursorPos(uint32 pos) override
    {
        // TODO
    }

    // Max text length.
    void SetMaxLength(int maxLength) override
    {
        // TODO
    }

    void SetRenderToTexture(bool value) override
    {
        // TODO
    }
    bool IsRenderToTexture() const override
    {
        // TODO
        return false;
    }

    UITextField* davaText = nullptr;
    NSTextField* nsTextField = nullptr;
    SingleLineDelegate* objcDelegate = nullptr;
};

class UberTextMacOs
{
public:
    UberTextMacOs(UITextField* davaText_, TextMode mode)
    {
        davaText = davaText_;
        textMode = mode;
        switch (textMode)
        {
        case TextMode::SingleLineText:
            textStrategy = new SingleLineText(davaText);
            break;
        case TextMode::MultylineText:
            Logger::Error("not implemented multiline text use stub");
            textStrategy = new SingleLineText(davaText);
            break;
        case TextMode::PasswordText:
            textStrategy = new SingleLineText(davaText);
            Logger::Error("not implemented passwork text use stub");
            break;
        }
    }
    ~UberTextMacOs()
    {
        delete textStrategy;
        textStrategy = nullptr;
        davaText = nullptr;
    }

private:
    TextMode textMode = TextMode::SingleLineText;
    ITextCtrl* textStrategy = nullptr;
    UITextField* davaText = nullptr;
};

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* tf)
{
    uberText = new UberTextMacOs(tf, TextMode::SingleLineText);
}
TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
}

void TextFieldPlatformImpl::OpenKeyboard()
{
}
void TextFieldPlatformImpl::CloseKeyboard()
{
}
void TextFieldPlatformImpl::GetText(WideString& string) const
{
}
void TextFieldPlatformImpl::SetText(const WideString& string)
{
}
void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
}

void TextFieldPlatformImpl::SetTextColor(const DAVA::Color& color)
{
}
void TextFieldPlatformImpl::SetFontSize(float size)
{
}

void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
{
}
DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return 0;
}
void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
}
bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return false;
}

void TextFieldPlatformImpl::SetVisible(bool value)
{
}
void TextFieldPlatformImpl::TextFieldPlatformImpl::ShowField()
{
}
void TextFieldPlatformImpl::HideField()
{
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
}

// Keyboard traits.
void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
}

// Cursor pos.
uint32 TextFieldPlatformImpl::GetCursorPos()
{
    return 0;
}
void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
}

// Max text length.
void TextFieldPlatformImpl::SetMaxLength(int maxLength)
{
}

void TextFieldPlatformImpl::SetMultiline(bool multiline)
{
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
}
bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return false;
}
void TextFieldPlatformImpl::SystemDraw(const UIGeometricData& geometricData)
{
}

} // end namespace DAVA

@implementation SingleLineDelegate

- (id)init
{
    if (self = [super init])
    {
        text = nullptr;
    }
    return self;
}

- (void)dealloc
{
    text = nullptr;

    [super dealloc];
}

- (BOOL)control:(NSControl*)control
textShouldBeginEditing:(NSText*)fieldEditor
{
    DAVA::Logger::Info("textShouldBeginEditing");
    return YES;
}

- (BOOL)control:(NSControl*)control
textShouldEndEditing:(NSText*)fieldEditor
{
    DAVA::Logger::Info("textShouldEndEditing");
    return YES;
}

- (void)controlTextDidChange:(NSNotification*)aNotification
{
    DAVA::Logger::Info("controlTextDidChange");
}

@end

#endif //__DAVAENGINE_MACOS__