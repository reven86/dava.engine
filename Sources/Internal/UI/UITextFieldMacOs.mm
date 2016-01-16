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
#include <AppKit/NSTextView.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSColor.h>
#include <AppKit/NSText.h>
#include <AppKit/NSFont.h>
#include <AppKit/NSSecureTextField.h>
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

@interface CustomTextFieldFormatter : NSFormatter
{
@public
    int maxLength;
    DAVA::SingleLineText* text;
}
@end

@interface RSVerticallyCenteredTextFieldCell : NSTextFieldCell
{
    BOOL mIsEditingOrSelecting;
}
@end

@interface RSVerticallyCenteredSecureTextFieldCell : NSSecureTextFieldCell
{
    BOOL mIsEditingOrSelecting;
}

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

    virtual void SetMultiline(bool value) = 0;
    virtual bool IsMultiline() const = 0;

    virtual void SetIsPassword(bool value) = 0;
    virtual bool IsPassword() const = 0;

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

        [NSTextField setCellClass:[RSVerticallyCenteredTextFieldCell class]];

        nsTextField = [[NSTextField alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];
        [nsTextField setWantsLayer:YES]; // need to be visible over opengl view

        formatter = [[CustomTextFieldFormatter alloc] init];
        formatter->text = this;
        [nsTextField setFormatter:formatter];

        objcDelegate = [[SingleLineDelegate alloc] init];
        objcDelegate->text = this;

        [nsTextField setDelegate:objcDelegate];

        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [openGLView addSubview:nsTextField];

        [nsTextField setEditable:YES];
        [nsTextField setEnabled:YES];

        // make control border and background transparent
        nsTextField.drawsBackground = NO;
        nsTextField.bezeled = NO;

        [nsTextField.cell setUsesSingleLineMode:YES];

        //        NSColor* backColor = [NSColor
        //        colorWithCalibratedRed:0.f
        //                         green:1.f
        //                          blue:0.f
        //                         alpha:1.f];
        //
        //        [nsTextField setBackgroundColor:backColor];
    }

    ~SingleLineText()
    {
        davaText = nullptr;
        [nsTextField removeFromSuperview];
        [nsTextField release];
        nsTextField = nullptr;
        [formatter release];
        formatter = nullptr;
        [objcDelegate release];
        objcDelegate = nullptr;
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
        [nsTextField setStringValue:text];
    }
    void UpdateRect(const Rect& rect) override
    {
        currentRect = rect;

        float32 divider = Core::Instance()->GetScreenScaleFactor();

        Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize();

        Rect physicalRect = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(rect);

        physicalRect.y = screenSize.dy - (physicalRect.y + physicalRect.dy);

        CGRect nativeRect = CGRectMake((physicalRect.x) / divider,
                                       (physicalRect.y) / divider,
                                       physicalRect.dx / divider,
                                       physicalRect.dy / divider);

        nativeRect = CGRectIntegral(nativeRect);
        [nsTextField setFrame:nativeRect];
    }

    void SetTextColor(const DAVA::Color& color) override
    {
        currentColor = color;

        NSColor* nsColor = [NSColor
        colorWithCalibratedRed:color.r
                         green:color.g
                          blue:color.b
                         alpha:color.a];
        [nsTextField setTextColor:nsColor];

        // make cursor same color as text (default - black caret on white back)
        NSTextView* fieldEditor = (NSTextView*)[nsTextField.window fieldEditor:YES
                                                                     forObject:nsTextField];
        fieldEditor.insertionPointColor = nsColor;
    }
    void SetFontSize(float virtualFontSize) override
    {
        currentFontSize = virtualFontSize;

        // like in win10
        float32 size = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(virtualFontSize);
        size /= Core::Instance()->GetScreenScaleFactor();

        [nsTextField setFont:[NSFont systemFontOfSize:size]];
    }

    void SetTextAlign(DAVA::int32 align) override
    {
        alignment = static_cast<eAlign>(align);

        NSTextAlignment aligment = NSCenterTextAlignment;
        if (align & ALIGN_LEFT)
        {
            aligment = NSLeftTextAlignment;
        }
        else if (align & ALIGN_RIGHT)
        {
            aligment = NSRightTextAlignment;
        }
        else if (align & ALIGN_VCENTER)
        {
            aligment = NSCenterTextAlignment;
        }
        [nsTextField setAlignment:aligment];

        if (align & ALIGN_VCENTER)
        {
            // TODO set custom cell properti - vAlignment
            //[NSSecureTextField setCellClass:[NSSecureTextFieldCell class]];
        }

        if (useRtlAlign && (aligment == NSLeftTextAlignment ||
                            aligment == NSRightTextAlignment))
        {
            [nsTextField setAlignment:NSNaturalTextAlignment];
        }
    }
    DAVA::int32 GetTextAlign() override
    {
        return alignment;
    }
    void SetTextUseRtlAlign(bool useRtlAlign_) override
    {
        useRtlAlign = useRtlAlign_;
        SetTextAlign(alignment);
    }
    bool GetTextUseRtlAlign() const override
    {
        return useRtlAlign;
    }

    void SetVisible(bool value) override
    {
        [nsTextField setHidden:!value];
    }
    void ShowField() override
    {
        // TODO do I need it on mac os?
    }
    void HideField() override
    {
        // TODO do I need it on mac os?
    }

    void SetInputEnabled(bool value) override
    {
        [nsTextField setEditable:value];
    }

    // Keyboard traits.
    void SetAutoCapitalizationType(DAVA::int32 value) override
    {
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
        uint32 cursorPos = 0;
        if ([nsTextField isEditable])
        {
            NSText* text = [nsTextField currentEditor];
            NSRange range = [text selectedRange];
            cursorPos = range.location;
        }
        return cursorPos;
    }
    void SetCursorPos(uint32 pos) override
    {
        if ([nsTextField isEditable])
        {
            NSText* text = [nsTextField currentEditor];
            [text setSelectedRange:(NSRange){ pos, 0 }];
        }
    }

    void SetMultiline(bool value) override
    {
        multiline = value;
        [nsTextField.cell setUsesSingleLineMode:!multiline];
    }
    bool IsMultiline() const override
    {
        return multiline;
    }

    void SetIsPassword(bool value) override
    {
        if (IsPassword() != value)
        {
            WideString oldText;
            GetText(oldText);

            NSTextField* oldCtrl = nsTextField;
            if (value)
            {
                [NSTextField setCellClass:[RSVerticallyCenteredSecureTextFieldCell class]];
            }
            else
            {
                [NSTextField setCellClass:[RSVerticallyCenteredTextFieldCell class]];
            }

            nsTextField = [[NSTextField alloc] initWithFrame:[oldCtrl frame]];

            [nsTextField setWantsLayer:YES]; // need to be visible over opengl view
            [nsTextField setFormatter:formatter];
            [nsTextField setDelegate:objcDelegate];

            [nsTextField setEditable:oldCtrl.editable];
            [nsTextField setEnabled:oldCtrl.enabled];

            // make control border and background transparent
            nsTextField.drawsBackground = NO;
            nsTextField.bezeled = NO;

            NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
            [openGLView addSubview:nsTextField];

            // copy all current properties
            SetFontSize(currentFontSize);
            SetTextColor(currentColor);
            SetText(oldText);
            SetTextUseRtlAlign(useRtlAlign);
            SetTextAlign(alignment);

            [oldCtrl removeFromSuperview];
            [oldCtrl release];
        }
    }
    bool IsPassword() const override
    {
        return [nsTextField isKindOfClass:[NSSecureTextField class]];
    }

    // Max text length.
    void SetMaxLength(int maxLength) override
    {
        formatter->maxLength = maxLength;
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
    CustomTextFieldFormatter* formatter = nullptr;

    Rect currentRect;
    Color currentColor;
    float32 currentFontSize = 0.f;

    eAlign alignment = ALIGN_LEFT;
    bool useRtlAlign = false;
    bool multiline = false;
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

    ITextCtrl* operator->()
    {
        return textStrategy;
    }

private:
    TextMode textMode = TextMode::SingleLineText;
    ITextCtrl* textStrategy = nullptr;
    UITextField* davaText = nullptr;
};

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* tf)
    : uberText{ *(new UberTextMacOs(tf, TextMode::SingleLineText)) }
{
}
TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    delete &uberText;
}

void TextFieldPlatformImpl::OpenKeyboard()
{
    uberText->OpenKeyboard();
}
void TextFieldPlatformImpl::CloseKeyboard()
{
    uberText->CloseKeyboard();
}
void TextFieldPlatformImpl::GetText(WideString& string) const
{
    uberText->GetText(string);
}
void TextFieldPlatformImpl::SetText(const WideString& string)
{
    uberText->SetText(string);
}
void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
    uberText->UpdateRect(rect);
}

void TextFieldPlatformImpl::SetTextColor(const DAVA::Color& color)
{
    uberText->SetTextColor(color);
}
void TextFieldPlatformImpl::SetFontSize(float size)
{
    uberText->SetFontSize(size);
}

void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
{
    uberText->SetTextAlign(align);
}
DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return uberText->GetTextAlign();
}
void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
    uberText->SetTextUseRtlAlign(useRtlAlign);
}
bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return uberText->GetTextUseRtlAlign();
}
void TextFieldPlatformImpl::SetVisible(bool value)
{
    uberText->SetVisible(value);
}
void TextFieldPlatformImpl::TextFieldPlatformImpl::ShowField()
{
    uberText->ShowField();
}
void TextFieldPlatformImpl::HideField()
{
    uberText->HideField();
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
    uberText->SetIsPassword(isPassword);
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
    uberText->SetInputEnabled(value);
}

// Keyboard traits.
void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
    uberText->SetAutoCapitalizationType(value);
}
void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
{
    uberText->SetAutoCorrectionType(value);
}
void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
{
    uberText->SetSpellCheckingType(value);
}
void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
    uberText->SetKeyboardAppearanceType(value);
}
void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
{
    uberText->SetKeyboardType(value);
}
void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
{
    uberText->SetReturnKeyType(value);
}
void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
    uberText->SetEnableReturnKeyAutomatically(value);
}

// Cursor pos.
uint32 TextFieldPlatformImpl::GetCursorPos()
{
    return uberText->GetCursorPos();
}
void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
    uberText->SetCursorPos(pos);
}
// Max text length.
void TextFieldPlatformImpl::SetMaxLength(int maxLength)
{
    uberText->SetMaxLength(maxLength);
}
void TextFieldPlatformImpl::SetMultiline(bool multiline)
{
    uberText->SetMultiline(multiline);
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
    uberText->SetRenderToTexture(value);
}
bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return uberText->IsRenderToTexture();
}
void TextFieldPlatformImpl::SystemDraw(const UIGeometricData& geometricData)
{
    // TODO do I need to update rect here? Like in iOS version?
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
    return YES;
}

- (BOOL)control:(NSControl*)control
textShouldEndEditing:(NSText*)fieldEditor
{
    return YES;
}

- (void)controlTextDidChange:(NSNotification*)aNotification
{
}

// https://developer.apple.com/library/mac/qa/qa1454/_index.html
- (BOOL)control:(NSControl*)control
           textView:(NSTextView*)textView
doCommandBySelector:(SEL)commandSelector
{
    BOOL result = NO;

    if (commandSelector == @selector(insertNewline:))
    {
        if (text->IsMultiline())
        {
            // new line action:
            // always insert a line-break character and dont cause the receiver to end editing
            [textView insertNewlineIgnoringFieldEditor:self];
            result = YES;
        }
        else
        {
            text->davaText->GetDelegate()->TextFieldShouldReturn(text->davaText);
        }
    }

    return result;
}

@end

@implementation CustomTextFieldFormatter

- (id)init
{
    if (self = [super init])
    {
        maxLength = INT_MAX;
    }

    return self;
}

- (NSString*)stringForObjectValue:(id)object
{
    return (NSString*)object;
}

- (BOOL)getObjectValue:(id*)object
             forString:(NSString*)string
      errorDescription:(NSString**)error
{
    *object = string;
    return YES;
}

- (BOOL)isPartialStringValid:(NSString**)partialStringPtr
       proposedSelectedRange:(NSRangePointer)proposedSelRangePtr
              originalString:(NSString*)origString
       originalSelectedRange:(NSRange)origSelRange
            errorDescription:(NSString**)error
{
    if ([*partialStringPtr length] > maxLength)
    {
        return NO;
    }

    if (text != nullptr)
    {
        DAVA::UITextField* davaCtrl = text->davaText;
        DAVA::UITextFieldDelegate* delegate = davaCtrl->GetDelegate();
        if (delegate != nullptr)
        {
            // simple change whole string for new string
            NSString* nsReplacement = *partialStringPtr;
            DAVA::WideString replacement;
            const char* cstr = [nsReplacement cStringUsingEncoding:NSUTF8StringEncoding];
            size_t strSize = std::strlen(cstr);
            DAVA::UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(cstr), strSize, replacement);

            DAVA::int32 location = 0;
            DAVA::int32 length = [origString length];
            bool result = delegate->TextFieldKeyPressed(davaCtrl, location, length, replacement);
            if (!result)
            {
                return NO;
            }
            else
            {
                DAVA::WideString oldText;
                text->GetText(oldText);
                delegate->TextFieldOnTextChanged(davaCtrl, replacement, oldText);
            }
        }
    }

    return YES;
}

- (NSAttributedString*)attributedStringForObjectValue:(id)anObject
                                withDefaultAttributes:(NSDictionary*)attributes
{
    return nil;
}

@end

@implementation RSVerticallyCenteredTextFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
    // Get the parent's idea of where we should draw
    NSRect newRect = [super drawingRectForBounds:theRect];

    // When the text field is being
    // edited or selected, we have to turn off the magic because it screws up
    // the configuration of the field editor.  We sneak around this by
    // intercepting selectWithFrame and editWithFrame and sneaking a
    // reduced, centered rect in at the last minute.
    if (mIsEditingOrSelecting == NO)
    {
        // Get our ideal size for current text
        NSSize textSize = [self cellSizeForBounds:theRect];

        // Center that in the proposed rect
        float heightDelta = newRect.size.height - textSize.height;
        if (heightDelta > 0)
        {
            newRect.size.height -= heightDelta;
            newRect.origin.y += (heightDelta / 2);
        }
    }

    return newRect;
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView*)controlView editor:(NSText*)textObj delegate:(id)anObject start:(long)selStart length:(long)selLength
{
    aRect = [self drawingRectForBounds:aRect];
    mIsEditingOrSelecting = YES;
    [super selectWithFrame:aRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
    mIsEditingOrSelecting = NO;
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView*)controlView editor:(NSText*)textObj delegate:(id)anObject event:(NSEvent*)theEvent
{
    aRect = [self drawingRectForBounds:aRect];
    mIsEditingOrSelecting = YES;
    [super editWithFrame:aRect inView:controlView editor:textObj delegate:anObject event:theEvent];
    mIsEditingOrSelecting = NO;
}

@end

@implementation RSVerticallyCenteredSecureTextFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
    // Get the parent's idea of where we should draw
    NSRect newRect = [super drawingRectForBounds:theRect];

    // When the text field is being
    // edited or selected, we have to turn off the magic because it screws up
    // the configuration of the field editor.  We sneak around this by
    // intercepting selectWithFrame and editWithFrame and sneaking a
    // reduced, centered rect in at the last minute.
    if (mIsEditingOrSelecting == NO)
    {
        // Get our ideal size for current text
        NSSize textSize = [self cellSizeForBounds:theRect];

        // Center that in the proposed rect
        float heightDelta = newRect.size.height - textSize.height;
        if (heightDelta > 0)
        {
            newRect.size.height -= heightDelta;
            newRect.origin.y += (heightDelta / 2);
        }
    }

    return newRect;
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView*)controlView editor:(NSText*)textObj delegate:(id)anObject start:(long)selStart length:(long)selLength
{
    aRect = [self drawingRectForBounds:aRect];
    mIsEditingOrSelecting = YES;
    [super selectWithFrame:aRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
    mIsEditingOrSelecting = NO;
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView*)controlView editor:(NSText*)textObj delegate:(id)anObject event:(NSEvent*)theEvent
{
    aRect = [self drawingRectForBounds:aRect];
    mIsEditingOrSelecting = YES;
    [super editWithFrame:aRect inView:controlView editor:textObj delegate:anObject event:theEvent];
    mIsEditingOrSelecting = NO;
}

@end

#endif //__DAVAENGINE_MACOS__