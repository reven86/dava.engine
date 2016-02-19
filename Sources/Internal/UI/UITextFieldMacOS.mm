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

#include "UI/UITextFieldMacOS.h"

#ifdef __DAVAENGINE_MACOS__

#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSTextView.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSText.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSSecureTextField.h>
#include "Utils/UTF8Utils.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"

// +-----------+          +------+
// |ObjCWrapper+----------+IField|
// +-----+-----+          +----+-+
// |                 +-----^   ^
// |                 |         |
// |                 |         |
// |                 |         |
// |         +-------+------+  |             +-----------------+
// |         |MultilineField+----------------+MultilineDelegate|
// |         +--------------+  |             +-----------------+
// |                           |
// |                           |
// |                           |
// |         +-----------------+-------+     +---------------------------------+
// |         |SingleLineOrPasswordField+-----+CustomTextField,                 |
// |         +-------------------------+     |CustomDelegate,                  |
// |                                         |CustomTextFieldFormatter,        |
// |                                         |RSVerticallyCenteredTextFieldCell|
// |                                         +---------------------------------+
// |
// |
// |
// +----------+----------+
// |TextFieldPlatformImpl|
// +---------------------+

// objective C declaration may appeared only in global scope
@interface MultilineDelegate : NSObject<NSTextViewDelegate>
{
@public
    DAVA::ObjCWrapper* text;
    int maxLength;
}
@end

@interface CustomTextField : NSTextField
{
}
@end

@interface CustomTextView : NSTextView
{
}
@end

@interface CustomTextFieldFormatter : NSFormatter
{
@public
    int maxLength;
    DAVA::ObjCWrapper* text;
}
@end

@interface CustomDelegate : NSObject<NSTextFieldDelegate>
{
@public
    DAVA::ObjCWrapper* text;
    CustomTextFieldFormatter* formatter;
}
- (id)init;
- (void)dealloc;
@end

@interface RSVerticallyCenteredTextFieldCell : NSTextFieldCell
{
@public
    BOOL isEditingOrSelecting;
    BOOL isMultilineControl;
}
@end

@interface RSVerticallyCenteredSecureTextFieldCell : NSSecureTextFieldCell
{
@public
    BOOL mIsEditingOrSelecting;
}

@end
// end objective C declarations

namespace DAVA
{
class IField
{
public:
    IField(UITextField* davaText_, ObjCWrapper* wrapper_)
        : davaText(davaText_)
        , wrapper(wrapper_)
    {
        DVASSERT(davaText != nullptr);
        DVASSERT(wrapper != nullptr);
    }
    virtual ~IField()
    {
        davaText = nullptr;
        wrapper = nullptr;
    }
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

    virtual void SetIsPassword(bool isPassword) = 0;

    virtual void SetInputEnabled(bool value) = 0;

    // Keyboard traits.
    void SetAutoCapitalizationType(DAVA::int32 value)
    {
        // not supported implement on client in delegate
    }

    void SetAutoCorrectionType(DAVA::int32 value)
    {
        // not supported implement on client in delegate
    }

    void SetSpellCheckingType(DAVA::int32 value)
    {
        // not supported for NSTextField
        // we can implement it in NSTextView with property
        // setContinuousSpellCheckingEnabled:YES
        // but does we really need it?
    }

    void SetKeyboardAppearanceType(DAVA::int32 value)
    {
        // not aplicable on mac os with hardware keyboard
    }

    void SetKeyboardType(DAVA::int32 value)
    {
        // not aplicable on mac os with hardware keyboard
    }

    void SetReturnKeyType(DAVA::int32 value)
    {
        // not aplicable on mac os with hardware keyboard
    }

    void SetEnableReturnKeyAutomatically(bool value)
    {
        // not aplicable on mac os with hardware keyboard
    }

    // Cursor pos.
    virtual uint32 GetCursorPos() = 0;
    virtual void SetCursorPos(uint32 pos) = 0;

    // Max text length.
    virtual void SetMaxLength(int maxLength) = 0;

    virtual void SetMultiline(bool multiline) = 0;
    virtual bool IsMultiline() const = 0;

    virtual void SetRenderToTexture(bool value) = 0;
    virtual bool IsRenderToTexture() const = 0;

    // return true on success
    bool RenderToTextureAndSetAsBackgroundSprite(NSView* nsView)
    {
        // https://developer.apple.com/library/mac/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_texturedata/opengl_texturedata.html

        NSBitmapImageRep* imageRep = nullptr;
        imageRep = [nsView bitmapImageRepForCachingDisplayInRect:[nsView frame]]; // 1

        if (nullptr == imageRep)
        {
            davaText->SetSprite(nullptr, 0);
            return false;
        }

        NSSize imageRepSize = [imageRep size];
        NSRect imageRect = NSMakeRect(0.f, 0.f, imageRepSize.width, imageRepSize.height);

        // render web view into bitmap image
        [nsView cacheDisplayInRect:imageRect toBitmapImageRep:imageRep]; // 2

        const uint8* rawData = [imageRep bitmapData];
        const int w = [imageRep pixelsWide];
        const int h = [imageRep pixelsHigh];
        const int BPP = [imageRep bitsPerPixel];
        const int pitch = [imageRep bytesPerRow];

        PixelFormat format = FORMAT_INVALID;
        if (24 == BPP)
        {
            format = FORMAT_RGB888;
        }
        else if (32 == BPP)
        {
            DVASSERT(!([imageRep bitmapFormat] & NSAlphaFirstBitmapFormat));
            format = FORMAT_RGBA8888;
        }
        else
        {
            DVASSERT(false && "[nsView] Unexpected bits per pixel value");
            davaText->SetSprite(nullptr, 0);
            return false;
        }

        {
            RefPtr<Image> imageRGB;
            int bytesPerLine = w * (BPP / 8);

            if (pitch == bytesPerLine)
            {
                imageRGB = Image::CreateFromData(w, h, format, rawData);
            }
            else
            {
                imageRGB = Image::Create(w, h, format);
                uint8* pixels = imageRGB->GetData();

                // copy line by line image
                for (int y = 0; y < h; ++y)
                {
                    uint8* dstLineStart = &pixels[y * bytesPerLine];
                    const uint8* srcLineStart = &rawData[y * pitch];
                    Memcpy(dstLineStart, srcLineStart, bytesPerLine);
                }
            }

            DVASSERT(imageRGB);
            {
                RefPtr<Texture> tex(Texture::CreateFromData(imageRGB.Get(), false));
                const Rect& rect = davaText->GetRect();
                {
                    RefPtr<Sprite> sprite(Sprite::CreateFromTexture(tex.Get(), 0, 0, w, h, rect.dx, rect.dy));
                    davaText->SetSprite(sprite.Get(), 0);
                }
            }
        }
        return true;
    }

    SigConnectionID signalMinimizeRestored;

    UITextField* davaText = nullptr;
    ObjCWrapper* wrapper = nullptr;

    Rect currentRect;
    Color currentColor;
    float32 currentFontSize = 0.f;

    eAlign alignment = ALIGN_LEFT;
    bool useRtlAlign = false;
    bool multiline = false;
    bool password = false;
    bool renderInTexture = false;
    bool updateViewState = true;

    bool isKeyboardOpened = false; // HACK to prevent endless recursion
    bool insideTextShouldReturn = false; // HACK mark what happened
    NSRect nativeControlRect = NSMakeRect(0, 0, 0, 0);
};

static NSRect ConvertToNativeWindowRect(Rect rectSrc)
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    Rect rect = coordSystem->ConvertVirtualToInput(rectSrc);

    NSView* openglView = static_cast<NSView*>(Core::Instance()->GetNativeView());
    rect.y = openglView.frame.size.height - (rect.y + rect.dy);
    return NSMakeRect(rect.x, rect.y, rect.dx, rect.dy);
}

class MultilineField : public IField
{
public:
    MultilineField(UITextField* davaText_, ObjCWrapper* wrapper_)
        : IField(davaText_, wrapper_)
    {
        nsScrollView = [[NSScrollView alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];

        [nsScrollView setBorderType:NSNoBorder];
        [nsScrollView setHasVerticalScroller:YES];
        [nsScrollView setHasHorizontalScroller:NO];
        [nsScrollView setAutoresizingMask:NSViewWidthSizable |
                      NSViewHeightSizable];

        nsTextView = [[CustomTextView alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];
        [nsTextView setWantsLayer:YES]; // need to be visible over opengl view

        objcDelegate = [[MultilineDelegate alloc] init];
        objcDelegate->text = wrapper;

        [nsTextView setDelegate:objcDelegate];

        [nsTextView setEditable:YES];

        // make control border and background transparent
        nsTextView.drawsBackground = NO;

        [nsScrollView setDocumentView:nsTextView];

        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [openGLView addSubview:nsScrollView];

        CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
        signalMinimizeRestored = xcore->signalAppMinimizedRestored.Connect(this, &MultilineField::OnAppMinimazedResored);
    }

    ~MultilineField()
    {
        CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
        xcore->signalAppMinimizedRestored.Disconnect(signalMinimizeRestored);

        [nsScrollView removeFromSuperview];
        [nsScrollView release];
        nsScrollView = nullptr;
        [nsTextView removeFromSuperview];
        [nsTextView release];
        nsTextView = nullptr;

        [objcDelegate release];
        objcDelegate = nullptr;
    }

    void OnAppMinimazedResored(bool value)
    {
        SetVisible(!value);
    }

    void OpenKeyboard() override
    {
        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [openGLView.window makeFirstResponder:nsTextView];

        UITextFieldDelegate* delegate = davaText->GetDelegate();

        if (delegate && !isKeyboardOpened)
        {
            isKeyboardOpened = true;
            Rect emptyRect;
            emptyRect.y = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
            delegate->OnKeyboardShown(emptyRect);
        }
    }

    void CloseKeyboard() override
    {
        UITextFieldDelegate* delegate = davaText->GetDelegate();
        if (delegate && isKeyboardOpened)
        {
            isKeyboardOpened = false;
            delegate->OnKeyboardHidden();
        }

        // http://stackoverflow.com/questions/4881676/changing-focus-from-nstextfield-to-nsopenglview
        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [[NSApp keyWindow] makeFirstResponder:openGLView];
    }

    void GetText(WideString& string) const override
    {
        NSString* currentText = [nsTextView string];
        const char* cstr = [currentText cStringUsingEncoding:NSUTF8StringEncoding];
        size_t strSize = std::strlen(cstr);
        UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(cstr), strSize, string);
    }

    void SetText(const WideString& string) override
    {
        NSString* text = [[[NSString alloc] initWithBytes:(char*)string.data()
                                                   length:string.size() * sizeof(wchar_t)
                                                 encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)] autorelease];
        [nsTextView setString:text];
    }

    void UpdateRect(const Rect& rectSrc) override
    {
        if (currentRect != rectSrc)
        {
            currentRect = rectSrc;
            NSRect controlRect = ConvertToNativeWindowRect(rectSrc);

            [nsScrollView setFrame:controlRect];
            [nsTextView setFrame:controlRect];
        }
    }

    void SetTextColor(const DAVA::Color& color) override
    {
        currentColor = color;

        NSColor* nsColor = [NSColor
        colorWithCalibratedRed:color.r
                         green:color.g
                          blue:color.b
                         alpha:color.a];
        [nsTextView setTextColor:nsColor];

        [nsTextView setInsertionPointColor:nsColor];
    }

    void SetFontSize(float virtualFontSize) override
    {
        currentFontSize = virtualFontSize;

        float32 size = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(virtualFontSize);

        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        NSSize origSz = NSMakeSize(size, 0);
        NSSize convSz = [openGLView convertSizeFromBacking:origSz];

        [nsTextView setFont:[NSFont systemFontOfSize:convSz.width]];
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
        [nsTextView setAlignment:aligment];

        // TODO investigate do we need to implement other then ALIGN_VCENTER
        // several time set align so comment it for now DVASSERT(align & ALIGN_VCENTER);
        if (align & ALIGN_VCENTER)
        {
            // TODO set custom cell properti - vAlignment
            //[NSSecureTextField setCellClass:[NSSecureTextFieldCell class]];
        }

        if (useRtlAlign && (aligment == NSLeftTextAlignment ||
                            aligment == NSRightTextAlignment))
        {
            [nsTextView setAlignment:NSNaturalTextAlignment];
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
        [nsScrollView setHidden:!value];
    }

    void ShowField() override
    {
        // we always on screen
    }
    void HideField() override
    {
        // we always on screen
    }

    void SetInputEnabled(bool value) override
    {
        [nsTextView setEditable:value];
    }

    // Cursor pos.
    uint32 GetCursorPos() override
    {
        NSInteger insertionPoint = [[[nsTextView selectedRanges] objectAtIndex:0] rangeValue].location;
        return insertionPoint;
    }

    void SetCursorPos(uint32 pos) override
    {
        [nsTextView setSelectedRange:NSMakeRange(pos, 0)];
    }

    void SetMultiline(bool value) override
    {
        multiline = value;
        DVASSERT(multiline);
    }

    bool IsMultiline() const override
    {
        return multiline;
    }

    void SetIsPassword(bool value) override
    {
        DVASSERT(!value);
        password = value;
    }

    bool IsPassword() const
    {
        return password;
    }

    // Max text length.
    void SetMaxLength(int maxLength) override
    {
        objcDelegate->maxLength = maxLength;
    }

    void SetRenderToTexture(bool value) override
    {
        static bool alreadyPrintLog = false;
        if (!alreadyPrintLog)
        {
            alreadyPrintLog = true;
            Logger::FrameworkDebug("UITextField::SetRenderTotexture not implemented on macos");
        }
    }

    bool IsRenderToTexture() const override
    {
        return false;
    }

    NSScrollView* nsScrollView = nullptr;
    NSTextView* nsTextView = nullptr;
    MultilineDelegate* objcDelegate = nullptr;
};

class SingleLineOrPasswordField : public IField
{
public:
    explicit SingleLineOrPasswordField(UITextField* davaText_, ObjCWrapper* wrapper_)
        : IField(davaText_, wrapper_)
    {
        [CustomTextField setCellClass:[RSVerticallyCenteredTextFieldCell class]];
        nsTextField = [[CustomTextField alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];
        [nsTextField setWantsLayer:YES]; // need to be visible over opengl view
        formatter = [[CustomTextFieldFormatter alloc] init];
        formatter->text = wrapper;
        [nsTextField setFormatter:formatter];
        objcDelegate = [[CustomDelegate alloc] init];
        objcDelegate->text = wrapper;
        objcDelegate->formatter = formatter;

        [nsTextField setDelegate:objcDelegate];

        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [openGLView addSubview:nsTextField];

        [nsTextField setEditable:YES];
        [nsTextField setEnabled:YES];
        // make control border and background transparent
        nsTextField.drawsBackground = NO;
        nsTextField.bezeled = NO;

        CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
        signalMinimizeRestored = xcore->signalAppMinimizedRestored.Connect(this, &SingleLineOrPasswordField::OnAppMinimazedResored);
        SetMultiline(false);
    }

    ~SingleLineOrPasswordField()
    {
        CoreMacOSPlatform* xcore = static_cast<CoreMacOSPlatform*>(Core::Instance());
        xcore->signalAppMinimizedRestored.Disconnect(signalMinimizeRestored);

        [nsTextField removeFromSuperview];
        [nsTextField release];
        nsTextField = nullptr;
        [formatter release];
        formatter = nullptr;
        [objcDelegate release];
        objcDelegate = nullptr;
    }

    void OnAppMinimazedResored(bool value)
    {
        SetVisible(!value);
    }

    void OpenKeyboard() override
    {
        nsTextField.enabled = YES;

        UITextFieldDelegate* delegate = davaText->GetDelegate();

        if (delegate && !isKeyboardOpened)
        {
            isKeyboardOpened = true;
            Rect emptyRect;
            emptyRect.y = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
            delegate->OnKeyboardShown(emptyRect);
        }

        [[NSApp keyWindow] makeFirstResponder:nsTextField];

        //        // HACK try set text and then remove it to show cursor
        //        if ([[nsTextField stringValue] length] == 0)
        //        {
        //            [nsTextField setStringValue:@"a"];
        //            [nsTextField setStringValue:@""];
        //        }

        // first attempt set cursor
        [nsTextField selectText:nsTextField];
        NSRange range = [[nsTextField currentEditor] selectedRange];
        [[nsTextField currentEditor] setSelectedRange:NSMakeRange(range.length, 0)];

        // second attemt set cursor
        //        NSText* textEditor = [nsTextField.window fieldEditor:YES forObject:nsTextField];
        //        if (textEditor)
        //        {
        //            id cell = [nsTextField selectedCell];
        //            [cell selectWithFrame:[nsTextField bounds]
        //                           inView:nsTextField
        //                           editor:textEditor
        //                         delegate:nsTextField
        //                            start:range.length
        //                           length:0];
        //        }

        // on mac os all NSTextField controls share same NSTextView as cell for
        // user input so better set cursor and curcor color every time
        SetTextColor(currentColor);

        // HACK for (DF-9457) fix blue border visible on close app where was UITextField
        NSCell* cell = [nsTextField cell];
        if (cell != nullptr)
        {
            [cell setFocusRingType:NSFocusRingTypeNone];
        }
    }

    void CloseKeyboard() override
    {
        // prevent recursion
        UITextFieldDelegate* delegate = davaText->GetDelegate();
        if (delegate && isKeyboardOpened)
        {
            isKeyboardOpened = false;
            delegate->OnKeyboardHidden();
        }

        // http://stackoverflow.com/questions/4881676/changing-focus-from-nstextfield-to-nsopenglview
        NSView* openGLView = (NSView*)Core::Instance()->GetNativeView();
        [[NSApp keyWindow] makeFirstResponder:openGLView];
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
        WideString oldText;
        GetText(oldText);

        if (oldText == string)
        {
            return;
        }

        updateViewState = true;

        NSString* text = [[[NSString alloc] initWithBytes:(char*)string.data()
                                                   length:string.size() * sizeof(wchar_t)
                                                 encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)] autorelease];
        [nsTextField setStringValue:text];

        UITextFieldDelegate* delegate = davaText->GetDelegate();
        if (delegate != nullptr)
        {
            delegate->TextFieldOnTextChanged(davaText, string, oldText);
        }

        // HACK if user click cleartext button and current
        // native control not in focus - remove focus from dava control too
        // to show hint to user
        if (string.empty() &&
            [[NSApp keyWindow] firstResponder] != nsTextField &&
            !insideTextShouldReturn &&
            davaText == UIControlSystem::Instance()->GetFocusedControl())
        {
            UIControlSystem::Instance()->SetFocusedControl(nullptr, false);
        }
    }

    void UpdateRect(const Rect& rectSrc) override
    {
        // HACK for battle screen
        // check if focus not synced
        bool isFocused = (UIControlSystem::Instance()->GetFocusedControl() == davaText);
        if (isFocused)
        {
            NSWindow* window = [NSApp keyWindow];
            NSResponder* currentResponder = [window firstResponder];
            if (currentResponder == nil)
            {
                // no focus in window at all
            }
            else
            {
                BOOL isNSText = [currentResponder isKindOfClass:[NSText class]];
                if (isNSText && [(id)currentResponder delegate] == (id)nsTextField)
                {
                    // we still has focus do nothing
                }
                else
                {
                    if (isKeyboardOpened)
                    {
                        UITextFieldDelegate* delegate = davaText->GetDelegate();
                        if (delegate && !delegate->IsTextFieldCanLostFocus(davaText))
                        {
                            // select text field
                            [window makeFirstResponder:nsTextField];
                            // remove selection to caret at end
                            if ([[nsTextField stringValue] length] > 0)
                            {
                                NSRange range = [[nsTextField currentEditor] selectedRange];
                                [[nsTextField currentEditor] setSelectedRange:NSMakeRange(range.length, 0)];
                            }
                        }
                    }
                }
            }
        }

        // we have to convert coord every time
        // if user change window/fullscreen mode
        NSRect controlRect = ConvertToNativeWindowRect(rectSrc);

        if (renderInTexture && !isFocused)
        {
            if (updateViewState)
            {
                updateViewState = false;
                if (!RenderToTextureAndSetAsBackgroundSprite(nsTextField))
                {
                    updateViewState = true; // try on next frame
                }
            }
            if (!updateViewState)
            {
                // can hide native control
                controlRect.origin.x -= 10000;
            }
        }
        else
        {
            davaText->SetSprite(nullptr, 0);
        }

        if (!NSEqualRects(nativeControlRect, controlRect))
        {
            [nsTextField setFrame:controlRect];
            nativeControlRect = controlRect;
        }
    }

    void SetTextColor(const DAVA::Color& color) override
    {
        updateViewState = true;

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
        updateViewState = true;

        currentFontSize = virtualFontSize;

        float32 size = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(virtualFontSize);

        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        NSSize origSz = NSMakeSize(size, 0);
        NSSize convSz = [openGLView convertSizeFromBacking:origSz];

        [nsTextField setFont:[NSFont systemFontOfSize:convSz.width]];
    }

    void SetTextAlign(DAVA::int32 align) override
    {
        updateViewState = true;

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

        // TODO investigate do we need to implement other then ALIGN_VCENTER
        // several time set align so comment it for now DVASSERT(align & ALIGN_VCENTER);
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
        updateViewState = true;

        useRtlAlign = useRtlAlign_;
        SetTextAlign(alignment);
    }

    bool GetTextUseRtlAlign() const override
    {
        return useRtlAlign;
    }

    void SetVisible(bool value) override
    {
        updateViewState = true;

        [nsTextField setHidden:!value];
    }

    void ShowField() override
    {
        // we always on screen
    }

    void HideField() override
    {
        // we always on screen
    }

    void SetInputEnabled(bool value) override
    {
        [nsTextField setEditable:value];
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
        DVASSERT(!value);

        multiline = value;
        // fix for macos 10.9
        if ([nsTextField respondsToSelector:@selector(setUsesSingleLineMode:)])
        {
            [nsTextField setUsesSingleLineMode:(!multiline)];
        }
        [nsTextField.cell setWraps:(!multiline)];
        [nsTextField.cell setScrollable:(!multiline)];
    }

    bool IsMultiline() const override
    {
        return multiline;
    }

    void SetIsPassword(bool value) override
    {
        if (password != value)
        {
            updateViewState = true;

            WideString oldText;
            GetText(oldText);

            CustomTextField* oldCtrl = nsTextField;
            if (value)
            {
                [CustomTextField setCellClass:[RSVerticallyCenteredSecureTextFieldCell class]];
            }
            else
            {
                [CustomTextField setCellClass:[RSVerticallyCenteredTextFieldCell class]];
            }

            // we have to recreate nsTextField for new CellClass
            // do you know way beter?
            nsTextField = [[CustomTextField alloc] initWithFrame:[oldCtrl frame]];

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
            SetMultiline(multiline);

            [oldCtrl removeFromSuperview];
            [oldCtrl release];

            password = value;
        }
    }

    bool IsPassword() const
    {
        return password;
    }

    // Max text length.
    void SetMaxLength(int maxLength) override
    {
        formatter->maxLength = maxLength;
    }

    void SetRenderToTexture(bool value) override
    {
        renderInTexture = value;
        updateViewState = true;
    }

    bool IsRenderToTexture() const override
    {
        return renderInTexture;
    }

    CustomTextField* nsTextField = nullptr;
    CustomDelegate* objcDelegate = nullptr;
    CustomTextFieldFormatter* formatter = nullptr;
};

class ObjCWrapper
{
public:
    ObjCWrapper(UITextField* tf)
    {
        ctrl = new SingleLineOrPasswordField(tf, this);
    }
    ~ObjCWrapper()
    {
        delete ctrl;
        ctrl = nullptr;
    }

    IField* operator->()
    {
        return ctrl;
    }

    void ChangeMultilineProperty(bool value)
    {
        if (ctrl->IsMultiline() != value)
        {
            IField* prevCtrl = ctrl;
            IField* newField = nullptr;
            if (value)
            {
                newField = new MultilineField(ctrl->davaText, this);
            }
            else
            {
                newField = new SingleLineOrPasswordField(ctrl->davaText, this);
            }

            newField->SetFontSize(prevCtrl->currentFontSize);
            newField->SetTextColor(prevCtrl->currentColor);

            WideString oldText;
            prevCtrl->GetText(oldText);

            newField->SetText(oldText);
            newField->SetTextUseRtlAlign(prevCtrl->useRtlAlign);
            newField->SetTextAlign(prevCtrl->alignment);
            newField->SetMultiline(value);

            delete prevCtrl;
            ctrl = newField;
        }
    }

    IField* ctrl = nullptr;
};

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* tf)
    : objcWrapper{ *(new ObjCWrapper(tf)) }
{
}

TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    ObjCWrapper* ptr = &objcWrapper;
    delete ptr;
    ptr = nullptr;
}

void TextFieldPlatformImpl::OpenKeyboard()
{
    objcWrapper->OpenKeyboard();
}

void TextFieldPlatformImpl::CloseKeyboard()
{
    objcWrapper->CloseKeyboard();
}

void TextFieldPlatformImpl::GetText(WideString& string) const
{
    objcWrapper->GetText(string);
}

void TextFieldPlatformImpl::SetText(const WideString& string)
{
    objcWrapper->SetText(string);
}

void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
    objcWrapper->UpdateRect(rect);
}

void TextFieldPlatformImpl::SetTextColor(const DAVA::Color& color)
{
    objcWrapper->SetTextColor(color);
}

void TextFieldPlatformImpl::SetFontSize(float size)
{
    objcWrapper->SetFontSize(size);
}

void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
{
    objcWrapper->SetTextAlign(align);
}

DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return objcWrapper->GetTextAlign();
}

void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
    objcWrapper->SetTextUseRtlAlign(useRtlAlign);
}

bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return objcWrapper->GetTextUseRtlAlign();
}

void TextFieldPlatformImpl::SetVisible(bool value)
{
    objcWrapper->SetVisible(value);
}

void TextFieldPlatformImpl::TextFieldPlatformImpl::ShowField()
{
    objcWrapper->ShowField();
}

void TextFieldPlatformImpl::HideField()
{
    objcWrapper->HideField();
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
    objcWrapper->SetIsPassword(isPassword);
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
    objcWrapper->SetInputEnabled(value);
}

// Keyboard traits.
void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
    objcWrapper->SetAutoCapitalizationType(value);
}

void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
{
    objcWrapper->SetAutoCorrectionType(value);
}

void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
{
    objcWrapper->SetSpellCheckingType(value);
}

void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
    objcWrapper->SetKeyboardAppearanceType(value);
}

void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
{
    objcWrapper->SetKeyboardType(value);
}

void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
{
    objcWrapper->SetReturnKeyType(value);
}

void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
    objcWrapper->SetEnableReturnKeyAutomatically(value);
}

// Cursor pos.
uint32 TextFieldPlatformImpl::GetCursorPos()
{
    return objcWrapper->GetCursorPos();
}

void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
    objcWrapper->SetCursorPos(pos);
}

// Max text length.
void TextFieldPlatformImpl::SetMaxLength(int maxLength)
{
    if (maxLength < 0)
    {
        maxLength = INT_MAX;
    }
    objcWrapper->SetMaxLength(maxLength);
}

void TextFieldPlatformImpl::SetMultiline(bool multiline)
{
    // here use (.) to change internal state, not to transit call to internal IField
    objcWrapper.ChangeMultilineProperty(multiline);
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
    objcWrapper->SetRenderToTexture(value);
}

bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return objcWrapper->IsRenderToTexture();
}

} // end namespace DAVA

@implementation CustomDelegate

- (id)init
{
    if (self = [super init])
    {
        text = nullptr;
        formatter = nullptr;
    }
    return self;
}

- (void)dealloc
{
    text = nullptr;
    formatter = nullptr;

    [super dealloc];
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

    text->ctrl->insideTextShouldReturn = true;

    if (commandSelector == @selector(insertNewline:))
    {
        if (text->ctrl->IsMultiline())
        {
            // new line action:
            // always insert a line-break character and dont cause the receiver to end editing
            [textView insertNewlineIgnoringFieldEditor:self];
        }
        else
        {
            text->ctrl->davaText->GetDelegate()->TextFieldShouldReturn(text->ctrl->davaText);
        }
        result = YES;
    }
    else if (commandSelector == @selector(cancelOperation:))
    {
        text->ctrl->davaText->GetDelegate()->TextFieldShouldCancel(text->ctrl->davaText);
        result = YES;
    }
    else if (commandSelector == @selector(insertTab:))
    {
        // do nothing on TAB key
        if (!text->ctrl->IsMultiline())
        {
            result = YES;
        }
    }
    else if (commandSelector == @selector(paste:))
    {
        // detect paste in textfield
        if (!text->ctrl->IsMultiline())
        {
            DAVA::WideString currentText;
            text->ctrl->GetText(currentText);
            size_t len = currentText.length();
            if (len >= formatter->maxLength)
            {
                // skip paste, no more room
                result = YES;
            }
        }
    }

    text->ctrl->insideTextShouldReturn = false;

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

- (BOOL)isPartialStringValid:(NSString*)partialString
            newEditingString:(NSString**)newString
            errorDescription:(NSString**)error
{
    if ([partialString length] >= maxLength)
    {
        return NO;
    }
    return YES;
}

- (BOOL)isPartialStringValid:(NSString**)partialStringPtr
       proposedSelectedRange:(NSRangePointer)proposedSelRangePtr
              originalString:(NSString*)origString
       originalSelectedRange:(NSRange)origSelRange
            errorDescription:(NSString**)error
{
    BOOL result = YES;

    NSString* inputStr = (*partialStringPtr);
    NSRange inputRange = (*proposedSelRangePtr);

    if ([*partialStringPtr length] >= maxLength)
    {
        int spaceLeft = maxLength - [origString length] - 1;
        // we can crop part of string only if user try to add to end
        if (spaceLeft > 0 && origSelRange.location == [origString length])
        {
            NSRange correctRange = NSMakeRange(origSelRange.location, spaceLeft);
            NSString* matchSizeStr = [*partialStringPtr substringWithRange:correctRange];
            NSString* resultStr = [NSString stringWithFormat:@"%@%@", origString, matchSizeStr];
            // write back
            *partialStringPtr = resultStr;
            *proposedSelRangePtr = NSMakeRange([resultStr length], 0);
            result = NO;
        }
        else
        {
            // if we crop and insert text in middle user may not
            // see absent of croped part, it is not good
            return NO; // cancel change now
        }
    }

    if (text != nullptr)
    {
        DAVA::UITextField* davaCtrl = text->ctrl->davaText;

        // if user paste text with gesture in native control
        // we need make dava control in sync with focus
        if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != davaCtrl)
        {
            DAVA::UIControlSystem::Instance()->SetFocusedControl(davaCtrl, false);
        }

        DAVA::UITextFieldDelegate* delegate = davaCtrl->GetDelegate();
        if (delegate != nullptr)
        {
            NSString* nsReplacement = nullptr;
            DAVA::int32 location = 0;
            // if we just add to string end
            if (origSelRange.location == [origString length] && origSelRange.length == 0)
            {
                nsReplacement = [*partialStringPtr substringFromIndex:origSelRange.location];
                location = origSelRange.location;
            }
            else
            {
                // simple change whole string for new string
                nsReplacement = *partialStringPtr;
                location = 0;
            }
            DAVA::WideString replacement;
            const char* cstr = [nsReplacement cStringUsingEncoding:NSUTF8StringEncoding];
            size_t strSize = std::strlen(cstr);
            DAVA::UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(cstr), strSize, replacement);

            DAVA::int32 length = [origString length];
            bool resultDelegate = delegate->TextFieldKeyPressed(davaCtrl, location, length, replacement);
            if (!resultDelegate)
            {
                if (result == NO)
                {
                    // restore input params
                    *partialStringPtr = inputStr;
                    *proposedSelRangePtr = inputRange;
                }
                else
                {
                    result = NO;
                }
            }
            else
            {
                DAVA::WideString oldText;
                DAVA::WideString newText;

                text->ctrl->GetText(oldText);

                const char* cstrNew = [inputStr cStringUsingEncoding:NSUTF8StringEncoding];
                size_t cstrNewSize = std::strlen(cstrNew);
                DAVA::UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(cstrNew), cstrNewSize, newText);

                delegate->TextFieldOnTextChanged(davaCtrl, newText, oldText);
            }
        }
    }

    return result;
}

- (NSAttributedString*)attributedStringForObjectValue:(id)anObject
                                withDefaultAttributes:(NSDictionary*)attributes
{
    return nil;
}

@end

@implementation MultilineDelegate

- (id)init
{
    if (self = [super init])
    {
        maxLength = INT_MAX;
    }

    return self;
}

- (BOOL)textShouldBeginEditing:(NSText*)textObject
{
    DAVA::UITextField* textField = (*text).ctrl->davaText;
    if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != textField)
    {
        DAVA::UIControlSystem::Instance()->SetFocusedControl(textField, false);
    }
    return YES;
}

- (BOOL)textView:(NSTextView*)aTextView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString*)replacementString
{
    BOOL result = YES;

    // Get string after changing
    NSString* newString = [[aTextView string] stringByReplacingCharactersInRange:affectedCharRange withString:replacementString];

    // check size
    if ([newString length] > maxLength)
    {
        result = NO;
    }
    else
    {
        // call client delegate
        DAVA::UITextField* textField = (*text).ctrl->davaText;
        DAVA::UITextFieldDelegate* delegate = textField->GetDelegate();
        if (delegate)
        {
            DAVA::WideString oldStr = textField->GetText();

            DAVA::WideString repString;
            const char* cstr = [replacementString cStringUsingEncoding:NSUTF8StringEncoding];
            DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), repString);

            bool isValid = delegate->TextFieldKeyPressed(
            textField,
            static_cast<DAVA::int32>(affectedCharRange.location),
            static_cast<DAVA::int32>(affectedCharRange.length),
            repString);

            if (isValid)
            {
                DAVA::WideString newStr;
                const char* cstr = [newString cStringUsingEncoding:NSUTF8StringEncoding];
                DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, (DAVA::int32)strlen(cstr), newStr);
                delegate->TextFieldOnTextChanged(textField, newStr, oldStr);
            }

            result = isValid ? YES : NO;
        }
    }

    return result;
}
@end

@implementation RSVerticallyCenteredTextFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
    // Get the parent's idea of where we should draw
    NSRect newRect = [super drawingRectForBounds:theRect];

    if (!isMultilineControl)
    {
        // When the text field is being
        // edited or selected, we have to turn off the magic because it screws up
        // the configuration of the field editor.  We sneak around this by
        // intercepting selectWithFrame and editWithFrame and sneaking a
        // reduced, centered rect in at the last minute.
        if (isEditingOrSelecting == NO)
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
    }
    return newRect;
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView*)controlView editor:(NSText*)textObj delegate:(id)anObject start:(long)selStart length:(long)selLength
{
    CustomTextField* textField = (CustomTextField*)controlView;
    CustomTextFieldFormatter* formatter = (CustomTextFieldFormatter*)textField.formatter;
    isMultilineControl = formatter->text->ctrl->IsMultiline();
    if (isMultilineControl)
    {
        [super selectWithFrame:aRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
    }
    else
    {
        aRect = [self drawingRectForBounds:aRect];
        isEditingOrSelecting = YES;
        [super selectWithFrame:aRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
        isEditingOrSelecting = NO;
    }
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView*)controlView editor:(NSText*)textObj delegate:(id)anObject event:(NSEvent*)theEvent
{
    CustomTextField* textField = (CustomTextField*)controlView;
    CustomTextFieldFormatter* formatter = (CustomTextFieldFormatter*)textField.formatter;
    isMultilineControl = formatter->text->ctrl->IsMultiline();
    if (isMultilineControl)
    {
        [super editWithFrame:aRect inView:controlView editor:textObj delegate:anObject event:theEvent];
    }
    else
    {
        aRect = [self drawingRectForBounds:aRect];
        isEditingOrSelecting = YES;
        [super editWithFrame:aRect inView:controlView editor:textObj delegate:anObject event:theEvent];
        isEditingOrSelecting = NO;
    }
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

@implementation CustomTextField
- (void)mouseDown:(NSEvent*)theEvent
{
    // pass event to DAVA input for selection and focus work
    NSView* openGLView = (NSView*)DAVA::Core::Instance()->GetNativeView();
    [openGLView mouseDown:theEvent];
}

//
//- (BOOL)performKeyEquivalent:(NSEvent*)event
//{
//    if (([event modifierFlags] & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
//    {   // The command key is the ONLY modifier key being pressed.
//        if ([[event charactersIgnoringModifiers] isEqualToString:@"x"])
//        {
//            return [NSApp sendAction:@selector(cut:) to:[[self window] firstResponder] from:self];
//        }
//        else if ([[event charactersIgnoringModifiers] isEqualToString:@"c"])
//        {
//            return [NSApp sendAction:@selector(copy:) to:[[self window] firstResponder] from:self];
//        }
//        else if ([[event charactersIgnoringModifiers] isEqualToString:@"v"])
//        {
//            return [NSApp sendAction:@selector(paste:) to:[[self window] firstResponder] from:self];
//        }
//        else if ([[event charactersIgnoringModifiers] isEqualToString:@"a"])
//        {
//            return [NSApp sendAction:@selector(selectAll:) to:[[self window] firstResponder] from:self];
//        }
//    }
//    return [super performKeyEquivalent:event];
//}
@end

@implementation CustomTextView

@end

#endif //__DAVAENGINE_MACOS__