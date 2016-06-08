#include "ControlsFactory.h"
#include "Qt/Settings/SettingsManager.h"

DAVA::Font* ControlsFactory::font12 = NULL;
DAVA::Font* ControlsFactory::font20 = NULL;

DAVA::UIButton* ControlsFactory::CreateButton(DAVA::Vector2 pos, const DAVA::WideString& buttonText, bool designers)
{
    DAVA::UIButton* btn = new DAVA::UIButton(DAVA::Rect(pos.x, pos.y, BUTTON_WIDTH, BUTTON_HEIGHT));
    CustomizeButton(btn, buttonText, designers);
    return btn;
}

DAVA::UIButton* ControlsFactory::CreateButton(const DAVA::Rect& rect, const DAVA::WideString& buttonText, bool designers)
{
    DAVA::UIButton* btn = new DAVA::UIButton(rect);
    CustomizeButton(btn, buttonText, designers);
    return btn;
}

void ControlsFactory::CustomizeButton(DAVA::UIButton* btn, const DAVA::WideString& buttonText, bool designers)
{
    DAVA::Font* font = GetFont12();

    btn->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(DAVA::UIControl::STATE_PRESSED_INSIDE, DAVA::UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(DAVA::UIControl::STATE_DISABLED, DAVA::UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(DAVA::UIControl::STATE_SELECTED, DAVA::UIControlBackground::DRAW_FILL);

    if (designers)
    {
        btn->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->SetColor(DAVA::Color(97.f / 255.f, 69.f / 255.f, 68.f / 255.f, 1.f));
    }
    else
    {
        btn->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->SetColor(DAVA::Color(0.0f, 0.0f, 0.0f, 0.5f));
    }
    btn->GetStateBackground(DAVA::UIControl::STATE_PRESSED_INSIDE)->SetColor(DAVA::Color(0.5f, 0.5f, 0.5f, 0.5f));
    btn->GetStateBackground(DAVA::UIControl::STATE_DISABLED)->SetColor(DAVA::Color(0.2f, 0.2f, 0.2f, 0.2f));
    btn->GetStateBackground(DAVA::UIControl::STATE_SELECTED)->SetColor(DAVA::Color(0.0f, 0.0f, 1.0f, 0.2f));

    btn->SetStateFont(DAVA::UIControl::STATE_PRESSED_INSIDE, font);
    btn->SetStateFont(DAVA::UIControl::STATE_DISABLED, font);
    btn->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    btn->SetStateFont(DAVA::UIControl::STATE_SELECTED, font);

    btn->SetStateText(DAVA::UIControl::STATE_PRESSED_INSIDE, buttonText);
    btn->SetStateText(DAVA::UIControl::STATE_DISABLED, buttonText);
    btn->SetStateText(DAVA::UIControl::STATE_NORMAL, buttonText);
    btn->SetStateText(DAVA::UIControl::STATE_SELECTED, buttonText);

    AddBorder(btn);
}

void ControlsFactory::CustomizeButtonExpandable(DAVA::UIButton* btn)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    DAVA::Texture::SetDefaultGPU(DAVA::GPU_ORIGIN);

    DAVA::UIControl* expandable = new DAVA::UIControl(DAVA::Rect(btn->GetSize().dx - btn->GetSize().dy, 0, btn->GetSize().dy, btn->GetSize().dy));
    expandable->SetInputEnabled(false);
    expandable->SetSprite("~res:/Gfx/UI/arrowdown", 0);
    btn->AddControl(expandable);

    SafeRelease(expandable);

    DAVA::Texture::SetDefaultGPU(static_cast<DAVA::eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}

DAVA::UIButton* ControlsFactory::CreateImageButton(const DAVA::Rect& rect, const DAVA::FilePath& imagePath)
{
    DAVA::UIButton* btn = new DAVA::UIButton(rect);
    CustomizeImageButton(btn, imagePath);
    return btn;
}

void ControlsFactory::CustomizeImageButton(DAVA::UIButton* btn, const DAVA::FilePath& imagePath)
{
    btn->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_SCALE_TO_RECT);
    btn->SetStateDrawType(DAVA::UIControl::STATE_PRESSED_INSIDE, DAVA::UIControlBackground::DRAW_SCALE_TO_RECT);
    btn->SetStateDrawType(DAVA::UIControl::STATE_DISABLED, DAVA::UIControlBackground::DRAW_SCALE_TO_RECT);
    btn->SetStateDrawType(DAVA::UIControl::STATE_SELECTED, DAVA::UIControlBackground::DRAW_SCALE_TO_RECT);

    btn->SetStateSprite(DAVA::UIControl::STATE_PRESSED_INSIDE, imagePath);
    btn->SetStateSprite(DAVA::UIControl::STATE_DISABLED, imagePath);
    btn->SetStateSprite(DAVA::UIControl::STATE_NORMAL, imagePath);
    btn->SetStateSprite(DAVA::UIControl::STATE_SELECTED, imagePath);

    btn->SetStateFrame(DAVA::UIControl::STATE_PRESSED_INSIDE, 0);
    btn->SetStateFrame(DAVA::UIControl::STATE_DISABLED, 0);
    btn->SetStateFrame(DAVA::UIControl::STATE_NORMAL, 0);
    btn->SetStateFrame(DAVA::UIControl::STATE_SELECTED, 0);
}

DAVA::UIButton* ControlsFactory::CreateCloseWindowButton(const DAVA::Rect& rect)
{
    DAVA::UIButton* btn = new DAVA::UIButton(rect);
    CustomizeCloseWindowButton(btn);
    return btn;
}

void ControlsFactory::CustomizeCloseWindowButton(DAVA::UIButton* btn)
{
    DAVA::Font* font = GetFont12();

    btn->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(DAVA::UIControl::STATE_PRESSED_INSIDE, DAVA::UIControlBackground::DRAW_FILL);

    btn->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->SetColor(DAVA::Color(1.0f, 0.0f, 0.0f, 0.5f));
    btn->GetStateBackground(DAVA::UIControl::STATE_PRESSED_INSIDE)->SetColor(DAVA::Color(0.8f, 0.0f, 0.0f, 0.5f));

    btn->SetStateFont(DAVA::UIControl::STATE_PRESSED_INSIDE, font);
    btn->SetStateFont(DAVA::UIControl::STATE_DISABLED, font);
    btn->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    btn->SetStateFont(DAVA::UIControl::STATE_SELECTED, font);

    btn->SetStateText(DAVA::UIControl::STATE_PRESSED_INSIDE, L"X");
    btn->SetStateText(DAVA::UIControl::STATE_DISABLED, L"X");
    btn->SetStateText(DAVA::UIControl::STATE_NORMAL, L"X");
    btn->SetStateText(DAVA::UIControl::STATE_SELECTED, L"X");

    AddBorder(btn);
}

DAVA::Font* ControlsFactory::GetFont12()
{
    if (!font12)
    {
        font12 = DAVA::FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
        font12->SetSize(12);
    }
    return font12;
}

DAVA::Font* ControlsFactory::GetFont20()
{
    if (!font20)
    {
        font20 = DAVA::FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
        font20->SetSize(20);
    }
    return font12;
}

DAVA::Color ControlsFactory::GetColorLight()
{
    return DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f);
}

DAVA::Color ControlsFactory::GetColorDark()
{
    return DAVA::Color(0.0f, 0.0f, 0.0f, 1.0f);
}

DAVA::Color ControlsFactory::GetColorError()
{
    return DAVA::Color(1.0f, 0.0f, 0.0f, 0.8f);
}

void ControlsFactory::CustomizeScreenBack(DAVA::UIControl* screen)
{
    screen->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    screen->GetBackground()->SetColor(DAVA::Color(0.7f, 0.7f, 0.7f, 1.0f));
}

DAVA::UIControl* ControlsFactory::CreateLine(const DAVA::Rect& rect)
{
    return CreateLine(rect, DAVA::Color(0.8f, 0.8f, 0.8f, 1.0f));
}

DAVA::UIControl* ControlsFactory::CreateLine(const DAVA::Rect& rect, DAVA::Color color)
{
    DAVA::UIControl* lineControl = new DAVA::UIControl(rect);
    lineControl->GetBackground()->color = color;
    lineControl->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    return lineControl;
}

void ControlsFactory::CusomizeBottomLevelControl(DAVA::UIControl* c)
{
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(DAVA::Color(0.5f, 0.5f, 0.5f, 1.0f));
}

void ControlsFactory::CusomizeTransparentControl(DAVA::UIControl* c, DAVA::float32 transparentLevel)
{
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(DAVA::Color(0.0f, 0.0f, 0.0f, transparentLevel));
}

void ControlsFactory::CusomizeTopLevelControl(DAVA::UIControl* c)
{
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(DAVA::Color(0.5f, 0.5f, 0.5f, 1.0f));
}

void ControlsFactory::CusomizeListControl(DAVA::UIControl* c)
{
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(DAVA::Color(0.92f, 0.92f, 0.92f, 1.0f));
}

DAVA::UIControl* ControlsFactory::CreatePanelControl(const DAVA::Rect& rect, bool addBorder)
{
    DAVA::UIControl* ctrl = new DAVA::UIControl(rect);
    CustomizePanelControl(ctrl, addBorder);
    return ctrl;
}

void ControlsFactory::CustomizePanelControl(DAVA::UIControl* c, bool addBorder)
{
    c->GetBackground()->color = DAVA::Color(0.4f, 0.4f, 0.4f, 1.0f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);

    if (addBorder)
    {
        AddBorder(c);
    }
}

void ControlsFactory::CustomizeExpandButton(DAVA::UIButton* btn)
{
    DAVA::Color color(0.1f, 0.5f, 0.05f, 1.0f);
    btn->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(DAVA::UIControl::STATE_PRESSED_INSIDE, DAVA::UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(DAVA::UIControl::STATE_HOVER, DAVA::UIControlBackground::DRAW_FILL);
    btn->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->color = color;
    btn->GetStateBackground(DAVA::UIControl::STATE_HOVER)->color = color + 0.1f;
    btn->GetStateBackground(DAVA::UIControl::STATE_PRESSED_INSIDE)->color = color + 0.3f;
}

void ControlsFactory::CustomizeListCell(DAVA::UIListCell* c, const DAVA::WideString& text, bool setLightFont)
{
    DAVA::Font* font = GetFont12();

    c->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    c->SetStateFont(DAVA::UIControl::STATE_SELECTED, font);

    c->SetStateText(DAVA::UIControl::STATE_NORMAL, text);
    c->SetStateText(DAVA::UIControl::STATE_SELECTED, text);

    c->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(DAVA::UIControl::STATE_SELECTED, DAVA::UIControlBackground::DRAW_FILL);
    c->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->color = DAVA::Color(1.0f, 1.0f, 1.0f, 0.0f);
    c->GetStateBackground(DAVA::UIControl::STATE_SELECTED)->color = DAVA::Color(1.0f, 0.8f, 0.8f, 1.0f);

    if (setLightFont)
    {
        c->GetStateTextControl(DAVA::UIControl::STATE_NORMAL)->SetTextColor(DAVA::Color::White);
        c->GetStateTextControl(DAVA::UIControl::STATE_SELECTED)->SetTextColor(DAVA::Color::White);
    }
    else
    {
        c->GetStateTextControl(DAVA::UIControl::STATE_NORMAL)->SetTextColor(DAVA::Color::Black);
        c->GetStateTextControl(DAVA::UIControl::STATE_SELECTED)->SetTextColor(DAVA::Color::Black);
    }
}

void ControlsFactory::CustomizeListCellAlternative(DAVA::UIListCell* c, const DAVA::WideString& text)
{
    DAVA::Font* font = GetFont12();

    c->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    c->SetStateFont(DAVA::UIControl::STATE_SELECTED, font);

    c->SetStateText(DAVA::UIControl::STATE_NORMAL, text);
    c->SetStateText(DAVA::UIControl::STATE_SELECTED, text);

    c->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(DAVA::UIControl::STATE_SELECTED, DAVA::UIControlBackground::DRAW_FILL);
    c->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->color = DAVA::Color(0.4f, 0.8f, 0.4f, 1.0f);
    c->GetStateBackground(DAVA::UIControl::STATE_SELECTED)->color = DAVA::Color(1.0f, 0.8f, 0.8f, 1.0f);
}

void ControlsFactory::CustomizeMenuPopupCell(DAVA::UIListCell* c, const DAVA::WideString& text)
{
    DAVA::Font* font = GetFont12();

    c->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    c->SetStateFont(DAVA::UIControl::STATE_SELECTED, font);

    c->SetStateText(DAVA::UIControl::STATE_NORMAL, text);
    c->SetStateText(DAVA::UIControl::STATE_SELECTED, text);

    c->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(DAVA::UIControl::STATE_SELECTED, DAVA::UIControlBackground::DRAW_FILL);

    c->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->color = DAVA::Color(0.1f, 0.1f, 0.1f, 0.6f);
    c->GetStateBackground(DAVA::UIControl::STATE_SELECTED)->color = DAVA::Color(0.6f, 0.6f, 0.6f, 0.6f);

    DAVA::Rect rect = c->GetRect();
    rect.y = rect.dy - 1;
    rect.dy = 1;
    DAVA::UIControl* line = CreateLine(rect);
    c->AddControl(line);
    SafeRelease(line);
}

void ControlsFactory::CustomizePropertyCell(DAVA::UIControl* c, bool isActivePart)
{
    if (isActivePart)
    {
        c->GetBackground()->color = DAVA::Color(0.5f, 0.5f, 0.5f, 0.5f);
    }
    else
    {
        c->GetBackground()->color = DAVA::Color(0.4f, 0.4f, 0.4f, 1.0f);
    }
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeEditablePropertyCell(DAVA::UIControl* c)
{
    c->GetBackground()->color = DAVA::Color(0.2f, 0.2f, 0.2f, 0.6f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeUneditablePropertyCell(DAVA::UIControl* c)
{
    c->GetBackground()->color = DAVA::Color(0.4f, 0.4f, 0.4f, 0.5f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizePropertySectionCell(DAVA::UIControl* c)
{
    c->GetBackground()->color = DAVA::Color(0.4f, 0.8f, 0.4f, 1.0f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizePropertySubsectionCell(DAVA::UIControl* c)
{
    c->GetBackground()->color = DAVA::Color(0.4f, 0.8f, 0.4f, 0.5f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizePropertyButtonCell(DAVA::UIListCell* c)
{
    c->SetStateDrawType(DAVA::UIControl::STATE_NORMAL, DAVA::UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(DAVA::UIControl::STATE_PRESSED_INSIDE, DAVA::UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(DAVA::UIControl::STATE_DISABLED, DAVA::UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(DAVA::UIControl::STATE_SELECTED, DAVA::UIControlBackground::DRAW_FILL);

    c->GetStateBackground(DAVA::UIControl::STATE_NORMAL)->SetColor(DAVA::Color(0.0f, 0.0f, 0.0f, 0.5f));
    c->GetStateBackground(DAVA::UIControl::STATE_PRESSED_INSIDE)->SetColor(DAVA::Color(0.5f, 0.5f, 0.5f, 0.5f));
    c->GetStateBackground(DAVA::UIControl::STATE_DISABLED)->SetColor(DAVA::Color(0.2f, 0.2f, 0.2f, 0.2f));
    c->GetStateBackground(DAVA::UIControl::STATE_SELECTED)->SetColor(DAVA::Color(0.0f, 0.0f, 1.0f, 0.2f));

    DAVA::Font* font = GetFont12();
    c->SetStateFont(DAVA::UIControl::STATE_PRESSED_INSIDE, font);
    c->SetStateFont(DAVA::UIControl::STATE_DISABLED, font);
    c->SetStateFont(DAVA::UIControl::STATE_NORMAL, font);
    c->SetStateFont(DAVA::UIControl::STATE_SELECTED, font);
}

void ControlsFactory::CustomizeDialogFreeSpace(DAVA::UIControl* c)
{
    c->GetBackground()->color = DAVA::Color(0.0f, 0.0f, 0.0f, 0.3f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeDialog(DAVA::UIControl* c)
{
    c->GetBackground()->color = DAVA::Color(0.0f, 0.0f, 0.0f, 0.5f);
    c->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
}

void ControlsFactory::SetScrollbar(DAVA::UIList* l)
{
    //Temporary fix for loading of UI Interface to avoid reloading of textures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    DAVA::Texture::SetDefaultGPU(DAVA::GPU_ORIGIN);

    DAVA::UIControl* c = l->FindByName("ScrollBar");
    if (c)
        return;

    DAVA::Rect fr = l->GetRect();

    DAVA::Sprite* scrollSpr = DAVA::Sprite::Create("~res:/Gfx/UI/scroll");

    DAVA::UIScrollBar* scrollBar = new DAVA::UIScrollBar(DAVA::Rect(fr.dx - scrollSpr->GetWidth(), 0, scrollSpr->GetWidth(), fr.dy),
                                                         DAVA::UIScrollBar::ORIENTATION_VERTICAL);
    scrollBar->SetName("ScrollBar");

    scrollBar->GetSlider()->SetSprite(scrollSpr, 0);
    scrollBar->GetSlider()->GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_STRETCH_VERTICAL);
    scrollBar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);

    scrollBar->SetDelegate(l);
    scrollBar->SetInputEnabled(false);
    l->AddControl(scrollBar);

    SafeRelease(scrollSpr);
    SafeRelease(scrollBar);

    DAVA::Texture::SetDefaultGPU(static_cast<DAVA::eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}

void ControlsFactory::RemoveScrollbar(DAVA::UIList* l)
{
    DAVA::UIControl* scrollBar = l->FindByName("ScrollBar");
    if (scrollBar)
    {
        scrollBar->GetParent()->RemoveControl(scrollBar);
    }
}

void ControlsFactory::AddBorder(DAVA::UIControl* c)
{
    DAVA::Rect fullRect = c->GetRect();

    DAVA::Color lineColor(1.f, 1.f, 1.f, 0.5f);

    DAVA::UIControl* leftLine = c->FindByName("LeftLine", false);
    if (!leftLine)
    {
        leftLine = ControlsFactory::CreateLine(DAVA::Rect(0, 1, 1, fullRect.dy - 2), lineColor);
        leftLine->SetName("LeftLine");
        c->AddControl(leftLine);
        SafeRelease(leftLine);
    }

    DAVA::UIControl* rightLine = c->FindByName("RightLine", false);
    if (!rightLine)
    {
        rightLine = ControlsFactory::CreateLine(DAVA::Rect(fullRect.dx - 1, 1, 1, fullRect.dy - 2), lineColor);
        rightLine->SetName("RightLine");
        c->AddControl(rightLine);
        SafeRelease(rightLine);
    }

    DAVA::UIControl* topLine = c->FindByName("TopLine", false);
    if (!topLine)
    {
        topLine = ControlsFactory::CreateLine(DAVA::Rect(0, 0, fullRect.dx, 1), lineColor);
        topLine->SetName("TopLine");
        c->AddControl(topLine);
        SafeRelease(topLine);
    }

    DAVA::UIControl* bottomtLine = c->FindByName("BottomLine", false);
    if (!bottomtLine)
    {
        bottomtLine = ControlsFactory::CreateLine(DAVA::Rect(0, fullRect.dy - 1, fullRect.dx, 1), lineColor);
        bottomtLine->SetName("BottomLine");
        c->AddControl(bottomtLine);
        SafeRelease(bottomtLine);
    }
}

void ControlsFactory::ReleaseFonts()
{
    SafeRelease(font12);
    SafeRelease(font20);
}
