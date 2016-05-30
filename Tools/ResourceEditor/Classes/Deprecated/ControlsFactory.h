#ifndef __CONTROLS_FACTORY_H__
#define __CONTROLS_FACTORY_H__

#include "DAVAEngine.h"

class ControlsFactory
{
public:
    enum eGeneralControlSizes
    {
        BUTTON_HEIGHT = 20,
        BUTTON_WIDTH = 80,

        LEFT_PANEL_WIDTH = 200,
        RIGHT_PANEL_WIDTH = 200,
        OUTPUT_PANEL_HEIGHT = 70,
        PREVIEW_PANEL_HEIGHT = 200,

        OFFSET = 10,

        ERROR_MESSAGE_HEIGHT = 30,

        TEXTURE_PREVIEW_HEIGHT = 100,
        TEXTURE_PREVIEW_WIDTH = 200,

        TOOLS_HEIGHT = 40,
        TOOL_BUTTON_SIDE = 32,

        CELL_HEIGHT = 20,
    };

    enum eColorPickerSizes
    {
        COLOR_MAP_SIDE = 202,
        COLOR_SELECTOR_WIDTH = 20,
        COLOR_PREVIEW_SIDE = 80,
    };

public:
    static void ReleaseFonts();

    static void AddBorder(DAVA::UIControl* c);

    static DAVA::UIButton* CreateButton(DAVA::Vector2 pos, const DAVA::WideString& buttonText, bool designers = false);
    static DAVA::UIButton* CreateButton(const DAVA::Rect& rect, const DAVA::WideString& buttonText, bool designers = false);
    static void CustomizeButton(DAVA::UIButton* btn, const DAVA::WideString& buttonText, bool designers = false);

    static void CustomizeButtonExpandable(DAVA::UIButton* btn);

    static DAVA::UIButton* CreateImageButton(const DAVA::Rect& rect, const DAVA::FilePath& imagePath);
    static void CustomizeImageButton(DAVA::UIButton* btn, const DAVA::FilePath& imagePath);

    static DAVA::UIButton* CreateCloseWindowButton(const DAVA::Rect& rect);
    static void CustomizeCloseWindowButton(DAVA::UIButton* btn);

    static DAVA::Font* GetFont12();
    static DAVA::Font* GetFont20();

    static DAVA::Color GetColorLight();
    static DAVA::Color GetColorDark();
    static DAVA::Color GetColorError();

    //static void CustomizeFontLight(Font *font);
    //static void CustomizeFontDark(Font *font);
    //static void CustomizeFontError(Font *font);

    static void CustomizeScreenBack(DAVA::UIControl* screen);

    static DAVA::UIControl* CreateLine(const DAVA::Rect& rect);
    static DAVA::UIControl* CreateLine(const DAVA::Rect& rect, DAVA::Color color);

    static void CusomizeBottomLevelControl(DAVA::UIControl* c);

    static void CusomizeTopLevelControl(DAVA::UIControl* c);

    static void CusomizeListControl(DAVA::UIControl* c);

    static void CusomizeTransparentControl(DAVA::UIControl* c, DAVA::float32 transparentLevel);

    static DAVA::UIControl* CreatePanelControl(const DAVA::Rect& rect, bool addBorder = true);
    static void CustomizePanelControl(DAVA::UIControl* c, bool addBorder = true);

    static void CustomizeExpandButton(DAVA::UIButton* btn);

    static void CustomizeListCell(DAVA::UIListCell* c, const DAVA::WideString& text, bool setLightFont = true);
    static void CustomizeListCellAlternative(DAVA::UIListCell* c, const DAVA::WideString& text);

    static void CustomizeMenuPopupCell(DAVA::UIListCell* c, const DAVA::WideString& text);

    static void CustomizePropertyCell(DAVA::UIControl* c, bool isActivePart);
    static void CustomizeEditablePropertyCell(DAVA::UIControl* c);
    static void CustomizeUneditablePropertyCell(DAVA::UIControl* c);
    static void CustomizePropertySectionCell(DAVA::UIControl* c);
    static void CustomizePropertySubsectionCell(DAVA::UIControl* c);
    static void CustomizePropertyButtonCell(DAVA::UIListCell* c);

    static void CustomizeDialogFreeSpace(DAVA::UIControl* c);
    static void CustomizeDialog(DAVA::UIControl* c);

    static void SetScrollbar(DAVA::UIList* l);

    static void RemoveScrollbar(DAVA::UIList* l);

    static DAVA::Font* font12;
    static DAVA::Font* font20;
};



#endif // __CONTROLS_FACTORY_H__