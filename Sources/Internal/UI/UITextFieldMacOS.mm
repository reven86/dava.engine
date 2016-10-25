#include "UI/UITextFieldMacOS.h"

#if defined(__DAVAENGINE_MACOS__) && !defined(DISABLE_NATIVE_TEXTFIELD)

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
#include "Utils/NSStringUtils.h"
#include "Render/Image/Image.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/Focus/FocusHelpers.h"
#include "UI/UIControlSystem.h"
#include "Platform/Steam.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/EngineModule.h"
#include "Engine/Public/WindowNativeService.h"
#else
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#endif
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
    DAVA::WideString lastString;
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
    DAVA::WideString lastString;
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
#if defined(__DAVAENGINE_COREV2__)
    IField(UITextField* davaText_, ObjCWrapper* wrapper_, Window* w)
        : davaText(davaText_)
        , wrapper(wrapper_)
        , window(w)
#else
    IField(UITextField* davaText_, ObjCWrapper* wrapper_)
        : davaText(davaText_)
        , wrapper(wrapper_)
#endif
    {
        DVASSERT(davaText != nullptr);
        DVASSERT(wrapper != nullptr);
        
#if defined(__DAVAENGINE_COREV2__)
        windowVisibilityChangedConnection = window->visibilityChanged.Connect(this, &IField::OnWindowVisibilityChanged);
#else
        CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
        signalMinimizeRestored = xcore->signalAppMinimizedRestored.Connect(this, &IField::OnAppMinimizedRestored);
#endif

#if defined(__DAVAENGINE_STEAM__)
        overlayConnectionId = Steam::GameOverlayActivated.Connect(this, &IField::OnSteamOverlayChanged);
#endif
    }
    virtual ~IField()
    {
#if defined(__DAVAENGINE_STEAM__)
        Steam::GameOverlayActivated.Disconnect(overlayConnectionId);
#endif

        davaText = nullptr;
        wrapper = nullptr;
#if defined(__DAVAENGINE_COREV2__)
        window->visibilityChanged.Disconnect(windowVisibilityChangedConnection);
#else
        CoreMacOSPlatformBase* xcore = static_cast<CoreMacOSPlatformBase*>(Core::Instance());
        xcore->signalAppMinimizedRestored.Disconnect(signalMinimizeRestored);
#endif
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

    NSRect ConvertToNativeWindowRect(const Rect& srcRect, NSView* parent)
    {
        VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();
     
#if defined(__DAVAENGINE_COREV2__)
        // 1. map virtual to physical
        Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
        rect += coordSystem->GetPhysicalDrawOffset();
        rect.y = coordSystem->GetPhysicalScreenSize().dy - (rect.y + rect.dy);

        rect.dx = std::max(0.0f, rect.dx);
        rect.dy = std::max(0.0f, rect.dy);

        NSRect controlRect = [[parent superview] convertRectFromBacking:NSMakeRect(rect.x, rect.y, rect.dx, rect.dy)];
        return controlRect;
#else
        Rect rect = coordSystem->ConvertVirtualToInput(srcRect);

        NSView* openglView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        rect.y = openglView.frame.size.height - (rect.y + rect.dy);
        return NSMakeRect(rect.x, rect.y, rect.dx, rect.dy);
#endif
    }

#if defined(__DAVAENGINE_COREV2__)
    void OnWindowVisibilityChanged(Window&, bool visible)
    {
        SetVisible(visible);
    }

    size_t windowVisibilityChangedConnection = 0;
#else
    void OnAppMinimizedRestored(bool minimized)
    {
        SetVisible(!minimized);
    }

    SigConnectionID signalMinimizeRestored;
#endif

    UITextField* davaText = nullptr;
    ObjCWrapper* wrapper = nullptr;
#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
#endif

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

#if defined(__DAVAENGINE_STEAM__)
    SigConnectionID overlayConnectionId = 0;
    bool visibleBeforeSteamOverlayActivated = true;
    virtual void OnSteamOverlayChanged(bool overlayActivated)
    {
        if (overlayActivated)
        {
            visibleBeforeSteamOverlayActivated = IsNativeVisible();
            SetVisible(false);
        }
        else
        {
            SetVisible(visibleBeforeSteamOverlayActivated);
        }
    }
#endif
    virtual bool IsNativeVisible() const = 0;
};

class MultilineField : public IField
{
public:
#if defined(__DAVAENGINE_COREV2__)
    MultilineField(UITextField* davaText_, ObjCWrapper* wrapper_, Window* w)
        : IField(davaText_, wrapper_, w)
#else
    MultilineField(UITextField* davaText_, ObjCWrapper* wrapper_)
        : IField(davaText_, wrapper_)
#endif
    {
        nsScrollView = [[NSScrollView alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];

        [nsScrollView setBorderType:NSNoBorder];
        [nsScrollView setHasVerticalScroller:YES];
        [nsScrollView setHasHorizontalScroller:NO];
        [nsScrollView setAutoresizingMask:NSViewWidthSizable |
                      NSViewHeightSizable];

        nsTextView = [[CustomTextView alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];
        [nsTextView setWantsLayer:YES]; // need to be visible over opengl view
        [nsTextView setRichText:NO]; // disable rich text support (copy from clipboard)

        objcDelegate = [[MultilineDelegate alloc] init];
        if (nullptr != davaText_)
        {
            objcDelegate->maxLength = davaText_->GetMaxLength();
        }
        objcDelegate->text = wrapper;

        [nsTextView setDelegate:objcDelegate];

        [nsTextView setEditable:YES];

        // make control border and background transparent
        nsTextView.drawsBackground = NO;

        [nsScrollView setDocumentView:nsTextView];

#if defined(__DAVAENGINE_COREV2__)
        window->GetNativeService()->AddNSView(nsScrollView);
#else
        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        [openGLView addSubview:nsScrollView];
#endif
    }

    ~MultilineField()
    {
#if defined(__DAVAENGINE_COREV2__)
        window->GetNativeService()->RemoveNSView(nsScrollView);
#else
        [nsScrollView removeFromSuperview];
#endif
        [nsScrollView release];
        nsScrollView = nullptr;
        [nsTextView removeFromSuperview];
        [nsTextView release];
        nsTextView = nullptr;

        [objcDelegate release];
        objcDelegate = nullptr;
    }

    void OpenKeyboard() override
    {
#if defined(__DAVAENGINE_COREV2__)
        [[nsScrollView superview]
         .window makeFirstResponder:nsTextView];
#else
        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        [openGLView.window makeFirstResponder:nsTextView];
#endif

        if (!isKeyboardOpened)
        {
            isKeyboardOpened = true;
            Rect emptyRect;
            emptyRect.y = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
            davaText->OnKeyboardShown(emptyRect);
        }
    }

    void CloseKeyboard() override
    {
        if (isKeyboardOpened)
        {
            isKeyboardOpened = false;
            davaText->OnKeyboardHidden();
        }

#if defined(__DAVAENGINE_COREV2__)
        // http://stackoverflow.com/questions/4881676/changing-focus-from-nstextfield-to-nsopenglview
        [[NSApp keyWindow] makeFirstResponder:[nsScrollView superview]];
#else
        // http://stackoverflow.com/questions/4881676/changing-focus-from-nstextfield-to-nsopenglview
        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        [[NSApp keyWindow] makeFirstResponder:openGLView];
#endif
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
        NSString* text = [[[NSString alloc] initWithBytes:reinterpret_cast<const char*>(string.data())
                                                   length:string.size() * sizeof(wchar_t)
                                                 encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)] autorelease];
        DAVA::WideString oldString = davaText->GetText();
        [nsTextView setString:text];
        if (nullptr != davaText && string != oldString)
        {
            davaText->GetDelegate()->TextFieldOnTextChanged(davaText, string, oldString, UITextFieldDelegate::eReason::CODE);
        }

        // HACK if user click cleartext button and current
        // native control not in focus - remove focus from dava control too
        // to show hint to user
        if (string.empty() &&
            [[NSApp keyWindow] firstResponder] != nsTextView &&
            !insideTextShouldReturn &&
            davaText == UIControlSystem::Instance()->GetFocusedControl())
        {
            davaText->ReleaseFocus();
        }
        // notify after text changed
        UITextFieldDelegate* delegate = davaText->GetDelegate();
        if (nullptr != delegate)
        {
            WideString oldText;
            GetText(oldText);
            delegate->TextFieldOnTextChanged(davaText, string, oldText);
        }
    }

    void UpdateRect(const Rect& rectSrc) override
    {
        if (currentRect != rectSrc)
        {
            currentRect = rectSrc;
            NSRect controlRect = ConvertToNativeWindowRect(rectSrc, [nsScrollView superview]);

            controlRect.size.width = std::max(0.0, controlRect.size.width);
            controlRect.size.height = std::max(0.0, controlRect.size.height);

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

        NSSize origSz = NSMakeSize(size, 0);
#if defined(__DAVAENGINE_COREV2__)
        [[nsScrollView superview] convertSizeFromBacking:origSz];
        NSSize convSz = [[nsScrollView superview] convertSizeFromBacking:origSz];
#else
        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        NSSize convSz = [openGLView convertSizeFromBacking:origSz];
#endif
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

    bool IsNativeVisible() const override
    {
        return nsScrollView != nil && !nsScrollView.hidden;
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
#if defined(__DAVAENGINE_COREV2__)
    explicit SingleLineOrPasswordField(UITextField* davaText_, ObjCWrapper* wrapper_, Window* w)
        : IField(davaText_, wrapper_, w)
#else
    explicit SingleLineOrPasswordField(UITextField* davaText_, ObjCWrapper* wrapper_)
        : IField(davaText_, wrapper_)
#endif
    {
        [CustomTextField setCellClass:[RSVerticallyCenteredTextFieldCell class]];
        nsTextField = [[CustomTextField alloc] initWithFrame:CGRectMake(0.f, 0.f, 0.f, 0.f)];
        [nsTextField setWantsLayer:YES]; // need to be visible over opengl view
        formatter = [[CustomTextFieldFormatter alloc] init];
        formatter->text = wrapper;
        if (nullptr != davaText_)
        {
            formatter->maxLength = davaText_->GetMaxLength();
        }
        [nsTextField setFormatter:formatter];
        objcDelegate = [[CustomDelegate alloc] init];
        objcDelegate->text = wrapper;
        objcDelegate->formatter = formatter;

        [nsTextField setDelegate:objcDelegate];

#if defined(__DAVAENGINE_COREV2__)
        window->GetNativeService()->AddNSView(nsTextField);
#else
        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        [openGLView addSubview:nsTextField];
#endif

        [nsTextField setEditable:YES];
        [nsTextField setEnabled:YES];
        // make control border and background transparent
        nsTextField.drawsBackground = NO;
        nsTextField.bezeled = NO;

        SetMultiline(false);
    }

    ~SingleLineOrPasswordField()
    {
#if defined(__DAVAENGINE_COREV2__)
        window->GetNativeService()->RemoveNSView(nsTextField);
#else
        [nsTextField removeFromSuperview];
#endif
        [nsTextField release];
        nsTextField = nullptr;
        [formatter release];
        formatter = nullptr;
        [objcDelegate release];
        objcDelegate = nullptr;
    }

    void OpenKeyboard() override
    {
        nsTextField.enabled = YES;

        if (!isKeyboardOpened)
        {
            isKeyboardOpened = true;
            Rect emptyRect;
            emptyRect.y = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
            davaText->OnKeyboardShown(emptyRect);
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
        if (isKeyboardOpened)
        {
            isKeyboardOpened = false;
            davaText->OnKeyboardHidden();
        }

#if defined(__DAVAENGINE_COREV2__)
        // http://stackoverflow.com/questions/4881676/changing-focus-from-nstextfield-to-nsopenglview
        [[NSApp keyWindow] makeFirstResponder:[nsTextField superview]];
#else
        // http://stackoverflow.com/questions/4881676/changing-focus-from-nstextfield-to-nsopenglview
        NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
        [[NSApp keyWindow] makeFirstResponder:openGLView];
#endif
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

        NSString* text = [[[NSString alloc] initWithBytes:reinterpret_cast<const char*>(string.data())
                                                   length:string.size() * sizeof(wchar_t)
                                                 encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)] autorelease];
        [nsTextField setStringValue:text];

        // notify after text changed
        if (nullptr != davaText && nullptr != davaText->GetDelegate())
        {
            davaText->GetDelegate()->TextFieldOnTextChanged(davaText, string, oldText, UITextFieldDelegate::eReason::CODE);
        }

        // HACK if user click cleartext button and current
        // native control not in focus - remove focus from dava control too
        // to show hint to user
        if (string.empty() &&
            [[NSApp keyWindow] firstResponder] != nsTextField &&
            !insideTextShouldReturn &&
            davaText == UIControlSystem::Instance()->GetFocusedControl())
        {
            davaText->ReleaseFocus();
        }
    }

    void UpdateRect(const Rect& rectSrc) override
    {
        // HACK for battle screen
        // check if focus not synced
        bool isFocused = (UIControlSystem::Instance()->GetFocusedControl() == davaText) && davaText->IsEditing();
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
                if (isNSText && [static_cast<id>(currentResponder) delegate] == static_cast<id>(nsTextField))
                {
                    // we still has focus do nothing
                }
                else
                {
                    if (isKeyboardOpened)
                    {
                        //                        // select text field
                        if (davaText->GetStopEditPolicy() == UITextField::STOP_EDIT_WHEN_FOCUS_LOST)
                        {
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
        NSRect controlRect = ConvertToNativeWindowRect(rectSrc, [nsTextField superview]);

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
            controlRect.size.width = std::max(0.0, controlRect.size.width);
            controlRect.size.height = std::max(0.0, controlRect.size.height);

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
        NSTextView* fieldEditor = static_cast<NSTextView*>([nsTextField.window fieldEditor:YES
                                                                                 forObject:nsTextField]);
        fieldEditor.insertionPointColor = nsColor;
    }

    void SetFontSize(float virtualFontSize) override
    {
        updateViewState = true;

        currentFontSize = virtualFontSize;

        float32 size = VirtualCoordinatesSystem::Instance()->ConvertVirtualToInputX(virtualFontSize);

        [nsTextField setFont:[NSFont systemFontOfSize:size]];
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
        if (!value)
        {
            // force UpdateRect call when dava control is hiding. should be done to ensure that nativeControl will also become hidden (moved away from visible screen)
            UpdateRect(davaText->GetRect());
        }
    }

    bool IsNativeVisible() const override
    {
        return nsTextField != nil && !nsTextField.hidden;
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

#if defined(__DAVAENGINE_COREV2__)
            window->GetNativeService()->AddNSView(nsTextField);
#else
            NSView* openGLView = static_cast<NSView*>(Core::Instance()->GetNativeView());
            [openGLView addSubview:nsTextField];
#endif

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
#if defined(__DAVAENGINE_COREV2__)
    ObjCWrapper(UITextField* tf, Window* w)
        : window(w)
    {
        ctrl = new SingleLineOrPasswordField(tf, this, window);
    }
#else
    ObjCWrapper(UITextField* tf)
    {
        ctrl = new SingleLineOrPasswordField(tf, this);
    }
#endif
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
#if defined(__DAVAENGINE_COREV2__)
            if (value)
            {
                newField = new MultilineField(ctrl->davaText, this, window);
            }
            else
            {
                newField = new SingleLineOrPasswordField(ctrl->davaText, this, window);
            }
#else
            if (value)
            {
                newField = new MultilineField(ctrl->davaText, this);
            }
            else
            {
                newField = new SingleLineOrPasswordField(ctrl->davaText, this);
            }
#endif

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

#if defined(__DAVAENGINE_COREV2__)
    Window* window = nullptr;
#endif
    IField* ctrl = nullptr;
};

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* tf)
#if defined(__DAVAENGINE_COREV2__)
    : objcWrapper
{
    *(new ObjCWrapper(tf, Engine::Instance()->PrimaryWindow()))
}
#else
    : objcWrapper
{
    *(new ObjCWrapper(tf))
}
#endif
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

- (void)controlTextDidBeginEditing:(NSNotification*)notification
{
    if (nullptr != text && nullptr != text->ctrl && nullptr != text->ctrl->davaText)
    {
        lastString = text->ctrl->davaText->GetText();
    }
}

- (void)controlTextDidChange:(NSNotification*)notification
{
    if (nullptr != text && nullptr != text->ctrl && nullptr != text->ctrl->davaText && nullptr != text->ctrl->davaText->GetDelegate())
    {
        const DAVA::WideString& newText = text->ctrl->davaText->GetText();
        if (newText != lastString)
        {
            text->ctrl->davaText->GetDelegate()->TextFieldOnTextChanged(text->ctrl->davaText, newText, lastString, DAVA::UITextFieldDelegate::eReason::USER);
            lastString = text->ctrl->davaText->GetText();
        }
    }
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
            DAVA::UITextFieldDelegate* delegate = text->ctrl->davaText->GetDelegate();
            if (delegate != nullptr)
            {
                delegate->TextFieldShouldReturn(text->ctrl->davaText);
            }
        }
        result = YES;
    }
    else if (commandSelector == @selector(cancelOperation:))
    {
        DAVA::UITextFieldDelegate* delegate = text->ctrl->davaText->GetDelegate();
        if (delegate != nullptr)
        {
            delegate->TextFieldShouldCancel(text->ctrl->davaText);
        }
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
    return static_cast<NSString*>(object);
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
              originalString:(NSString*)inOrigString
       originalSelectedRange:(NSRange)origSelRange
            errorDescription:(NSString**)error
{
    DAVA::UITextField* cppTextField = nullptr;
    if (nullptr != text && nullptr != text->ctrl && nullptr != text->ctrl->davaText)
    {
        cppTextField = text->ctrl->davaText;
    }

    BOOL applyChanges = YES;
    NSString* replString = @"";
    NSRange replRange = NSMakeRange(proposedSelRangePtr->location, 0);
    // only if paste or replace text, which need check
    if (proposedSelRangePtr->location > origSelRange.location)
    {
        replRange.location = origSelRange.location;
        replRange.length = proposedSelRangePtr->location - origSelRange.location;
        replString = [*partialStringPtr substringWithRange:replRange];
    }
    // calculate range for insert replString in origString
    NSRange correctRange;
    correctRange.location = DAVA::Min(origSelRange.location, proposedSelRangePtr->location);
    correctRange.length = DAVA::Max([*partialStringPtr length], [inOrigString length] + [replString length]) - DAVA::Min([*partialStringPtr length], [inOrigString length] + [replString length]);

    if ([replString length] > 0)
    {
        applyChanges = !DAVA::NSStringModified(correctRange, inOrigString, maxLength, &replString);
    }

    if (nullptr != cppTextField && nullptr != cppTextField->GetDelegate())
    {
        BOOL clientApply = YES;
        if (correctRange.length > 0 || [replString length] > 0)
        {
            DAVA::WideString clientString = DAVA::WideStringFromNSString(replString);
            clientApply = cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, static_cast<DAVA::int32>(correctRange.location), static_cast<DAVA::int32>(correctRange.length), clientString);
        }
        if (!clientApply)
        {
            *partialStringPtr = [[NSString alloc] initWithString:inOrigString];
            *proposedSelRangePtr = NSMakeRange(origSelRange.location, origSelRange.length);

            return NO;
        }
    }
    // when youself set changes in native control
    if (!applyChanges)
    {
        NSString* newString = [inOrigString stringByReplacingCharactersInRange:correctRange withString:replString];
        DAVA::WideString newDAVAString = DAVA::WideStringFromNSString(newString);
        *partialStringPtr = newString;
        *proposedSelRangePtr = NSMakeRange(correctRange.location + [replString length], 0);
        if (nullptr != cppTextField && ![inOrigString isEqualToString:newString])
        {
            DAVA::WideString clientString = DAVA::WideStringFromNSString(newString);
            DAVA::WideString oldString = DAVA::WideStringFromNSString(inOrigString);
            cppTextField->GetDelegate()->TextFieldOnTextChanged(cppTextField, clientString, oldString, DAVA::UITextFieldDelegate::eReason::CODE);
        }
    }
    return applyChanges;
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
    if (nullptr != text && nullptr != text->ctrl && nullptr != text->ctrl->davaText)
    {
        lastString = text->ctrl->davaText->GetText();
    }
    DAVA::UITextField* textField = (*text).ctrl->davaText;
    if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != textField)
    {
        DAVA::UIControlSystem::Instance()->SetFocusedControl(textField);
        if (DAVA::UIControlSystem::Instance()->GetFocusedControl() == textField)
        {
            textField->StartEdit();
        }
        else
            return NO;
    }
    return YES;
}

- (void)textDidChange:(NSNotification*)notification
{
    if (nullptr != text && nullptr != text->ctrl && nullptr != text->ctrl->davaText && text->ctrl->davaText->GetDelegate() != nullptr)
    {
        const DAVA::WideString& newText = text->ctrl->davaText->GetText();
        if (newText != lastString)
        {
            text->ctrl->davaText->GetDelegate()->TextFieldOnTextChanged(text->ctrl->davaText, newText, lastString, DAVA::UITextFieldDelegate::eReason::USER);
            lastString = text->ctrl->davaText->GetText();
        }
    }
}

- (BOOL)textView:(NSTextView*)aTextView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString*)replacementString
{
    DAVA::UITextField* cppTextField = nullptr;
    if (nullptr != text && nullptr != text->ctrl && nullptr != text->ctrl->davaText)
    {
        cppTextField = text->ctrl->davaText;
    }

    BOOL applyChanges = YES;
    // if user paste text with gesture in native control
    // we need make dava control in sync with focus
    if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != cppTextField)
    {
        DAVA::UIControlSystem::Instance()->SetFocusedControl(cppTextField);
    }

    NSString* origString = [aTextView string];
    NSString* replStr = replacementString;

    cppTextField->StartEdit();
    if ([replStr length] > 0)
    {
        applyChanges = !DAVA::NSStringModified(affectedCharRange, origString, maxLength, &replStr);
    }

    if (nullptr != cppTextField && nullptr != cppTextField->GetDelegate())
    {
        BOOL clientApply = YES;
        if (affectedCharRange.length > 0 || [replStr length] > 0)
        {
            DAVA::WideString clientString = DAVA::WideStringFromNSString(replStr);
            clientApply = cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, static_cast<DAVA::int32>(affectedCharRange.location), static_cast<DAVA::int32>(affectedCharRange.length), clientString);
        }
        if (!clientApply)
        {
            return NO;
        }
    }
    if (!applyChanges)
    {
        NSString* newString = [origString stringByReplacingCharactersInRange:affectedCharRange withString:replStr];
        DAVA::WideString newDAVAString = DAVA::WideStringFromNSString(newString);
        if (nullptr != cppTextField && nullptr != cppTextField->GetDelegate())
        {
            text->ctrl->SetText(newDAVAString);
            text->ctrl->SetCursorPos(affectedCharRange.location + [replStr length]);
            cppTextField->GetDelegate()->TextFieldOnTextChanged(cppTextField, cppTextField->GetText(), newDAVAString);
        }
        if (nullptr != cppTextField && ![origString isEqualToString:newString])
        {
            DAVA::WideString clientString = DAVA::WideStringFromNSString(newString);
            DAVA::WideString oldString = DAVA::WideStringFromNSString(origString);
            cppTextField->GetDelegate()->TextFieldOnTextChanged(cppTextField, clientString, oldString, DAVA::UITextFieldDelegate::eReason::CODE);
        }
    }
    return applyChanges;
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
    CustomTextField* textField = static_cast<CustomTextField*>(controlView);
    CustomTextFieldFormatter* formatter = static_cast<CustomTextFieldFormatter*>(textField.formatter);
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
    CustomTextField* textField = static_cast<CustomTextField*>(controlView);
    CustomTextFieldFormatter* formatter = static_cast<CustomTextFieldFormatter*>(textField.formatter);
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
#if defined(__DAVAENGINE_COREV2__)
    // pass event to DAVA input for selection and focus work
    [[self superview] mouseDown:theEvent];
#else
    // pass event to DAVA input for selection and focus work
    NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
    [openGLView mouseDown:theEvent];
#endif
}

@end

@implementation CustomTextView

- (bool)DAVATextFieldEditing
{
    MultilineDelegate* delegate = static_cast<MultilineDelegate*>([self delegate]);
    if (nullptr == delegate || nullptr == delegate->text || nullptr == delegate->text->ctrl || nullptr == delegate->text->ctrl->davaText)
    {
        return true;
    }
    return (delegate->text->ctrl->davaText == DAVA::UIControlSystem::Instance()->GetFocusedControl()) && delegate->text->ctrl->davaText->IsEditing();
}

- (void)mouseUp:(NSEvent*)theEvent
{
    if ([self DAVATextFieldEditing])
    {
        [super mouseUp:theEvent];
    }
    else
    {
#if defined(__DAVAENGINE_COREV2__)
        [[self superview] mouseUp:theEvent];
#else
        NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
        [openGLView mouseUp:theEvent];
#endif
    }
}

- (void)mouseDown:(NSEvent*)theEvent
{
    if ([self DAVATextFieldEditing])
    {
        [super mouseDown:theEvent];
    }
    else
    {
#if defined(__DAVAENGINE_COREV2__)
        [[self superview] mouseDown:theEvent];
#else
        NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
        [openGLView mouseDown:theEvent];
#endif
    }
}

- (void)rightMouseDown:(NSEvent*)theEvent
{
    if ([self DAVATextFieldEditing])
    {
        [super rightMouseDown:theEvent];
    }
    else
    {
#if defined(__DAVAENGINE_COREV2__)
        [[self superview] rightMouseDown:theEvent];
#else
        NSView* openGLView = static_cast<NSView*>(DAVA::Core::Instance()->GetNativeView());
        [openGLView rightMouseDown:theEvent];
#endif
    }
}

@end

#endif //__DAVAENGINE_MACOS__ && !DISABLE_NATIVE_TEXTFIELD
