#ifndef __DAVAENGINE_UI_STATIC_TEXT_H__
#define __DAVAENGINE_UI_STATIC_TEXT_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "Render/2D/TextBlock.h"

namespace DAVA
{
class UIStaticText : public UIControl
{
public:
    // Use NO_REQUIRED_SIZE to notify SetText that we don't want
    // to enable of any kind of static text fitting
    static const Vector2 NO_REQUIRED_SIZE;
    // Use REQUIRED_CONTROL_SIZE to notify SetText that we want
    // to enable of some kind of static text fitting with staticText control
    // size
    static const Vector2 REQUIRED_CONTROL_SIZE;
    // Use REQUIRED_CONTROL_WIDTH to notify SetText that we want
    // to enable limit of text width and not any limits of text
    // height
    static const Vector2 REQUIRED_CONTROL_WIDTH;
    // Use REQUIRED_CONTROL_HEIGHT to notify SetText that we want
    // to enable limit of text height and not any limits of text
    // width
    static const Vector2 REQUIRED_CONTROL_HEIGHT;

    enum eMultiline
    {
        MULTILINE_DISABLED = 0,
        MULTILINE_ENABLED,
        MULTILINE_ENABLED_BY_SYMBOL
    };
    
#if defined(LOCALIZATION_DEBUG)
    static const Color HIGHLIGHT_COLORS[];
    enum DebugHighliteColor
    {
        RED = 0,
        BLUE,
        YELLOW,
        WHITE,
        MAGENTA,
        GREEN,
        NONE
    };
    static const float32 LOCALIZATION_RESERVED_PORTION;
#endif
protected:
    virtual ~UIStaticText();

public:
    UIStaticText(const Rect& rect = Rect());

    virtual void Draw(const UIGeometricData& geometricData) override;
    virtual void SetParentColor(const Color& parentColor) override;
    //if requested size is 0 - text creates in the rect with size of the drawRect on draw phase
    //if requested size is >0 - text creates int the rect with the requested size
    //if requested size in <0 - rect creates for the all text size
    virtual void SetText(const WideString& string, const Vector2& requestedTextRectSize = Vector2::Zero);
    void SetTextWithoutRect(const WideString& text);

    void SetFont(Font* font);
    void SetFontSize(float32 newSize);
    void SetTextColor(const Color& color);

    void SetShadowColor(const Color& color);
    void SetShadowOffset(const Vector2& offset);

    void SetMultiline(bool isMultilineEnabled, bool bySymbol = false);
    bool GetMultiline() const;
    bool GetMultilineBySymbol() const;

    void SetMargins(const UIControlBackground::UIMargins* margins);
    const UIControlBackground::UIMargins* GetMargins() const;

    void SetFittingOption(int32 fittingType); //may be FITTING_ENLARGE, FITTING_REDUCE, FITTING_ENLARGE | FITTING_REDUCE
    int32 GetFittingOption() const;

    virtual void SetTextAlign(int32 _align);
    virtual int32 GetTextAlign() const;
    virtual int32 GetTextVisualAlign() const;
    virtual bool GetTextIsRtl() const;
    virtual void SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign);
    virtual TextBlock::eUseRtlAlign GetTextUseRtlAlign() const;

    virtual void SetTextUseRtlAlignFromInt(int32 value);
    virtual int32 GetTextUseRtlAlignAsInt() const;

    virtual const WideString& GetVisualText() const;
    const Vector2& GetTextSize();

    Vector2 GetContentPreferredSize(const Vector2& constraints) const override;
    bool IsHeightDependsOnWidth() const override;

    void PrepareSprite();

    const WideString& GetText() const;
    const Vector<WideString>& GetMultilineStrings() const;

    Font* GetFont() const;
    float32 GetFontSize() const;

    virtual UIStaticText* Clone() override;
    virtual void CopyDataFrom(UIControl* srcControl) override;
    TextBlock* GetTextBlock()
    {
        return textBlock;
    }
    const Color& GetTextColor() const;
    const Color& GetShadowColor() const;
    const Vector2& GetShadowOffset() const;

    inline UIControlBackground* GetTextBackground() const
    {
        return textBg;
    };
    inline UIControlBackground* GetShadowBackground() const
    {
        return shadowBg;
    };

    // Animation methods for Text Color and Shadow Color.
    virtual Animation* TextColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    virtual Animation* ShadowColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 1);

    const Vector<int32>& GetStringSizes() const;

    inline bool IsForceBiDiSupportEnabled() const
    {
        return textBlock->IsForceBiDiSupportEnabled();
    }
    void SetForceBiDiSupportEnabled(bool value);

protected:
    Rect CalculateTextBlockRect(const UIGeometricData& geometricData) const;
#if defined(LOCALIZATION_DEBUG)
    void DrawLocalizationDebug(const UIGeometricData& textGeomData) const;
    void DrawLocalizationErrors(const UIGeometricData& textGeomData, const UIGeometricData& elementGeomData) const;
    void RecalculateDebugColoring();
#endif
protected:
    TextBlock* textBlock;
    Vector2 shadowOffset;
    UIControlBackground* shadowBg;
    UIControlBackground* textBg;
#if defined(LOCALIZATION_DEBUG)
    DebugHighliteColor warningColor;
    DebugHighliteColor lineBreakError;
#endif

public:
    String GetFontPresetName() const;
    void SetFontByPresetName(const String& presetName);

    int32 GetTextColorInheritType() const;
    void SetTextColorInheritType(int32 type);

    int32 GetTextPerPixelAccuracyType() const;
    void SetTextPerPixelAccuracyType(int32 type);

    int32 GetMultilineType() const;
    void SetMultilineType(int32 multilineType);

    Vector4 GetMarginsAsVector4() const;
    void SetMarginsAsVector4(const Vector4& margins);

    INTROSPECTION_EXTEND(UIStaticText, UIControl,
                         PROPERTY("textColor", "Text Color", GetTextColor, SetTextColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textcolorInheritType", InspDesc("Text Color Inherit Type", GlobalEnumMap<UIControlBackground::eColorInheritType>::Instance()), GetTextColorInheritType, SetTextColorInheritType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textperPixelAccuracyType", InspDesc("Text Per Pixel Accuracy Type", GlobalEnumMap<UIControlBackground::ePerPixelAccuracyType>::Instance()), GetTextPerPixelAccuracyType, SetTextPerPixelAccuracyType, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("shadowoffset", "Shadow Offset", GetShadowOffset, SetShadowOffset, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("shadowcolor", "Shadow Color", GetShadowColor, SetShadowColor, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("multiline", InspDesc("Multi Line", GlobalEnumMap<eMultiline>::Instance()), GetMultilineType, SetMultilineType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("fitting", InspDesc("Fitting", GlobalEnumMap<TextBlock::eFitType>::Instance(), InspDesc::T_FLAGS), GetFittingOption, SetFittingOption, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textalign", InspDesc("Text Align", GlobalEnumMap<eAlign>::Instance(), InspDesc::T_FLAGS), GetTextAlign, SetTextAlign, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textUseRtlAlign", InspDesc("Use Rtl Align", GlobalEnumMap<TextBlock::eUseRtlAlign>::Instance(), InspDesc::T_ENUM), GetTextUseRtlAlignAsInt, SetTextUseRtlAlignFromInt, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textMargins", "Text margins", GetMarginsAsVector4, SetMarginsAsVector4, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("text", "Text", GetText, SetTextWithoutRect, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("font", "Font", GetFontPresetName, SetFontByPresetName, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("forceBiDiSupport", "Force BiDi support", IsForceBiDiSupportEnabled, SetForceBiDiSupportEnabled, I_SAVE | I_VIEW | I_EDIT));
};
};

#endif //__DAVAENGINE_UI_STATIC_TEXT_H__
