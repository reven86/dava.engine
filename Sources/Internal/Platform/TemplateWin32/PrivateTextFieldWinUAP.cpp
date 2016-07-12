#include "Platform/TemplateWin32/PrivateTextFieldWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

#include "Math/Color.h"

#include "Concurrency/LockGuard.h"

#include "UI/UIControlSystem.h"
#include "UI/UITextField.h"
#include "UI/Focus/FocusHelpers.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/Image/Image.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#include "Engine/Public/WindowNativeService.h"
#else
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#endif

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

    enum eDiffType
    {
        NO_CHANGE = 0, // No changes between original string and new string
        INSERTION, // Character range was inserted into new string
        DELETION, // Character range was deleted from original string
        REPLACEMENT // Character range in original string was replaced by another character range in new string
    };

    eDiffType diffType = NO_CHANGE;
    int32 originalStringDiffPosition = 0; // Position in original string where difference starts
    int32 newStringDiffPosition = 0; // Position in new string where difference starts
    WideString originalStringDiff; // What was changed in original string
    WideString newStringDiff; // What was changed in new string

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
    // Skip same characters from the beginning of both strings
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
    // Skip same characters from the end of both strings
    while (origDiffEnd >= origDiffBegin && newDiffEnd >= newDiffBegin && originalString[origDiffEnd] == newString[newDiffEnd])
    {
        origDiffEnd -= 1;
        newDiffEnd -= 1;
    }

    if (origDiffEnd < origDiffBegin) // Insertion took place
    {
        result.diffType = StringDiffResult::INSERTION;
        result.originalStringDiffPosition = origDiffBegin;
        result.originalStringDiff = WideString();
        result.newStringDiffPosition = newDiffBegin;
        result.newStringDiff = WideString(newString, newDiffBegin, newDiffEnd - newDiffBegin + 1);
    }
    else if (newDiffEnd < origDiffBegin) // Deletion took place
    {
        result.diffType = StringDiffResult::DELETION;
        result.originalStringDiffPosition = origDiffBegin;
        result.originalStringDiff = WideString(originalString, origDiffBegin, origDiffEnd - origDiffBegin + 1);
        result.originalStringDiffPosition = newDiffBegin;
        result.newStringDiff = WideString();
    }
    else // Replacement took place
    {
        result.diffType = StringDiffResult::REPLACEMENT;
        result.originalStringDiffPosition = origDiffBegin;
        result.originalStringDiff = WideString(originalString, origDiffBegin, origDiffEnd - origDiffBegin + 1);
        result.newStringDiffPosition = newDiffBegin;
        result.newStringDiff = WideString(newString, newDiffBegin, newDiffEnd - newDiffBegin + 1);
    }
}

} // unnamed namespace

PrivateTextFieldWinUAP::TextFieldProperties::TextFieldProperties()
    : createNew(false)
    , focusChanged(false)
    , anyPropertyChanged(false)
    , rectChanged(false)
    , textChanged(false)
    , textColorChanged(false)
    , visibleChanged(false)
    , passwordChanged(false)
    , multilineChanged(false)
    , inputEnabledChanged(false)
    , spellCheckingEnabledChanged(false)
    , textRtlAlignmentChanged(false)
    , textAlignmentChanged(false)
    , maxTextLengthChanged(false)
    , keyboardTypeChanged(false)
    , caretPositionChanged(false)
    , fontSizeChanged(false)
    , rectAssigned(false)
    , textAssigned(false)
    , textColorAssigned(false)
    , visibleAssigned(false)
    , passwordAssigned(false)
    , multilineAssigned(false)
    , inputEnabledAssigned(false)
    , spellCheckingEnabledAssigned(false)
    , textRtlAlignmentAssigned(false)
    , textAlignmentAssigned(false)
    , maxTextLengthAssigned(false)
    , keyboardTypeAssigned(false)
    , caretPositionAssigned(false)
    , fontSizeAssigned(false)
{
}

void PrivateTextFieldWinUAP::TextFieldProperties::ClearChangedFlags()
{
    anyPropertyChanged = false;
    rectChanged = false;
    textChanged = false;
    textColorChanged = false;
    visibleChanged = false;
    passwordChanged = false;
    multilineChanged = false;
    inputEnabledChanged = false;
    spellCheckingEnabledChanged = false;
    textRtlAlignmentChanged = false;
    textAlignmentChanged = false;
    maxTextLengthChanged = false;
    keyboardTypeChanged = false;
    caretPositionChanged = false;
    fontSizeChanged = false;
}

Windows::UI::Xaml::Style ^ PrivateTextFieldWinUAP::customTextBoxStyle = nullptr;
Windows::UI::Xaml::Style ^ PrivateTextFieldWinUAP::customPasswordBoxStyle = nullptr;

PrivateTextFieldWinUAP::PrivateTextFieldWinUAP(UITextField* uiTextField_)
#if defined(__DAVAENGINE_COREV2__)
    : window(Engine::Instance()->PrimaryWindow())
#else
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
#endif
    , uiTextField(uiTextField_)
    , properties()
{
    uiTextField->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    properties.createNew = true;
}

PrivateTextFieldWinUAP::~PrivateTextFieldWinUAP()
{
    if (nativeControl != nullptr)
    {
        UIElement ^ p = nativeControlHolder;
        EventRegistrationToken tokenHiding = tokenKeyboardHiding;
        EventRegistrationToken tokenShowing = tokenKeyboardShowing;
#if defined(__DAVAENGINE_COREV2__)
        WindowNativeService* nservice = window->GetNativeService();
        window->RunAsyncOnUIThread([p, nservice, tokenHiding, tokenShowing]() {
            InputPane::GetForCurrentView()->Showing -= tokenHiding;
            InputPane::GetForCurrentView()->Hiding -= tokenShowing;
            nservice->RemoveXamlControl(p);
        });
#else
        core->RunOnUIThread([p, tokenHiding, tokenShowing]() { // We don't need blocking call here
            InputPane::GetForCurrentView()->Showing -= tokenHiding;
            InputPane::GetForCurrentView()->Hiding -= tokenShowing;
            static_cast<CorePlatformWinUAP*>(Core::Instance())->XamlApplication()->RemoveUIElement(p);
        });
#endif
        nativeControlHolder = nullptr;
        nativeControl = nullptr;
        nativeText = nullptr;
        nativePassword = nullptr;
    }
}

void PrivateTextFieldWinUAP::OwnerAtPremortem()
{
    uiTextField = nullptr;
    textFieldDelegate = nullptr;
}

void PrivateTextFieldWinUAP::SetVisible(bool isVisible)
{
    if (properties.visible != isVisible)
    {
        properties.visible = isVisible;
        properties.visibleChanged = true;
        properties.visibleAssigned = true;
        properties.anyPropertyChanged = true;
        if (!isVisible)
        { // Immediately hide native control if it has been already created
            auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
            window->RunAsyncOnUIThread([this, self]() {
                if (nativeControl != nullptr)
                {
                    SetNativeVisible(false);
                }
            });
#else
            core->RunOnUIThread([this, self]() {
                if (nativeControl != nullptr)
                {
                    SetNativeVisible(false);
                }
            });
#endif
        }
    }
}

void PrivateTextFieldWinUAP::SetIsPassword(bool isPassword)
{
    // Do not allow multiline password fields
    DVASSERT((!isPassword || !properties.multiline) && "Password multiline text fields are not allowed");
    if (isPassword != properties.password)
    {
        properties.password = isPassword;
        properties.passwordChanged = true;
        properties.passwordAssigned = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateTextFieldWinUAP::SetMaxLength(int32 value)
{
    properties.maxTextLength = value;
    properties.maxTextLengthChanged = true;
    properties.maxTextLengthAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::OpenKeyboard()
{
    properties.focus = true;
    properties.focusChanged = true;
}

void PrivateTextFieldWinUAP::CloseKeyboard()
{
    properties.focus = false;
    properties.focusChanged = true;
}

void PrivateTextFieldWinUAP::UpdateRect(const Rect& rect)
{
    if (properties.rect != rect)
    {
        properties.rect = rect;
        properties.rectInWindowSpace = VirtualToWindow(rect);
        properties.rectChanged = true;
        properties.rectAssigned = true;
        properties.anyPropertyChanged = true;
    }

    if (properties.createNew || properties.anyPropertyChanged || properties.focusChanged)
    {
        if (properties.textChanged && properties.focusChanged && properties.focus)
            uiTextField->SetSprite(nullptr, 0);

        auto self{ shared_from_this() };
        TextFieldProperties props(properties);
#if defined(__DAVAENGINE_COREV2__)
        window->RunAsyncOnUIThread([this, self, props] {
            ProcessProperties(props);
        });
#else
        core->RunOnUIThread([this, self, props] {
            ProcessProperties(props);
        });
#endif

        properties.createNew = false;
        properties.focusChanged = false;
        properties.ClearChangedFlags();
    }
}

void PrivateTextFieldWinUAP::SetText(const WideString& text)
{
    // Do not set same text again as TextChanged event not fired after setting equal text
    if (text.length() == curText.length() && text == curText)
        return;
    properties.text = text;
    properties.textChanged = true;
    properties.textAssigned = true;
    properties.anyPropertyChanged = true;
    properties.caretPosition = static_cast<int32>(text.length());
    properties.caretPositionChanged = true;

    lastProgrammaticText = curText;
    curText = text;
    if (text.empty())
    { // Immediatly remove sprite image if new text is empty to get rid of some flickering
        uiTextField->SetSprite(nullptr, 0);
    }
    programmaticTextChange = true;
}

void PrivateTextFieldWinUAP::GetText(WideString& text) const
{
    text = curText;
}

void PrivateTextFieldWinUAP::SetTextColor(const Color& color)
{
    properties.textColor = color;
    properties.textColorChanged = true;
    properties.textColorAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetTextAlign(int32 align)
{
    properties.textAlignment = align;
    properties.textAlignmentChanged = true;
    properties.textAlignmentAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetTextUseRtlAlign(bool useRtlAlign)
{
    properties.textRtlAlignment = useRtlAlign;
    properties.textRtlAlignmentChanged = true;
    properties.textRtlAlignmentAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetFontSize(float32 virtualFontSize)
{
#if defined(__DAVAENGINE_COREV2__)
    const float32 scaleFactor = window->GetRenderSurfaceScaleX();
#else
    const float32 scaleFactor = core->GetScreenScaleFactor();
#endif
    float32 fontSize = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(virtualFontSize);
    fontSize /= scaleFactor;

    properties.fontSize = fontSize;
    properties.fontSizeChanged = true;
    properties.fontSizeAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetDelegate(UITextFieldDelegate* textFieldDelegate_)
{
    textFieldDelegate = textFieldDelegate_;
}

void PrivateTextFieldWinUAP::SetMultiline(bool enable)
{
    // Do not allow multiline password fields
    DVASSERT((!enable || !properties.password) && "Password multiline text fields are not allowed");
    if (properties.multiline != enable)
    {
        properties.multiline = enable;
        properties.multilineChanged = true;
        properties.multilineAssigned = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateTextFieldWinUAP::SetInputEnabled(bool enable)
{
    properties.inputEnabled = enable;
    properties.inputEnabledChanged = true;
    properties.inputEnabledAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetSpellCheckingType(int32 value)
{
    properties.spellCheckingEnabled = UITextField::SPELL_CHECKING_TYPE_YES == value;
    properties.spellCheckingEnabledChanged = true;
    properties.spellCheckingEnabledAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetAutoCapitalizationType(int32 /*value*/)
{
    // I didn't find this property in native TextBox
}

void PrivateTextFieldWinUAP::SetAutoCorrectionType(int32 /*value*/)
{
    // I didn't find this property in native TextBox
}

void PrivateTextFieldWinUAP::SetKeyboardAppearanceType(int32 /*value*/)
{
    // I didn't find this property in native TextBox
}

void PrivateTextFieldWinUAP::SetKeyboardType(int32 value)
{
    properties.keyboardType = value;
    properties.keyboardTypeChanged = true;
    properties.keyboardTypeAssigned = true;
    properties.anyPropertyChanged = true;
}

void PrivateTextFieldWinUAP::SetReturnKeyType(int32 /*value*/)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetEnableReturnKeyAutomatically(bool /*value*/)
{
    // I didn't found this property in native TextBox
}

void PrivateTextFieldWinUAP::SetCursorPos(uint32 pos)
{
    if (static_cast<int32>(pos) >= 0)
    {
        properties.caretPosition = static_cast<int32>(pos);
        properties.caretPositionChanged = true;
        properties.caretPositionAssigned = true;
        properties.anyPropertyChanged = true;
    }
}

void PrivateTextFieldWinUAP::CreateNativeControl(bool textControl)
{
    if (customTextBoxStyle == nullptr)
    {
        // Load custom textbox and password styles that allow transparent background when control has focus
        using ::Windows::UI::Xaml::Markup::XamlReader;
        using ::Windows::UI::Xaml::ResourceDictionary;
        using ::Windows::UI::Xaml::Style;
        ResourceDictionary ^ dict = static_cast<ResourceDictionary ^>(XamlReader::Load(xamlTextBoxStyles));

        customTextBoxStyle = static_cast<Style ^>(dict->Lookup(ref new Platform::String(L"dava_custom_textbox")));
        customPasswordBoxStyle = static_cast<Style ^>(dict->Lookup(ref new Platform::String(L"dava_custom_passwordbox")));
    }

    if (textControl)
    {
        nativeText = ref new Windows::UI::Xaml::Controls::TextBox();
        nativeControl = nativeText;
#if defined(__DAVAENGINE_COREV2__)
        nativeText->Style = customTextBoxStyle;
#else
        core->XamlApplication()->SetTextBoxCustomStyle(nativeText);
#endif
        InstallTextEventHandlers();
    }
    else
    {
        nativePassword = ref new PasswordBox();
        nativeControl = nativePassword;
#if defined(__DAVAENGINE_COREV2__)
        nativePassword->Style = customPasswordBoxStyle;
#else
        core->XamlApplication()->SetPasswordBoxCustomStyle(nativePassword);
#endif
        InstallPasswordEventHandlers();
    }
    InstallCommonEventHandlers();

    nativeControl->BorderThickness = Thickness(0.0);
    nativeControl->Background = ref new SolidColorBrush(Colors::Transparent);
    nativeControl->Foreground = ref new SolidColorBrush(Colors::White);
    nativeControl->BorderBrush = ref new SolidColorBrush(Colors::Transparent);
    nativeControl->Padding = Thickness(0.0);
    nativeControl->Visibility = Visibility::Visible;
    nativeControl->MinWidth = 0.0;
    nativeControl->MinHeight = 0.0;
    nativeControl->TabNavigation = KeyboardNavigationMode::Cycle;

    // Native control holder is used to keep text control inside itself to
    // emulate vertical text alignment
    nativeControlHolder = ref new Border();
    nativeControlHolder->Background = ref new SolidColorBrush(Colors::Transparent);
    nativeControlHolder->BorderBrush = ref new SolidColorBrush(Colors::Transparent);
    nativeControlHolder->BorderThickness = Thickness(0.0);
    nativeControlHolder->Padding = Thickness(0.0);
    nativeControlHolder->Margin = Thickness(0.0);
    nativeControlHolder->MinWidth = 0.0;
    nativeControlHolder->MinHeight = 0.0;
    nativeControlHolder->Child = nativeControl;
#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->AddXamlControl(nativeControlHolder);
#else
    core->XamlApplication()->AddUIElement(nativeControlHolder);
#endif
}

void PrivateTextFieldWinUAP::DeleteNativeControl()
{
#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->RemoveXamlControl(nativeControlHolder);
#else
    core->XamlApplication()->RemoveUIElement(nativeControlHolder);
#endif
    nativeControl = nullptr;
    nativeText = nullptr;
    nativePassword = nullptr;
    nativeControlHolder = nullptr;
}

void PrivateTextFieldWinUAP::InstallCommonEventHandlers()
{
    using Platform::Object;

    std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
    auto keyDown = ref new KeyEventHandler([this, self_weak](Object ^, KeyRoutedEventArgs ^ args) {
        if (auto self = self_weak.lock())
            OnKeyDown(args);
    });
    auto gotFocus = ref new RoutedEventHandler([this, self_weak](Object ^, RoutedEventArgs ^ ) {
        if (auto self = self_weak.lock())
            OnGotFocus();
    });
    auto lostFocus = ref new RoutedEventHandler([this, self_weak](Object ^, RoutedEventArgs ^ ) {
        if (auto self = self_weak.lock())
            OnLostFocus();
    });
    nativeControl->KeyDown += keyDown;
    nativeControl->GotFocus += gotFocus;
    nativeControl->LostFocus += lostFocus;
}

void PrivateTextFieldWinUAP::InstallTextEventHandlers()
{
    using Platform::Object;

    std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
    auto selectionChanged = ref new RoutedEventHandler([this, self_weak](Object ^, RoutedEventArgs ^ ) {
        if (auto self = self_weak.lock())
            OnSelectionChanged();
    });
    auto textChanged = ref new TextChangedEventHandler([this, self_weak](Object ^, TextChangedEventArgs ^ ) {
        if (auto self = self_weak.lock())
            OnTextChanged();
    });
    nativeText->SelectionChanged += selectionChanged;
    nativeText->TextChanged += textChanged;
}

void PrivateTextFieldWinUAP::InstallPasswordEventHandlers()
{
    using namespace Platform;

    std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
    auto passwordChanged = ref new RoutedEventHandler([this, self_weak](Object ^, RoutedEventArgs ^ ) {
        if (auto self = self_weak.lock())
            OnTextChanged();
    });
    nativePassword->PasswordChanged += passwordChanged;
}

void PrivateTextFieldWinUAP::InstallKeyboardEventHandlers()
{
    std::weak_ptr<PrivateTextFieldWinUAP> self_weak(shared_from_this());
    auto keyboardHiding = ref new TypedEventHandler<InputPane ^, InputPaneVisibilityEventArgs ^>([this, self_weak](InputPane ^, InputPaneVisibilityEventArgs ^ args) {
        if (auto self = self_weak.lock())
            OnKeyboardHiding(args);
    });
    auto keyboardShowing = ref new TypedEventHandler<InputPane ^, InputPaneVisibilityEventArgs ^>([this, self_weak](InputPane ^, InputPaneVisibilityEventArgs ^ args) {
        if (auto self = self_weak.lock())
            OnKeyboardShowing(args);
    });
    tokenKeyboardShowing = InputPane::GetForCurrentView()->Showing += keyboardShowing;
    tokenKeyboardHiding = InputPane::GetForCurrentView()->Hiding += keyboardHiding;
}

void PrivateTextFieldWinUAP::OnKeyDown(KeyRoutedEventArgs ^ args)
{
    savedCaretPosition = GetNativeCaretPosition();

    switch (args->Key)
    {
    case VirtualKey::Back:
        savedCaretPosition += 1;
        break;
    case VirtualKey::Tab:
        args->Handled = true; // To avoid handling tab navigation by windows. We will handle navigation by our focus system.
        break;
    case VirtualKey::Escape:
    {
        auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
        window->GetEngine()->RunAsyncOnMainThread([this, self]() {
            if (textFieldDelegate != nullptr)
                textFieldDelegate->TextFieldShouldCancel(uiTextField);
        });
#else
        core->RunOnMainThread([this, self]() {
            if (textFieldDelegate != nullptr)
                textFieldDelegate->TextFieldShouldCancel(uiTextField);
        });
#endif
    }
    break;
    case VirtualKey::Enter:
        // Known XAML bug: native TextBox generates two OnKeyDown events
        // So use RepeatCount field to filter out extra event: second event comes with RepeatCount > 0
        if (!IsMultiline() && 0 == args->KeyStatus.RepeatCount)
        {
            auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
            window->GetEngine()->RunAsyncOnMainThread([this, self]() {
                if (textFieldDelegate != nullptr)
                    textFieldDelegate->TextFieldShouldReturn(uiTextField);
            });
#else
            core->RunOnMainThread([this, self]() {
                if (textFieldDelegate != nullptr)
                    textFieldDelegate->TextFieldShouldReturn(uiTextField);
            });
#endif
        }
        break;
    default:
        break;
    }
}

void PrivateTextFieldWinUAP::OnGotFocus()
{
#if defined(__DAVAENGINE_COREV2__)
// TODO: core->XamlApplication()->CaptureTextBox(nativeControl);
#else
    core->XamlApplication()->CaptureTextBox(nativeControl);
#endif

    SetNativeCaretPosition(GetNativeText()->Length());

    Windows::Foundation::Rect nativeKeyboardRect = InputPane::GetForCurrentView()->OccludedRect;
    DAVA::Rect keyboardRect(nativeKeyboardRect.X, nativeKeyboardRect.Y, nativeKeyboardRect.Width, nativeKeyboardRect.Height);

    bool multiline = IsMultiline();
    if (!multiline)
    {
        SetNativePositionAndSize(rectInWindowSpace, false);
    }
    auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
    window->GetEngine()->RunAsyncOnMainThread([this, self, multiline, keyboardRect]() {
#else
    core->RunOnMainThread([this, self, multiline, keyboardRect]() {
#endif
        if (uiTextField != nullptr)
        {
            if (!multiline)
                uiTextField->SetSprite(nullptr, 0);

            // Manually set focus through direct call to UITextField::SetFocused()
            // Reason: UIControlSystem has no chance to know whether control has got focus when
            // one of the following occurs:
            // 1. click on text field in multiline mode as it is always shown on screen
            // 2. tab navigation
            UIControl* curFocused = UIControlSystem::Instance()->GetFocusedControl();
            if (curFocused != uiTextField && FocusHelpers::CanFocusControl(uiTextField))
            {
                uiTextField->SetFocused();
            }
            if (!uiTextField->IsEditing())
            {
                uiTextField->StartEdit();
            }

            // Sometimes OnKeyboardShowing event does not fired when keyboard is already on screen
            // If keyboard rect is not empty so manually notify delegate about keyboard size and position
            if (textFieldDelegate != nullptr && keyboardRect.dx != 0 && keyboardRect.dy != 0)
            {
                Rect rect = WindowToVirtual(keyboardRect);
                uiTextField->OnKeyboardShown(rect);
            }
        }
    });
}

void PrivateTextFieldWinUAP::OnLostFocus()
{
    if (!IsMultiline())
    {
        waitRenderToTextureComplete = true;
        RenderToTexture(true);
    }

    auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
    window->GetEngine()->RunAsyncOnMainThread([this, self]() {
#else
    core->RunOnMainThread([this, self]() {
#endif
        if (uiTextField != nullptr)
        {
            uiTextField->OnKeyboardHidden();
            uiTextField->ReleaseFocus();
        }
    });
}

void PrivateTextFieldWinUAP::OnSelectionChanged()
{
    caretPosition = GetNativeCaretPosition();
}

void PrivateTextFieldWinUAP::OnTextChanged()
{
    if (ignoreTextChange || nullptr == nativeControl)
    {
        ignoreTextChange = false;
        return;
    }

    WideString textToRestore;
    WideString newText(GetNativeText()->Data());
    if (IsMultiline())
    { // Remove '\r' characters
        auto i = std::remove_if(newText.begin(), newText.end(), [](wchar_t c) -> bool { return c == L'\r'; });
        newText.erase(i, newText.end());
    }

    bool textAccepted = true;
    auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
    window->GetEngine()->RunAndWaitOnMainThread([this, self, &newText, &textAccepted, &textToRestore]() {
#else
    core->RunOnMainThreadBlocked([this, self, &newText, &textAccepted, &textToRestore]() {
#endif
        bool targetAlive = uiTextField != nullptr && textFieldDelegate != nullptr;
        if (programmaticTextChange && targetAlive && newText != lastProgrammaticText)
        {
            // Event has originated from SetText() method so only notify delegate about text change
            textFieldDelegate->TextFieldOnTextChanged(uiTextField, newText, lastProgrammaticText, UITextFieldDelegate::eReason::CODE);
        }
        else if (targetAlive)
        {
            StringDiffResult diffR;
            StringDiff(curText, newText, diffR);
            if (diffR.diffType != StringDiffResult::NO_CHANGE)
            {
                textAccepted = textFieldDelegate->TextFieldKeyPressed(
                uiTextField,
                diffR.originalStringDiffPosition,
                static_cast<int32>(diffR.originalStringDiff.length()),
                diffR.newStringDiff);
                if (textAccepted)
                    textFieldDelegate->TextFieldOnTextChanged(uiTextField, newText, curText, UITextFieldDelegate::eReason::USER);
            }
        }
        programmaticTextChange = false;
        textAccepted ? curText = newText : textToRestore = curText;
    });

    if (!textAccepted)
    {
        // Restore control's text and caret position as before text change
        SetNativeText(textToRestore);
        SetNativeCaretPosition(savedCaretPosition);
        ignoreTextChange = true;
    }
}

void PrivateTextFieldWinUAP::OnKeyboardHiding(InputPaneVisibilityEventArgs ^ args)
{
    args->EnsuredFocusedElementInView = true;
}

void PrivateTextFieldWinUAP::OnKeyboardShowing(InputPaneVisibilityEventArgs ^ args)
{
    // Tell keyboard that application will position native controls by itself
    args->EnsuredFocusedElementInView = true;

    if (HasFocus())
    {
        // Use qualified Rect type to exclude name clash
        Windows::Foundation::Rect srcRect = InputPane::GetForCurrentView()->OccludedRect;
        DAVA::Rect keyboardRect(srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height);

        auto self{ shared_from_this() };
#if defined(__DAVAENGINE_COREV2__)
        window->GetEngine()->RunAsyncOnMainThread([this, self, keyboardRect]() {
#else
        core->RunOnMainThread([this, self, keyboardRect]() {
#endif
            if (textFieldDelegate != nullptr)
            {
                Rect rect = WindowToVirtual(keyboardRect);
                textFieldDelegate->OnKeyboardShown(rect);
            }
        });
    }
}

void PrivateTextFieldWinUAP::ProcessProperties(const TextFieldProperties& props)
{
    rectInWindowSpace = props.rectInWindowSpace;
    if (props.createNew)
    {
        waitRenderToTextureComplete = !props.multiline;
        CreateNativeControl(!props.password);
        ApplyAssignedProperties(props);
        InstallKeyboardEventHandlers();
    }
    else if (props.passwordChanged)
    {
        if (IsPassword() != props.password)
        {
            DeleteNativeControl();
            CreateNativeControl(!props.password);
            ApplyAssignedProperties(props);
        }
        else
            ApplyChangedProperties(props);
    }
    else if (props.anyPropertyChanged)
    {
        ApplyChangedProperties(props);
    }

    if (props.focusChanged)
    {
        if (props.focus)
            nativeControl->Focus(FocusState::Pointer);
        else if (HasFocus())
#if defined(__DAVAENGINE_COREV2__)
            window->GetNativeService()->UnfocusXamlControl();
#else
            core->XamlApplication()->UnfocusUIElement();
#endif
    }

    if (!IsMultiline() && !HasFocus())
    {
        RenderToTexture(waitRenderToTextureComplete);
    }
}

void PrivateTextFieldWinUAP::ApplyChangedProperties(const TextFieldProperties& props)
{
    if (props.multilineChanged)
        SetNativeMultiline(props.multiline);
    if (props.visibleChanged || props.multilineChanged)
        SetNativeVisible(props.visible);
    if (props.rectChanged)
        SetNativePositionAndSize(props.rectInWindowSpace, !(IsMultiline() || HasFocus() || waitRenderToTextureComplete));
    if (props.maxTextLengthChanged)
        SetNativeMaxTextLength(props.maxTextLength);
    if (props.textChanged)
        SetNativeText(props.text);
    if (props.textColorChanged)
        SetNativeTextColor(props.textColor);
    if (props.inputEnabledChanged)
        SetNativeInputEnabled(props.inputEnabled);
    if (props.spellCheckingEnabledChanged)
        SetNativeSpellChecking(props.spellCheckingEnabled);
    if (props.textAlignmentChanged || props.textRtlAlignmentChanged)
        SetNativeTextAlignment(props.textAlignment, props.textRtlAlignment);
    if (props.keyboardTypeChanged)
        SetNativeKeyboardType(props.keyboardType);
    if (props.caretPositionChanged)
        SetNativeCaretPosition(props.caretPosition);
    if (props.fontSizeChanged)
        SetNativeFontSize(props.fontSize);
}

void PrivateTextFieldWinUAP::ApplyAssignedProperties(const TextFieldProperties& props)
{
    if (props.multilineAssigned)
        SetNativeMultiline(props.multiline);
    if (props.visibleAssigned || props.multilineAssigned)
        SetNativeVisible(props.visible);
    if (props.rectAssigned)
        SetNativePositionAndSize(props.rectInWindowSpace, !(IsMultiline() || HasFocus() || waitRenderToTextureComplete));
    if (props.maxTextLengthAssigned)
        SetNativeMaxTextLength(props.maxTextLength);
    if (props.textAssigned)
        SetNativeText(props.text);
    if (props.textColorAssigned)
        SetNativeTextColor(props.textColor);
    if (props.inputEnabledAssigned)
        SetNativeInputEnabled(props.inputEnabled);
    if (props.spellCheckingEnabledAssigned)
        SetNativeSpellChecking(props.spellCheckingEnabled);
    if (props.textAlignmentAssigned || props.textRtlAlignmentAssigned)
        SetNativeTextAlignment(props.textAlignment, props.textRtlAlignment);
    if (props.keyboardTypeAssigned)
        SetNativeKeyboardType(props.keyboardType);
    if (props.caretPositionAssigned)
        SetNativeCaretPosition(props.caretPosition);
    if (props.fontSizeAssigned)
        SetNativeFontSize(props.fontSize);
}

void PrivateTextFieldWinUAP::SetNativePositionAndSize(const Rect& rect, bool offScreen)
{
    float32 xOffset = 0.0f;
    float32 yOffset = 0.0f;
    if (offScreen)
    {
        // Move control very far offscreen as on phone control with disabled input remains visible
        xOffset = rect.x + rect.dx + 1000.0f;
        yOffset = rect.y + rect.dy + 1000.0f;
    }
    nativeControlHolder->Width = std::max(0.0f, rect.dx);
    nativeControlHolder->Height = std::max(0.0f, rect.dy);
#if defined(__DAVAENGINE_COREV2__)
    window->GetNativeService()->PositionXamlControl(nativeControlHolder, rect.x - xOffset, rect.y - yOffset);
#else
    core->XamlApplication()->PositionUIElement(nativeControlHolder, rect.x - xOffset, rect.y - yOffset);
#endif
}

void PrivateTextFieldWinUAP::SetNativeVisible(bool visible)
{
    // Single line native text field is always rendered to texture and placed offscreen
    // Multiline native text field is always onscreen according to visibiliy flag
    if (IsMultiline())
    {
        nativeControl->Visibility = visible ? Visibility::Visible : Visibility::Collapsed;
        nativeControlHolder->Visibility = visible ? Visibility::Visible : Visibility::Collapsed;
    }
    else
    {
        // Single line TextBox is always visible to allow proper rendering into texture
        // When such a control should not be visible it is moved off screen
        // Disable TextBox in 'invisible' state to prevent tab navigation
        SetNativeInputEnabled(visible);
        if (!visible)
            SetNativePositionAndSize(rectInWindowSpace, true);
    }
}

void PrivateTextFieldWinUAP::SetNativeMultiline(bool multiline)
{
    if (!IsPassword())
    {
        nativeText->AcceptsReturn = multiline;
        nativeText->TextWrapping = multiline ? TextWrapping::Wrap : TextWrapping::NoWrap;
    }
}

void PrivateTextFieldWinUAP::SetNativeInputEnabled(bool enabled)
{
    nativeControl->IsEnabled = enabled;
}

void PrivateTextFieldWinUAP::SetNativeText(const WideString& text)
{
    Platform::String ^ platformText = ref new Platform::String(text.c_str());
    IsPassword() ? nativePassword->Password = platformText : nativeText->Text = platformText;
}

void PrivateTextFieldWinUAP::SetNativeMaxTextLength(int32 maxLength)
{
    // Native controls expect zero for unlimited input length
    int length = std::max(0, maxLength);
    IsPassword() ? nativePassword->MaxLength = length : nativeText->MaxLength = length;
}

void PrivateTextFieldWinUAP::SetNativeCaretPosition(int32 caretPosition)
{
    // NOTE: only TextBox supports setting caret position
    if (!IsPassword())
    {
        nativeText->SelectionStart = caretPosition;
    }
}

void PrivateTextFieldWinUAP::SetNativeFontSize(float32 fontSize)
{
    nativeControl->FontSize = fontSize;
}

void PrivateTextFieldWinUAP::SetNativeTextColor(const Color& textColor)
{
    Windows::UI::Color nativeColor;
    nativeColor.R = static_cast<unsigned char>(textColor.r * 255.0f);
    nativeColor.G = static_cast<unsigned char>(textColor.g * 255.0f);
    nativeColor.B = static_cast<unsigned char>(textColor.b * 255.0f);
    nativeColor.A = 255;
    nativeControl->Foreground = ref new SolidColorBrush(nativeColor);
}

void PrivateTextFieldWinUAP::SetNativeTextAlignment(int32 textAlignment, bool textRtlAlignment)
{
    // As far as I understood RTL text alignment affects only text alignment inside control rect
    // If RTL text alignment flag is set then invert text alignment from left to right and vice versa
    if (textRtlAlignment)
    {
        if (textAlignment & ALIGN_LEFT)
        {
            textAlignment &= ~ALIGN_LEFT;
            textAlignment |= ALIGN_RIGHT;
        }
        else if (textAlignment & ALIGN_RIGHT)
        {
            textAlignment &= ~ALIGN_RIGHT;
            textAlignment |= ALIGN_LEFT;
        }
    }

    TextAlignment nativeAlignment = TextAlignment::Left;
    if (textAlignment & ALIGN_LEFT)
        nativeAlignment = TextAlignment::Left;
    else if (textAlignment & ALIGN_HCENTER)
        nativeAlignment = TextAlignment::Center;
    else if (textAlignment & ALIGN_RIGHT)
        nativeAlignment = TextAlignment::Right;

    VerticalAlignment nativeVAlignment = VerticalAlignment::Top;
    if (textAlignment & ALIGN_TOP)
        nativeVAlignment = VerticalAlignment::Top;
    else if (textAlignment & ALIGN_VCENTER)
        nativeVAlignment = VerticalAlignment::Center;
    else if (textAlignment & ALIGN_BOTTOM)
        nativeVAlignment = VerticalAlignment::Bottom;

    nativeControl->VerticalAlignment = nativeVAlignment;
    // NOTE: only TextBox has TextAlignment property, not PasswordBox
    if (nativeText != nullptr)
        nativeText->TextAlignment = nativeAlignment;
}

void PrivateTextFieldWinUAP::SetNativeKeyboardType(int32 type)
{
    InputScopeNameValue nativeValue = InputScopeNameValue::Default;
    if (!IsPassword())
    {
        switch (type)
        {
        case UITextField::KEYBOARD_TYPE_URL:
            nativeValue = InputScopeNameValue::Url;
            break;
        case UITextField::KEYBOARD_TYPE_NUMBER_PAD:
        case UITextField::KEYBOARD_TYPE_DECIMAL_PAD:
        case UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION:
            nativeValue = InputScopeNameValue::Number;
            break;
        case UITextField::KEYBOARD_TYPE_PHONE_PAD:
            nativeValue = InputScopeNameValue::TelephoneNumber;
            break;
        case UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD:
            nativeValue = InputScopeNameValue::NameOrPhoneNumber;
            break;
        case UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS:
            nativeValue = InputScopeNameValue::EmailSmtpAddress;
            break;
        default:
            nativeValue = InputScopeNameValue::Default;
            break;
        }
    }
    else
    {
        // NOTE: PasswordBox supports only Password and NumericPin from InputScopeNameValue enum
        nativeValue = InputScopeNameValue::Password;
    }

    InputScope ^ inputScope = ref new InputScope();
    inputScope->Names->Append(ref new InputScopeName(nativeValue));
    IsPassword() ? nativePassword->InputScope = inputScope : nativeText->InputScope = inputScope;
}

void PrivateTextFieldWinUAP::SetNativeSpellChecking(bool enabled)
{
    // NOTE: only TextBox has IsSpellCheckEnabled property, not PasswordBox
    if (!IsPassword())
    {
        nativeText->IsSpellCheckEnabled = enabled;
    }
}

bool PrivateTextFieldWinUAP::HasFocus() const
{
    return FocusState::Unfocused != nativeControl->FocusState;
}

Platform::String ^ PrivateTextFieldWinUAP::GetNativeText() const
{
    return !IsPassword() ? nativeText->Text : nativePassword->Password;
}

int32 PrivateTextFieldWinUAP::GetNativeCaretPosition() const
{
    return !IsPassword() ? nativeText->SelectionStart : 0;
}

bool PrivateTextFieldWinUAP::IsPassword() const
{
    return nativePassword != nullptr;
}

bool PrivateTextFieldWinUAP::IsMultiline() const
{
    return nativeText != nullptr && true == nativeText->AcceptsReturn;
}

Rect PrivateTextFieldWinUAP::VirtualToWindow(const Rect& srcRect) const
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    // 1. map virtual to physical
    Rect rect = coordSystem->ConvertVirtualToPhysical(srcRect);
    rect += coordSystem->GetPhysicalDrawOffset();

    // 2. map physical to window
#if defined(__DAVAENGINE_COREV2__)
    const float32 scaleFactor = window->GetRenderSurfaceScaleX();
#else
    const float32 scaleFactor = core->GetScreenScaleFactor();
#endif
    rect.x /= scaleFactor;
    rect.y /= scaleFactor;
    rect.dx /= scaleFactor;
    rect.dy /= scaleFactor;
    return rect;
}

Rect PrivateTextFieldWinUAP::WindowToVirtual(const Rect& srcRect) const
{
    VirtualCoordinatesSystem* coordSystem = VirtualCoordinatesSystem::Instance();

    Rect rect = srcRect;
    // 1. map window to physical
#if defined(__DAVAENGINE_COREV2__)
    const float32 scaleFactor = window->GetRenderSurfaceScaleX();
#else
    const float32 scaleFactor = core->GetScreenScaleFactor();
#endif
    rect.x *= scaleFactor;
    rect.y *= scaleFactor;
    rect.dx *= scaleFactor;
    rect.dy *= scaleFactor;

    // 2. map physical to virtual
    rect = coordSystem->ConvertPhysicalToVirtual(rect);
    rect -= coordSystem->GetPhysicalDrawOffset();
    return rect;
}

void PrivateTextFieldWinUAP::RenderToTexture(bool moveOffScreenOnCompletion)
{
    auto self{ shared_from_this() };
    RenderTargetBitmap ^ renderTarget = ref new RenderTargetBitmap;

    auto renderTask = create_task(renderTarget->RenderAsync(nativeControlHolder)).then([this, self, renderTarget]() { return renderTarget->GetPixelsAsync(); }).then([this, self, renderTarget, moveOffScreenOnCompletion](IBuffer ^ renderBuffer) {
        int32 imageWidth = renderTarget->PixelWidth;
        int32 imageHeight = renderTarget->PixelHeight;
        size_t streamSize = static_cast<size_t>(renderBuffer->Length);
        DataReader^ reader = DataReader::FromBuffer(renderBuffer);

        size_t index = 0;
        Vector<uint8> buf(streamSize, 0);
        while (reader->UnconsumedBufferLength > 0)
        {
            buf[index] = reader->ReadByte();
            index += 1;
        }

        RefPtr<Sprite> sprite(CreateSpriteFromPreviewData(&buf[0], imageWidth, imageHeight));
#if defined(__DAVAENGINE_COREV2__)
        window->GetEngine()->RunAsyncOnMainThread([this, self, sprite, moveOffScreenOnCompletion]() {
#else
        core->RunOnMainThread([this, self, sprite, moveOffScreenOnCompletion]() {
#endif
            if (uiTextField != nullptr && sprite.Valid() && !curText.empty())
            {
                UIControl* curFocused = UIControlSystem::Instance()->GetFocusedControl();
                if (!uiTextField->IsEditing())
                {
                    // Do not set rendered texture if control has focus
                    uiTextField->SetSprite(sprite.Get(), 0);
                }
            }
            if (moveOffScreenOnCompletion)
            {
#if defined(__DAVAENGINE_COREV2__)
                window->RunAsyncOnUIThread([this, self]() {
#else
                core->RunOnUIThread([this, self]() {
#endif
                    waitRenderToTextureComplete = false;
                    if (!HasFocus())
                    { // Do not hide control if it has gained focus while rendering to texture
                        SetNativePositionAndSize(rectInWindowSpace, true);
                    }
                });
            }
        }); }).then([this, self](task<void> t) {
        try {
            t.get();
        }
        catch (Platform::COMException^ e) {
            HRESULT hr = e->HResult;
            Logger::Error("[TextField] RenderToTexture failed: 0x%08X", hr);
        } });
}

Sprite* PrivateTextFieldWinUAP::CreateSpriteFromPreviewData(uint8* imageData, int32 width, int32 height) const
{
    ScopedPtr<Image> imgSrc(Image::CreateFromData(width, height, FORMAT_RGBA8888, imageData));
    ImageConvert::SwapRedBlueChannels(imgSrc);
    return Sprite::CreateFromImage(imgSrc, true, false);
}

Platform::String ^ PrivateTextFieldWinUAP::xamlTextBoxStyles = LR"(
<ResourceDictionary
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    <Style x:Key="dava_custom_textbox" TargetType="TextBox">
        <Setter Property="MinWidth" Value="0" />
        <Setter Property="MinHeight" Value="0" />
        <Setter Property="Foreground" Value="White" />
        <Setter Property="Background" Value="Transparent" />
        <Setter Property="BorderBrush" Value="Transparent" />
        <Setter Property="SelectionHighlightColor" Value="{ThemeResource TextSelectionHighlightColorThemeBrush}" />
        <Setter Property="BorderThickness" Value="0" />
        <Setter Property="FontFamily" Value="{ThemeResource ContentControlThemeFontFamily}" />
        <Setter Property="FontSize" Value="{ThemeResource ControlContentThemeFontSize}" />
        <Setter Property="ScrollViewer.HorizontalScrollBarVisibility" Value="Hidden" />
        <Setter Property="ScrollViewer.VerticalScrollBarVisibility" Value="Hidden" />
        <Setter Property="ScrollViewer.IsDeferredScrollingEnabled" Value="False" />
        <Setter Property="Padding" Value="0"/>
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TextBox">
                    <Grid>
                        <ContentPresenter x:Name="HeaderContentPresenter"
                                      Grid.Row="0"
                                      Margin="0,4,0,4"
                                      Grid.ColumnSpan="2"
                                      Content="{TemplateBinding Header}"
                                      ContentTemplate="{TemplateBinding HeaderTemplate}"
                                      FontWeight="Semilight" />
                        <ScrollViewer x:Name="ContentElement"
                                    Grid.Row="1"
                                    HorizontalScrollMode="{TemplateBinding ScrollViewer.HorizontalScrollMode}"
                                    HorizontalScrollBarVisibility="{TemplateBinding ScrollViewer.HorizontalScrollBarVisibility}"
                                    VerticalScrollMode="{TemplateBinding ScrollViewer.VerticalScrollMode}"
                                    VerticalScrollBarVisibility="{TemplateBinding ScrollViewer.VerticalScrollBarVisibility}"
                                    IsHorizontalRailEnabled="{TemplateBinding ScrollViewer.IsHorizontalRailEnabled}"
                                    IsVerticalRailEnabled="{TemplateBinding ScrollViewer.IsVerticalRailEnabled}"
                                    IsDeferredScrollingEnabled="{TemplateBinding ScrollViewer.IsDeferredScrollingEnabled}"
                                    Margin="{TemplateBinding BorderThickness}"
                                    Padding="{TemplateBinding Padding}"
                                    IsTabStop="False"
                                    AutomationProperties.AccessibilityView="Raw"
                                    ZoomMode="Disabled" />
                    </Grid>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style x:Key="dava_custom_passwordbox" TargetType="PasswordBox">
        <Setter Property="MinWidth" Value="0" />
        <Setter Property="MinHeight" Value="0" />
        <Setter Property="Foreground" Value="White" />
        <Setter Property="Background" Value="Transparent" />
        <Setter Property="SelectionHighlightColor" Value="{ThemeResource TextSelectionHighlightColorThemeBrush}" />
        <Setter Property="BorderBrush" Value="Transparent" />
        <Setter Property="BorderThickness" Value="0" />
        <Setter Property="FontFamily" Value="{ThemeResource ContentControlThemeFontFamily}" />
        <Setter Property="FontSize" Value="{ThemeResource ControlContentThemeFontSize}" />
        <Setter Property="ScrollViewer.HorizontalScrollBarVisibility" Value="Hidden" />
        <Setter Property="ScrollViewer.VerticalScrollBarVisibility" Value="Hidden" />
        <Setter Property="Padding" Value="0"/>
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="PasswordBox">
                    <Grid>
                        <ContentPresenter x:Name="HeaderContentPresenter"
                                      Grid.Row="0"
                                      Margin="0,4,0,4"
                                      Grid.ColumnSpan="2"
                                      Content="{TemplateBinding Header}"
                                      ContentTemplate="{TemplateBinding HeaderTemplate}"
                                      FontWeight="Semilight" />
                        <ScrollViewer x:Name="ContentElement"
                            Grid.Row="1"
                                  HorizontalScrollMode="{TemplateBinding ScrollViewer.HorizontalScrollMode}"
                                  HorizontalScrollBarVisibility="{TemplateBinding ScrollViewer.HorizontalScrollBarVisibility}"
                                  VerticalScrollMode="{TemplateBinding ScrollViewer.VerticalScrollMode}"
                                  VerticalScrollBarVisibility="{TemplateBinding ScrollViewer.VerticalScrollBarVisibility}"
                                  IsHorizontalRailEnabled="{TemplateBinding ScrollViewer.IsHorizontalRailEnabled}"
                                  IsVerticalRailEnabled="{TemplateBinding ScrollViewer.IsVerticalRailEnabled}"
                                  Margin="{TemplateBinding BorderThickness}"
                                  Padding="{TemplateBinding Padding}"
                                  IsTabStop="False"
                                  ZoomMode="Disabled"
                                  AutomationProperties.AccessibilityView="Raw"/>
                    </Grid>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
</ResourceDictionary>
)";

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
