#include "UI/UITextFieldAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)
#if !defined(__DAVAENGINE_COREV2__)

#include "Utils/UTF8Utils.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "UI/UIControlSystem.h"
#include "UI/Focus/FocusHelpers.h"
#include "UI/UIControlBackground.h"
#include "Utils/UTF8Utils.h"
#include "Concurrency/LockGuard.h"

using namespace DAVA;

extern void CreateTextField(DAVA::UITextField*);
extern void ReleaseTextField();
extern void OpenKeyboard();
extern void CloseKeyboard();

JniTextField::JniTextField(uint32_t id)
    : jniTextField("com/dava/framework/JNITextField")
{
    this->id = id;

    create = jniTextField.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("Create");
    destroy = jniTextField.GetStaticMethod<void, jint>("Destroy");
    updateRect = jniTextField.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("UpdateRect");
    setText = jniTextField.GetStaticMethod<void, jint, jstring>("SetText");
    setTextColor = jniTextField.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("SetTextColor");
    setFontSize = jniTextField.GetStaticMethod<void, jint, jfloat>("SetFontSize");
    setIsPassword = jniTextField.GetStaticMethod<void, jint, jboolean>("SetIsPassword");
    setTextAlign = jniTextField.GetStaticMethod<void, jint, jint>("SetTextAlign");
    setTextUseRtlAlign = jniTextField.GetStaticMethod<void, jint, jboolean>("SetTextUseRtlAlign");
    setInputEnabled = jniTextField.GetStaticMethod<void, jint, jboolean>("SetInputEnabled");
    setAutoCapitalizationType = jniTextField.GetStaticMethod<void, jint, jint>("SetAutoCapitalizationType");
    setAutoCorrectionType = jniTextField.GetStaticMethod<void, jint, jint>("SetAutoCorrectionType");
    setSpellCheckingType = jniTextField.GetStaticMethod<void, jint, jint>("SetSpellCheckingType");
    setKeyboardAppearanceType = jniTextField.GetStaticMethod<void, jint, jint>("SetKeyboardAppearanceType");
    setKeyboardType = jniTextField.GetStaticMethod<void, jint, jint>("SetKeyboardType");
    setReturnKeyType = jniTextField.GetStaticMethod<void, jint, jint>("SetReturnKeyType");
    setEnableReturnKeyAutomatically = jniTextField.GetStaticMethod<void, jint, jboolean>("SetEnableReturnKeyAutomatically");
    setVisible = jniTextField.GetStaticMethod<void, jint, jboolean>("SetVisible");
    setRenderToTexture = jniTextField.GetStaticMethod<void, jint, jboolean>("SetRenderToTexture");
    isRenderToTexture = jniTextField.GetStaticMethod<jboolean, jint>("IsRenderToTexture");
    openKeyboard = jniTextField.GetStaticMethod<void, jint>("OpenKeyboard");
    closeKeyboard = jniTextField.GetStaticMethod<void, jint>("CloseKeyboard");
    getCursorPos = jniTextField.GetStaticMethod<jint, jint>("GetCursorPos");
    setCursorPos = jniTextField.GetStaticMethod<void, jint, jint>("SetCursorPos");
    setMaxLength = jniTextField.GetStaticMethod<void, jint, jint>("SetMaxLength");
    setMultiline = jniTextField.GetStaticMethod<void, jint, jboolean>("SetMultiline");
}

void JniTextField::Create(Rect controlRect)
{
    Rect rect = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(controlRect);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

    create(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniTextField::Destroy()
{
    destroy(id);
}

void JniTextField::UpdateRect(const Rect& controlRect)
{
    Rect rect = UIControlSystem::Instance()->vcs->ConvertVirtualToInput(controlRect);

    rect.dx = std::max(0.0f, rect.dx);
    rect.dy = std::max(0.0f, rect.dy);

    updateRect(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniTextField::SetText(const char* text)
{
    JNIEnv* env = JNI::GetEnv();
    jstring jStrDefaultText = env->NewStringUTF(text);
    setText(id, jStrDefaultText);
    env->DeleteLocalRef(jStrDefaultText);
}

void JniTextField::SetTextColor(float r, float g, float b, float a)
{
    setTextColor(id, r, g, b, a);
}

void JniTextField::SetFontSize(float size)
{
    setFontSize(id, UIControlSystem::Instance()->vcs->ConvertVirtualToInputY(size));
}

void JniTextField::SetIsPassword(bool isPassword)
{
    setIsPassword(id, isPassword);
}

void JniTextField::SetTextAlign(int32_t align)
{
    setTextAlign(id, align);
}

void JniTextField::SetTextUseRtlAlign(bool useRtlAlign)
{
    setTextUseRtlAlign(id, useRtlAlign);
}

void JniTextField::SetInputEnabled(bool value)
{
    setInputEnabled(id, value);
}

void JniTextField::SetAutoCapitalizationType(int32_t value)
{
    setAutoCapitalizationType(id, value);
}

void JniTextField::SetAutoCorrectionType(int32_t value)
{
    setAutoCorrectionType(id, value);
}

void JniTextField::SetSpellCheckingType(int32_t value)
{
    setSpellCheckingType(id, value);
}

void JniTextField::SetKeyboardAppearanceType(int32_t value)
{
    setKeyboardAppearanceType(id, value);
}

void JniTextField::SetKeyboardType(int32_t value)
{
    setKeyboardType(id, value);
}

void JniTextField::SetReturnKeyType(int32_t value)
{
    setReturnKeyType(id, value);
}

void JniTextField::SetEnableReturnKeyAutomatically(bool value)
{
    setEnableReturnKeyAutomatically(id, value);
}

void JniTextField::SetVisible(bool isVisible)
{
    setVisible(id, isVisible);
}

void JniTextField::SetRenderToTexture(bool value)
{
    setRenderToTexture(id, value);
}

bool JniTextField::IsRenderToTexture() const
{
    return JNI_TRUE == isRenderToTexture(id);
}

void JniTextField::OpenKeyboard()
{
    openKeyboard(id);
}

void JniTextField::CloseKeyboard()
{
    closeKeyboard(id);
}

uint32 JniTextField::GetCursorPos()
{
    return getCursorPos(id);
}

void JniTextField::SetCursorPos(uint32 pos)
{
    setCursorPos(id, pos);
}

void JniTextField::SetMaxLength(int32_t value)
{
    setMaxLength(id, value);
}

void JniTextField::SetMultiline(bool value)
{
    jboolean isMulti = static_cast<jboolean>(value);
    setMultiline(id, isMulti);
}

uint32_t TextFieldPlatformImpl::sId = 0;
DAVA::UnorderedMap<uint32_t, TextFieldPlatformImpl*> TextFieldPlatformImpl::controls;
static DAVA::Mutex controlsSync;

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* textField)
{
    this->textField = textField;
    id = sId++;
    rect = textField->GetRect();
    jniTextField = std::make_shared<JniTextField>(id);
    jniTextField->Create(rect);

    DAVA::LockGuard<Mutex> guard(controlsSync);
    controls[id] = this;
}

TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    jniTextField->Destroy();
    textField = nullptr;

    DAVA::LockGuard<Mutex> guard(controlsSync);
    controls.erase(id);
}

void TextFieldPlatformImpl::OpenKeyboard()
{
    jniTextField->OpenKeyboard();
}

void TextFieldPlatformImpl::CloseKeyboard()
{
    jniTextField->CloseKeyboard();
}

void TextFieldPlatformImpl::GetText(WideString& string) const
{
    string = text;
}

void TextFieldPlatformImpl::SetText(const WideString& string)
{
    if (text.compare(string) != 0)
    {
        programmaticTextChange = true;
        text = TruncateText(string, textField->GetMaxLength());

        String utfText = UTF8Utils::EncodeToUTF8(text);
        jniTextField->SetText(utfText.c_str());
    }
}

void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
    if (rect != this->rect)
    {
        this->rect = rect;
        jniTextField->UpdateRect(rect);
    }
}

void TextFieldPlatformImpl::SetTextColor(const DAVA::Color& color)
{
    jniTextField->SetTextColor(color.r, color.g, color.b, color.a);
}

void TextFieldPlatformImpl::SetFontSize(float size)
{
    jniTextField->SetFontSize(size);
}

void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
{
    this->align = align;
    jniTextField->SetTextAlign(align);
}

DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return align;
}

void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
    this->useRtlAlign = useRtlAlign;
    jniTextField->SetTextUseRtlAlign(useRtlAlign);
}

bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return useRtlAlign;
}

void TextFieldPlatformImpl::SetVisible(bool isVisible)
{
    jniTextField->SetVisible(isVisible);
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
    jniTextField->SetIsPassword(isPassword);
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
    jniTextField->SetInputEnabled(value);
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
    jniTextField->SetRenderToTexture(value);
}

bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return jniTextField->IsRenderToTexture();
}

// Keyboard traits.
void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
    jniTextField->SetAutoCapitalizationType(value);
}

void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
{
    jniTextField->SetAutoCorrectionType(value);
}

void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
{
    jniTextField->SetSpellCheckingType(value);
}

void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
    jniTextField->SetKeyboardAppearanceType(value);
}

void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
{
    jniTextField->SetKeyboardType(value);
}

void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
{
    jniTextField->SetReturnKeyType(value);
}

void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
    jniTextField->SetEnableReturnKeyAutomatically(value);
}

uint32 TextFieldPlatformImpl::GetCursorPos()
{
    return jniTextField->GetCursorPos();
}

void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
    jniTextField->SetCursorPos(pos);
}

void TextFieldPlatformImpl::SetMaxLength(DAVA::int32 value)
{
    WideString truncated = TruncateText(text, value);
    if (truncated != text)
    {
        SetText(truncated);
    }

    return jniTextField->SetMaxLength(value);
}

void TextFieldPlatformImpl::SetMultiline(bool value)
{
    jniTextField->SetMultiline(value);
}

WideString TextFieldPlatformImpl::TruncateText(const WideString& text, int32 maxLength)
{
    WideString str = text;

    if (maxLength >= 0 && maxLength < str.length())
    {
        str.resize(maxLength);
    }

    return str;
}

bool TextFieldPlatformImpl::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, WideString& text)
{
    bool res = true;
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        res = delegate->TextFieldKeyPressed(textField, replacementLocation, replacementLength, text);

    if (res)
    {
        WideString curText = textField->GetText();
        if (curText.length() >= replacementLocation)
        {
            curText.replace(replacementLocation, replacementLength, text);
            this->text = curText;
        }
    }
    return res;
}

bool TextFieldPlatformImpl::TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, WideString& text)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return false;

    return control->TextFieldKeyPressed(replacementLocation, replacementLength, text);
}

void TextFieldPlatformImpl::TextFieldOnTextChanged(const WideString& newText, const WideString& oldText)
{
    UITextFieldDelegate::eReason type = UITextFieldDelegate::eReason::USER;
    if (programmaticTextChange)
    {
        programmaticTextChange = false;
        type = UITextFieldDelegate::eReason::CODE;
    }
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
    {
        delegate->TextFieldOnTextChanged(textField, newText, oldText, type);
    }
}

void TextFieldPlatformImpl::TextFieldOnTextChanged(uint32_t id, const WideString& newText, const WideString& oldText)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
    {
        return;
    }
    control->TextFieldOnTextChanged(newText, oldText);
}

void TextFieldPlatformImpl::TextFieldShouldReturn()
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->TextFieldShouldReturn(textField);
}

void TextFieldPlatformImpl::TextFieldShouldReturn(uint32_t id)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return;

    control->TextFieldShouldReturn();
}

TextFieldPlatformImpl* TextFieldPlatformImpl::GetUITextFieldAndroid(uint32_t id)
{
    DAVA::LockGuard<Mutex> guard(controlsSync);
    auto iter = controls.find(id);
    if (iter != controls.end())
        return iter->second;

    return nullptr;
}

void TextFieldPlatformImpl::TextFieldKeyboardShown(const Rect& rect)
{
    textField->OnKeyboardShown(rect);
}

void TextFieldPlatformImpl::TextFieldKeyboardShown(uint32_t id, const Rect& rect)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return;
    control->TextFieldKeyboardShown(rect);
}

void TextFieldPlatformImpl::TextFieldKeyboardHidden()
{
    textField->OnKeyboardHidden();
}

void TextFieldPlatformImpl::TextFieldKeyboardHidden(uint32_t id)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return;
    control->TextFieldKeyboardHidden();
}

void TextFieldPlatformImpl::TextFieldFocusChanged(bool hasFocus)
{
    if (textField)
    {
        if (hasFocus)
        {
            if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != textField && FocusHelpers::CanFocusControl(textField))
            {
                DAVA::UIControlSystem::Instance()->SetFocusedControl(textField);
            }
            textField->StartEdit();
        }
        else
        {
            if (DAVA::UIControlSystem::Instance()->GetFocusedControl() == textField)
            {
                textField->StopEdit();
            }
        }
    }
}

void TextFieldPlatformImpl::TextFieldFocusChanged(uint32_t id, bool hasFocus)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (nullptr != control)
    {
        control->TextFieldFocusChanged(hasFocus);
    }
}

void TextFieldPlatformImpl::TextFieldUpdateTexture(uint32_t id, int32* rawPixels,
                                                   int width, int height)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (nullptr != control)
    {
        UITextField& textField = *control->textField;

        if (nullptr != rawPixels)
        {
            // convert on the same memory
            uint32 pitch = width * 4;
            uint8* imageData = reinterpret_cast<uint8*>(rawPixels);
            ImageConvert::SwapRedBlueChannels(FORMAT_RGBA8888, imageData, width, height, pitch);

            Texture* tex = Texture::CreateFromData(FORMAT_RGBA8888, imageData, width, height, false);
            SCOPE_EXIT
            {
                SafeRelease(tex);
            };

            Rect rect = textField.GetRect();
            Sprite* spr = Sprite::CreateFromTexture(tex, 0, 0, width, height, rect.dx, rect.dy);
            SCOPE_EXIT
            {
                SafeRelease(spr);
            };

            UIControlBackground* bg = textField.GetOrCreateComponent<UIControlBackground>();
            bg->SetSprite(spr, 0);
        }
        else
        {
            // reset sprite to prevent render old sprite under android view
            textField.RemoveComponent(Type::Instance<UIControlBackground>());
        }
    }
}

void TextFieldPlatformImpl::SystemDraw(const UIGeometricData& geometricData)
{
}

#endif // !__DAVAENGINE_COREV2__
#endif //__DAVAENGINE_ANDROID__
