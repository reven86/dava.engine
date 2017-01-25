#include "Tests/FontTest.h"

#include "UI/Focus/UIFocusComponent.h"

using namespace DAVA;

class InputDelegate : public UITextFieldDelegate
{
public:
    InputDelegate(UIStaticText* text)
        : staticText(SafeRetain(text))
    {
    }

    ~InputDelegate()
    {
        SafeRelease(staticText);
    }

    void TextFieldOnTextChanged(UITextField* textField, const WideString& newText, const WideString& oldText) override
    {
        DVASSERT(staticText);
        staticText->SetText(newText);
    }

private:
    UIStaticText* staticText;
};

enum Tags
{
    INCREASE_SIZE_TAG = 1,
    DECREASE_SIZE_TAG
};

FontTest::FontTest(TestBed& app)
    : BaseScreen(app, "FontTest")
{
}

void FontTest::LoadResources()
{
    BaseScreen::LoadResources();

    ftFont = FTFont::Create("~res:/Fonts/korinna.ttf");
    dfFont = GraphicFont::Create("~res:/Fonts/korinna_df.fnt", "~res:/Fonts/korinna_df.tex");
    graphicFont = GraphicFont::Create("~res:/Fonts/korinna_graphic.fnt", "~res:/Fonts/korinna_graphic.tex");

    ScopedPtr<Font> uiFont(FTFont::Create("~res:/Fonts/korinna.ttf"));

    ScopedPtr<UIStaticText> label(new UIStaticText(Rect(10, 10, 200, 20)));
    label->SetFont(uiFont);
    label->SetTextColor(Color::White);
    label->SetText(L"Preview:");
    label->SetTextAlign(ALIGN_TOP | ALIGN_LEFT);
    AddControl(label);

    previewText = new UIStaticText(Rect(10, 40, 400, 200));
    previewText->SetFont(ftFont);
    previewText->SetTextColor(Color::White);
    previewText->SetDebugDraw(true);
    previewText->SetTextAlign(ALIGN_TOP | ALIGN_LEFT);
    previewText->SetMultiline(true);
    AddControl(previewText);

    label = new UIStaticText(Rect(10, 250, 200, 20));
    label->SetFont(uiFont);
    label->SetTextColor(Color::White);
    label->SetText(L"Input:");
    label->SetTextAlign(ALIGN_TOP | ALIGN_LEFT);
    AddControl(label);

    inputText = new UITextField(Rect(10, 280, 400, 200));
    inputText->GetOrCreateComponent<UIFocusComponent>();
    inputText->SetFont(uiFont);
    inputText->SetTextColor(Color::White);
    inputText->SetTextAlign(ALIGN_TOP | ALIGN_LEFT);
    inputText->SetDebugDraw(true);
    inputText->SetDelegate(inputDelegate = new InputDelegate(previewText));
    inputText->SetMultiline(true);
    AddControl(inputText);

    label = new UIStaticText(Rect(420, 10, 200, 20));
    label->SetFont(uiFont);
    label->SetTextColor(Color::White);
    label->SetText(L"Font type:");
    label->SetTextAlign(ALIGN_TOP | ALIGN_LEFT);
    AddControl(label);

    ScopedPtr<UIButton> button(new UIButton(Rect(420, 40, 100, 20)));
    button->SetStateFont(0xFF, uiFont);
    button->SetStateText(0xFF, L"FreeType");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FontTest::OnFontSelectClick));
    button->SetTag(Font::TYPE_FT);
    AddControl(button);

    button = new UIButton(Rect(530, 40, 100, 20));
    button->SetStateFont(0xFF, uiFont);
    button->SetStateText(0xFF, L"Distance");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FontTest::OnFontSelectClick));
    button->SetTag(Font::TYPE_DISTANCE);
    AddControl(button);

    button = new UIButton(Rect(640, 40, 100, 20));
    button->SetStateFont(0xFF, uiFont);
    button->SetStateText(0xFF, L"Graphic");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FontTest::OnFontSelectClick));
    button->SetTag(Font::TYPE_GRAPHIC);
    AddControl(button);

    label = new UIStaticText(Rect(420, 70, 200, 20));
    label->SetFont(uiFont);
    label->SetTextColor(Color::White);
    label->SetText(L"Font size:");
    label->SetTextAlign(ALIGN_TOP | ALIGN_LEFT);
    AddControl(label);

    button = new UIButton(Rect(420, 100, 100, 20));
    button->SetStateFont(0xFF, uiFont);
    button->SetStateText(0xFF, L"Increase");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FontTest::OnFontSizeClick));
    button->SetTag(INCREASE_SIZE_TAG);
    AddControl(button);

    button = new UIButton(Rect(530, 100, 100, 20));
    button->SetStateFont(0xFF, uiFont);
    button->SetStateText(0xFF, L"Decrease");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FontTest::OnFontSizeClick));
    button->SetTag(DECREASE_SIZE_TAG);
    AddControl(button);
}

void FontTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    SafeRelease(ftFont);
    SafeRelease(dfFont);
    SafeRelease(graphicFont);
    SafeDelete(inputDelegate);
    SafeRelease(inputText);
    SafeRelease(previewText);
}

void FontTest::OnFontSelectClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = DynamicTypeCheck<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case Font::TYPE_FT:
        previewText->SetFont(ftFont);
        inputText->SetFont(ftFont);
        break;
    case Font::TYPE_DISTANCE:
        previewText->SetFont(dfFont);
        inputText->SetFont(dfFont);
        break;
    case Font::TYPE_GRAPHIC:
        previewText->SetFont(graphicFont);
        inputText->SetFont(graphicFont);
        break;
    }
}

void FontTest::OnFontSizeClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = DynamicTypeCheck<UIButton*>(sender);
    Font* font = previewText->GetFont();
    switch (btn->GetTag())
    {
    case INCREASE_SIZE_TAG:
        font->SetSize(font->GetSize() + 1);
        break;
    case DECREASE_SIZE_TAG:
        font->SetSize(font->GetSize() - 1);
        break;
    }
}
