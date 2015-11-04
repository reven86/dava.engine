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


#include "Base/BaseTypes.h"
#include "Core/Core.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include "UI/UITextField.h"
#include "UI/UITextFieldiPhone.h"
#include "Platform/TemplateiOS/UITextFieldHolder.h"
#include "Core/Core.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

#include "Platform/TemplateiOS/WebViewControliOS.h"

#import "Platform/TemplateiOS/HelperAppDelegate.h"

namespace
{
    const int MOVE_TO_OFFSCREEN_STEP = 20000;
}

namespace DAVA 
{
TextFieldPlatformImpl::TextFieldPlatformImpl(DAVA::UITextField* tf)
    : davaTextField(*tf)
    , renderToTexture(false)
    {
        DVASSERT(isSingleLine);
        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        BackgroundView* backgroundView = [appDelegate renderViewController].backgroundView;

        UITextFieldHolder * textFieldHolder= [backgroundView CreateTextField];
        DVASSERT(textFieldHolder->textCtrl != nullptr);
        
        [textFieldHolder setTextField:&davaTextField];
        [textFieldHolder dropCachedText];
        
        objcClassPtr = textFieldHolder;

        prevRect = tf->GetRect();
        if (renderToTexture)
        {
            UpdateNativeRect(prevRect, MOVE_TO_OFFSCREEN_STEP);
        } else
        {
            UpdateNativeRect(prevRect, 0);
        }
    }
    TextFieldPlatformImpl::~TextFieldPlatformImpl()
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setTextField:(DAVA::UITextField*)nil];

        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        BackgroundView* backgroundView = [appDelegate renderViewController].backgroundView;
        if (!isSingleLine)
    {
        textFieldHolder->textField.userInteractionEnabled = NO;
        // destroy UITextView and restore textFild back
        [textFieldHolder->textCtrl removeFromSuperview];

        textFieldHolder->textCtrl = textFieldHolder->textField;
        [backgroundView PrepareView:textFieldHolder->textCtrl];
        [textFieldHolder addSubview:textFieldHolder->textCtrl];
    }

    [backgroundView ReleaseTextField:textFieldHolder];
    objcClassPtr = 0;
    }

    void TextFieldPlatformImpl::SetTextColor(const DAVA::Color& color)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UIColor* col = [UIColor colorWithRed:color.r green:color.g blue:color.b alpha:color.a];
        UIView* view = textFieldHolder->textCtrl;
        [view setValue:col forKey:@"textColor"];

        isNeedToUpdateTexture = true;
    }
    void TextFieldPlatformImpl::SetFontSize(float size)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        float scaledSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(size);
        scaledSize /= Core::Instance()->GetScreenScaleFactor();

        UIView* view = textFieldHolder->textCtrl;
        UIFont* font = [UIFont systemFontOfSize:scaledSize];
        [view setValue:font forKey:@"font"];

        isNeedToUpdateTexture = true;
    }

    void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UIView* view = textFieldHolder->textCtrl;
        if (isSingleLine)
        {
            ::UITextField* field = (::UITextField*)view;
            if (align & ALIGN_LEFT)
            {
                field.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft;
            }
            else if (align & ALIGN_HCENTER)
            {
                field.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
            }
            else if (align & ALIGN_RIGHT)
        {
            field.contentHorizontalAlignment = UIControlContentHorizontalAlignmentRight;
        }

        if (align & ALIGN_TOP)
            field.contentVerticalAlignment = UIControlContentVerticalAlignmentTop;
        else if (align & ALIGN_VCENTER)
            field.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        else if (align & ALIGN_BOTTOM)
            field.contentVerticalAlignment = UIControlContentVerticalAlignmentBottom;

        // Set natural alignment if need
        switch (field.contentHorizontalAlignment)
        {
        case UIControlContentHorizontalAlignmentLeft:
            field.textAlignment = textFieldHolder->useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentLeft;
            break;
        case UIControlContentVerticalAlignmentCenter:
            field.textAlignment = NSTextAlignmentCenter;
            break;
        case UIControlContentHorizontalAlignmentRight:
            field.textAlignment = textFieldHolder->useRtlAlign ? NSTextAlignmentNatural : NSTextAlignmentRight;
            break;
        default:
            break;
        }

        isNeedToUpdateTexture = true;
        }
        else
    {
        DAVA::Logger::Error("UITextField::SetTextAlign not supported in multiline");
    }
    }

    DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        DAVA::int32 retValue = 0;
        UIView* view = textFieldHolder->textCtrl;
        if (isSingleLine)
        {
            ::UITextField* field = (::UITextField*)view;

            UIControlContentHorizontalAlignment horAligment = field.contentHorizontalAlignment;
            UIControlContentVerticalAlignment verAligment = field.contentVerticalAlignment;

            switch (horAligment)
            {
            case UIControlContentHorizontalAlignmentLeft:
                retValue |= ALIGN_LEFT;
                break;
            case UIControlContentHorizontalAlignmentCenter:
                retValue |= ALIGN_HCENTER;
                break;
            case UIControlContentHorizontalAlignmentRight:
                retValue |= ALIGN_RIGHT;
                break;

            default:
                break;
            }

            switch (verAligment)
            {
            case UIControlContentVerticalAlignmentTop:
                retValue |= ALIGN_TOP;
                break;
            case UIControlContentVerticalAlignmentCenter:
                retValue |= ALIGN_VCENTER;
                break;
            case UIControlContentVerticalAlignmentBottom:
                retValue |= ALIGN_BOTTOM;
                break;
            default:
                break;
            }
    }

    return retValue;
    }

    void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setUseRtlAlign:useRtlAlign];
        isNeedToUpdateTexture = true;
    }

    bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        return textFieldHolder->useRtlAlign == YES;
    }

    void TextFieldPlatformImpl::OpenKeyboard()
    {
        auto OnKeyboardFrameChange = [](CFNotificationCenterRef center, void* observer,
                                        CFStringRef name, const void* object, CFDictionaryRef userInfo) {
            NSDictionary* userInfoDic = (__bridge NSDictionary*)userInfo;

            CGRect keyboardEndFrame = [[userInfoDic objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
            CGRect screenRect = [[ ::UIScreen mainScreen] bounds];

            TextFieldPlatformImpl* tf = (static_cast<TextFieldPlatformImpl*>(observer));

            if (CGRectIntersectsRect(keyboardEndFrame, screenRect))
            {
                // Keyboard did show or move
                tf->deltaMoveControl = 0;
                tf->UpdateStaticTexture();
            }
            else
            {
                // Keyboard did hide
                if (!tf->renderToTexture)
                {
                    // workaround if user click hide softkeyboard but we don't lose
                    // focus on current control just leave native control on screen
                }
                else
                {
                    tf->deltaMoveControl = MOVE_TO_OFFSCREEN_STEP;
                }
                tf->UpdateStaticTexture();
            }
        };

        // Add keyboard frame change observer
        CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(),
                                        this,
                                        OnKeyboardFrameChange,
                                        (__bridge CFStringRef)UIKeyboardDidChangeFrameNotification,
                                        nil,
                                        CFNotificationSuspensionBehaviorDeliverImmediately);

        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textCtrl.userInteractionEnabled = YES;
        [textFieldHolder->textCtrl becomeFirstResponder];
    }

    void TextFieldPlatformImpl::CloseKeyboard()
    {
        // Remove keyboard frame change observer
        CFNotificationCenterRemoveObserver(CFNotificationCenterGetLocalCenter(),
                                           this,
                                           (__bridge CFStringRef)UIKeyboardDidChangeFrameNotification,
                                           nil);

        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (isSingleLine)
        {
            textFieldHolder->textCtrl.userInteractionEnabled = NO;
    }
    [textFieldHolder->textCtrl resignFirstResponder];
    }

    void TextFieldPlatformImpl::ShowField()
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        DVASSERT([textFieldHolder superview] != nil);
        [textFieldHolder setHidden:NO];
        [textFieldHolder->textCtrl setHidden:NO];

        // Attach to "keyboard shown/keyboard hidden" notifications.
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:textFieldHolder
                   selector:@selector(keyboardDidShow:)
                       name:UIKeyboardDidShowNotification
                     object:nil];
        [center addObserver:textFieldHolder
                   selector:@selector(keyboardWillHide:)
                       name:UIKeyboardWillHideNotification
                     object:nil];
        [center addObserver:textFieldHolder
                   selector:@selector(keyboardFrameDidChange:)
                       name:UIKeyboardDidChangeFrameNotification
                     object:nil];
    }

    void TextFieldPlatformImpl::HideField()
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setHidden:YES];

        // Attach to "keyboard shown/keyboard hidden" notifications.
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center removeObserver:textFieldHolder name:UIKeyboardDidShowNotification object:nil];
        [center removeObserver:textFieldHolder name:UIKeyboardWillHideNotification object:nil];
        [center removeObserver:textFieldHolder name:UIKeyboardDidChangeFrameNotification object:nil];
    }

    void TextFieldPlatformImpl::UpdateNativeRect(const Rect& virtualRect, int xOffset)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        DAVA::float32 divider = DAVA::Core::Instance()->GetScreenScaleFactor();
        DAVA::Rect physicalRect = DAVA::VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(virtualRect);
        DAVA::Vector2 physicalOffset = DAVA::VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
        CGRect nativeRect = CGRectMake((physicalRect.x + physicalOffset.x) / divider, (physicalRect.y + physicalOffset.y) / divider, physicalRect.dx / divider, physicalRect.dy / divider);

        nativeRect = CGRectIntegral(nativeRect);
        nativeRect.origin.x += xOffset;
        textFieldHolder->textCtrl.frame = nativeRect;
    }

    void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
    {
        nextRect = rect;
    }

    void TextFieldPlatformImpl::SetText(const WideString& string)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        NSString* text = [[[NSString alloc] initWithBytes:(char*)string.data()
                                                   length:string.size() * sizeof(wchar_t)
                                                 encoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)] autorelease];
        NSString* truncatedText = (NSString*)TruncateText(text, textFieldHolder->cppTextField->GetMaxLength());

        UIView* view = textFieldHolder->textCtrl;
        NSString* textInField = [view valueForKey:@"text"];
        DVASSERT(nullptr != textInField);
        bool textChanged = ![textInField isEqualToString:truncatedText];

        [view setValue:truncatedText forKey:@"text"];
        // Drop cached text in text field holder for correct dispatching OnTextChanged event
        [textFieldHolder dropCachedText];

        [textFieldHolder->textCtrl.undoManager removeAllActions];

        if (textChanged || string.empty())
    {
        isNeedToUpdateTexture = true;
    }
    }

    void TextFieldPlatformImpl::GetText(WideString& string) const
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        UIView* view = textFieldHolder->textCtrl;
        NSString* textInField = [view valueForKey:@"text"];

        DVASSERT(nullptr != textInField);

        const char* cstr = [textInField cStringUsingEncoding:NSUTF8StringEncoding];
        DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)cstr, strlen(cstr), string);
    }

    void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setIsPassword:isPassword];
        isNeedToUpdateTexture = true;
    }

    void TextFieldPlatformImpl::SetInputEnabled(bool value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder setTextInputAllowed:value];
    }

    void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UIView* view = textFieldHolder->textCtrl;

        UITextAutocapitalizationType type_ = [textFieldHolder convertAutoCapitalizationType:
                                                              (DAVA::UITextField::eAutoCapitalizationType)value];
        if (isSingleLine)
        {
            ::UITextField* field = (::UITextField*)view;
            field.autocapitalizationType = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.autocapitalizationType = type_;
        }
    }

    void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        UIView* view = textFieldHolder->textCtrl;
        UITextAutocorrectionType type_ = [textFieldHolder convertAutoCorrectionType:
                                                          (DAVA::UITextField::eAutoCorrectionType)value];
        if (isSingleLine)
        {
            ::UITextField* field = (::UITextField*)view;
            field.autocorrectionType = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.autocorrectionType = type_;
        }
    }

    void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
    {
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_5_0
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UITextSpellCheckingType type_ = [textFieldHolder convertSpellCheckingType:
                                                         (DAVA::UITextField::eSpellCheckingType)value];

        UIView* view = textFieldHolder->textCtrl;
        if (isSingleLine)
        {
            ::UITextField* field = (::UITextField*)view;
            field.spellCheckingType = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.spellCheckingType = type_;
        }
#endif
    }

    void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UIView* view = textFieldHolder->textCtrl;
        UIKeyboardAppearance type_ = [textFieldHolder convertKeyboardAppearanceType:
                                                      (DAVA::UITextField::eKeyboardAppearanceType)value];
        if (isSingleLine)
        {
            ::UITextField* textField = (::UITextField*)view;
            textField.keyboardAppearance = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.keyboardAppearance = type_;
        }
    }

    void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UIView* view = textFieldHolder->textCtrl;
        UIKeyboardType type_ = [textFieldHolder convertKeyboardType:
                                                (DAVA::UITextField::eKeyboardType)value];
        if (isSingleLine)
        {
            ::UITextField* textField = (::UITextField*)view;
            textField.keyboardType = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.keyboardType = type_;
        }
    }

    void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        UIReturnKeyType type_ = [textFieldHolder convertReturnKeyType:
                                                 (DAVA::UITextField::eReturnKeyType)value];
        UIView* view = textFieldHolder->textCtrl;
        if (isSingleLine)
        {
            ::UITextField* textField = (::UITextField*)view;
            textField.returnKeyType = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.returnKeyType = type_;
        }
    }

    void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        BOOL type_ = [textFieldHolder convertEnablesReturnKeyAutomatically:value];
        UIView* view = textFieldHolder->textCtrl;
        if (isSingleLine)
        {
            ::UITextField* textField = (::UITextField*)view;
            textField.enablesReturnKeyAutomatically = type_;
        }
        else
        {
            ::UITextView* textView = (::UITextView*)view;
            textView.enablesReturnKeyAutomatically = type_;
        }
    }

    uint32 TextFieldPlatformImpl::GetCursorPos()
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (!textFieldHolder)
    {
        return 0;
    }

    UIView* view = textFieldHolder->textCtrl;
    int32 pos = 0;
    if (isSingleLine)
    {
        ::UITextField* textField = (::UITextField*)view;
        pos = static_cast<int32>([textField offsetFromPosition:textField.beginningOfDocument
                                                    toPosition:textField.selectedTextRange.start]);
    }
    else
    {
        ::UITextView* textView = (::UITextView*)view;
        pos = static_cast<int32>([textView offsetFromPosition:textView.beginningOfDocument
                                                   toPosition:textView.selectedTextRange.start]);
    }
    return pos;
    }

    void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (!textFieldHolder)
    {
        return;
    }

    UIView* view = textFieldHolder->textCtrl;
    NSString* text = [view valueForKey:@"text"];
    NSUInteger textLength = [text length];
    if (textLength == 0)
    {
        return;
    }
    if (pos > textLength)
    {
        pos = static_cast<uint32>(textLength - 1);
    }

    if (isSingleLine)
    {
        ::UITextField* textField = (::UITextField*)view;
        UITextPosition* start = [textField positionFromPosition:[textField beginningOfDocument] offset:pos];
        UITextPosition* end = [textField positionFromPosition:start offset:0];
        [textField setSelectedTextRange:[textField textRangeFromPosition:start toPosition:end]];
    }
    else
    {
        ::UITextView* textView = (::UITextView*)view;
        UITextPosition* start = [textView positionFromPosition:[textView beginningOfDocument] offset:pos];
        UITextPosition* end = [textView positionFromPosition:start offset:0];
        [textView setSelectedTextRange:[textView textRangeFromPosition:start toPosition:end]];
    }
    }

    void TextFieldPlatformImpl::SetVisible(bool value)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;

        [textFieldHolder setHidden:value == false];
    }

    void TextFieldPlatformImpl::SetMaxLength(int maxLength)
    {
        UITextFieldHolder* textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (textFieldHolder)
    {
        UIView* view = textFieldHolder->textCtrl;
        NSString* currentText = [view valueForKey:@"text"];
        DVASSERT(currentText);
        NSString* newText = (NSString*)TruncateText(currentText, maxLength);
        DVASSERT(newText);
        [view setValue:newText forKey:@"text"];
    }
    }

    void TextFieldPlatformImpl::UpdateStaticTexture()
    {
        UITextFieldHolder* textFieldHolder = static_cast<UITextFieldHolder*>(objcClassPtr);
        DVASSERT(textFieldHolder);

        ::UIView* textView = textFieldHolder->textCtrl;
        DVASSERT(textView);
        NSString* text = [textView valueForKey:@"text"];

        if (renderToTexture && deltaMoveControl != 0 && text.length > 0)
        {
            void* imgPtr = DAVA::WebViewControl::RenderIOSUIViewToImage(textView);
            ::UIImage* image = static_cast<::UIImage*>(imgPtr);
            if (nullptr != image) // can't render to empty rect so skip
            {
                // set backgroud image into davaTextField control
                WebViewControl::SetImageAsSpriteToControl(image, davaTextField);
        }
        isNeedToUpdateTexture = false;
    }
    else
    {
        // set null background
        davaTextField.GetBackground()->SetSprite(nullptr, 0);
    }
    }

    void TextFieldPlatformImpl::SetMultiline(bool multiline)
    {
        UITextFieldHolder* textFieldHolder = static_cast<UITextFieldHolder*>(objcClassPtr);
        DVASSERT(textFieldHolder);

        if (isSingleLine && multiline)
    {
        // store current properties, font, size, text etc.

        DAVA::int32 cursorPos = GetCursorPos();
        DAVA::WideString wstring;
        GetText(wstring);
        // font, textColor, frameRect
        ::UITextField* textField = (::UITextField*)textFieldHolder->textCtrl;
        UIFont* font = textField.font;
        UIColor* color = textField.textColor;
        CGRect rect = textField.frame;
        BOOL isHidden = textField.isHidden;

        // now hide textField and store it for future restore
        [textFieldHolder->textCtrl removeFromSuperview];
        [textFieldHolder->textCtrl setHidden:YES];
        textFieldHolder->textField = (::UITextField*)textFieldHolder->textCtrl;

        // replace textField with new textView and apply current properties
        ::UITextView* textView = [[UITextView alloc] initWithFrame:rect textContainer:nil];

        //Workaround: fix OS built-in padding on UITextView
        //See http://foobarpig.com/iphone/get-rid-of-uitextview-padding.html
        //See http://stackoverflow.com/questions/746670/how-to-lose-margin-padding-in-uitextview
        textView.contentInset = UIEdgeInsetsMake(-10, -5, 0, 0);

        HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
        BackgroundView* backgroundView = [[appDelegate renderViewController] backgroundView];
        [backgroundView PrepareView:textFieldHolder->textCtrl];

        [textFieldHolder addSubview:textView];

        textFieldHolder->textCtrl = textView;

        textView.textColor = color;
        textView.font = font;
        // Workaround! in multiline mode use need ability to scroll
        // text without open keyboard
        textView.userInteractionEnabled = YES;
        [textView setHidden:isHidden];
        textView.delegate = textFieldHolder;

        isSingleLine = false;

        SetText(wstring);
        SetCursorPos(cursorPos);

        [textFieldHolder setupTraits];

        [textView setBackgroundColor:[UIColor clearColor]];

        textView.scrollEnabled = YES;

        [textView release];
        // Workaround! in multiline mode always listen for user
        // touches
        SetRenderToTexture(false);
    }
    else if (!isSingleLine && !multiline)
    {
        // revert back single line native control
        // TODO in future completely remove UITextField native control
        //
        // store current properties, font, size, text etc.
        DAVA::int32 cursorPos = GetCursorPos();
        DAVA::WideString wstring;
        GetText(wstring);
        // font, textColor, frameRect
        ::UITextView* textView = (::UITextView*)textFieldHolder->textCtrl;
        UIFont* font = textView.font;
        UIColor* color = textView.textColor;
        BOOL isHidden = textView.isHidden;

        // now hide textField and store it for future restore
        [textView removeFromSuperview];
        [textView setHidden:YES];

        // replace textField with old textField and apply current properties
        ::UITextField* textField = textFieldHolder->textField;
        textFieldHolder->textField = nullptr;
        [textFieldHolder addSubview:textField];
        [textView setHidden:isHidden];

        textView = nullptr;

        textFieldHolder->textCtrl = textField;

        textField.textColor = color;
        textField.font = font;
        textField.userInteractionEnabled = YES;
        [textField setHidden:isHidden];

        isSingleLine = true;

        SetText(wstring);
        SetCursorPos(cursorPos);

        [textFieldHolder setupTraits];

        [textField setBackgroundColor:[UIColor clearColor]];
    }
    }

    void TextFieldPlatformImpl::SetRenderToTexture(bool value)
    {
        if (renderToTexture == value)
            return;

        renderToTexture = value;

        if (renderToTexture)
    {
        deltaMoveControl = MOVE_TO_OFFSCREEN_STEP;
    }
    else
    {
        deltaMoveControl = 0;
    }

    isNeedToUpdateTexture = true;
    }

    bool TextFieldPlatformImpl::IsRenderToTexture() const
    {
        return renderToTexture;
    }

    void TextFieldPlatformImpl::SystemDraw(const UIGeometricData& geometricData)
    {
        UpdateNativeRect(nextRect, deltaMoveControl);
        if (nextRect.dx != prevRect.dx || nextRect.dy != prevRect.dy || isNeedToUpdateTexture)
    {
        UpdateStaticTexture();
    }
    prevRect = nextRect;
    }

    void* TextFieldPlatformImpl::TruncateText(void* text, int maxLength)
    {
        if (maxLength >= 0)
    {
        NSString* textString = (NSString*)text;
        NSUInteger textLimit = MIN([textString length], (NSUInteger)maxLength);
        return [textString substringToIndex:textLimit];
    }

    return text;
    }
}

#endif
