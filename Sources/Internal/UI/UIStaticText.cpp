#include "Utils/Utils.h"
#include "UI/UIStaticText.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "FileSystem/LocalizationSystem.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Animation/LinearAnimation.h"
#include "Utils/StringUtils.h"
#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/RenderHelper.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"
#include "Utils/UTF8Utils.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
const Vector2 UIStaticText::NO_REQUIRED_SIZE = Vector2(-1.f, -1.f);
const Vector2 UIStaticText::REQUIRED_CONTROL_SIZE = Vector2::Zero;
const Vector2 UIStaticText::REQUIRED_CONTROL_WIDTH = Vector2(0.f, -1.f);
const Vector2 UIStaticText::REQUIRED_CONTROL_HEIGHT = Vector2(-1.f, 0.f);

#if defined(LOCALIZATION_DEBUG)
const float32 UIStaticText::LOCALIZATION_RESERVED_PORTION = 0.6f;
const Color UIStaticText::HIGHLIGHT_COLORS[] = { DAVA::Color(1.0f, 0.0f, 0.0f, 0.4f),
                                                 DAVA::Color(0.0f, 0.0f, 1.0f, 0.4f),
                                                 DAVA::Color(1.0f, 1.0f, 0.0f, 0.4f),
                                                 DAVA::Color(1.0f, 1.0f, 1.0f, 0.4f),
                                                 DAVA::Color(1.0f, 0.0f, 1.0f, 0.4f),
                                                 DAVA::Color(0.0f, 1.0f, 0.0f, 0.4f) };
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(UIStaticText)
{
    ReflectionRegistrator<UIStaticText>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIStaticText* o) { o->Release(); })
    .Field("textColor", &UIStaticText::GetTextColor, &UIStaticText::SetTextColor)
    .Field("textcolorInheritType", &UIStaticText::GetTextColorInheritType, &UIStaticText::SetTextColorInheritType)[M::EnumT<UIControlBackground::eColorInheritType>()] // TODO: camel style
    .Field("textperPixelAccuracyType", &UIStaticText::GetTextPerPixelAccuracyType, &UIStaticText::SetTextPerPixelAccuracyType)[M::EnumT<UIControlBackground::ePerPixelAccuracyType>()] // TODO: camel style
    .Field("shadowoffset", &UIStaticText::GetShadowOffset, &UIStaticText::SetShadowOffset) // TODO: camel style
    .Field("shadowcolor", &UIStaticText::GetShadowColor, &UIStaticText::SetShadowColor) // TODO: camel style
    .Field("multiline", &UIStaticText::GetMultilineType, &UIStaticText::SetMultilineType)[M::EnumT<eMultiline>()]
    .Field("fitting", &UIStaticText::GetFittingOption, &UIStaticText::SetFittingOption)[M::FlagsT<TextBlock::eFitType>()]
    .Field("textalign", &UIStaticText::GetTextAlign, &UIStaticText::SetTextAlign)[M::FlagsT<eAlign>()] // TODO: camel style
    .Field("textUseRtlAlign", &UIStaticText::GetTextUseRtlAlign, &UIStaticText::SetTextUseRtlAlign)[M::EnumT<TextBlock::eUseRtlAlign>()]
    .Field("text", &UIStaticText::GetUtf8Text, &UIStaticText::SetUtf8TextWithoutRect)
    .Field("font", &UIStaticText::GetFontPresetName, &UIStaticText::SetFontByPresetName)
    .Field("forceBiDiSupport", &UIStaticText::IsForceBiDiSupportEnabled, &UIStaticText::SetForceBiDiSupportEnabled)
    .End();
}

UIStaticText::UIStaticText(const Rect& rect)
    : UIControl(rect)
    , shadowOffset(0, 0)
{
    SetInputEnabled(false, false);
    textBlock = TextBlock::Create(Vector2(rect.dx, rect.dy));

    textBg = new UIControlBackground();
    textBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    textBg->SetColorInheritType(UIControlBackground::COLOR_MULTIPLY_ON_PARENT);

    textBg->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);

    shadowBg = new UIControlBackground();
    shadowBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    shadowBg->SetColorInheritType(UIControlBackground::COLOR_MULTIPLY_ON_PARENT);
    shadowBg->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);

    SetTextColor(Color::White);
    SetShadowColor(Color::Black);
    SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
}

UIStaticText::~UIStaticText()
{
    SafeRelease(textBlock);
    SafeRelease(shadowBg);
    SafeRelease(textBg);
}

UIStaticText* UIStaticText::Clone()
{
    UIStaticText* t = new UIStaticText(GetRect());
    t->CopyDataFrom(this);
    return t;
}

void UIStaticText::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIStaticText* t = static_cast<UIStaticText*>(srcControl);

    SafeRelease(textBlock);
    textBlock = t->textBlock->Clone();

    shadowOffset = t->shadowOffset;

    SafeRelease(shadowBg);
    SafeRelease(textBg);
    shadowBg = t->shadowBg->Clone();
    textBg = t->textBg->Clone();
}

void UIStaticText::SetText(const WideString& _string, const Vector2& requestedTextRectSize /* = Vector2(0,0)*/)
{
    textBlock->SetRectSize(size);
    textBlock->SetText(_string, requestedTextRectSize);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}

void UIStaticText::SetUtf8Text(const String& utf8String, const Vector2& requestedTextRectSize /*= Vector2::Zero*/)
{
    SetText(UTF8Utils::EncodeToWideString(utf8String), requestedTextRectSize);
}

void UIStaticText::SetUtf8TextWithoutRect(const String& utf8String)
{
    SetUtf8Text(utf8String, Vector2::Zero);
}

String UIStaticText::GetUtf8Text() const
{
    return UTF8Utils::EncodeToUTF8(GetText());
}

void UIStaticText::SetFittingOption(int32 fittingType)
{
    textBlock->SetRectSize(size);
    textBlock->SetFittingOption(fittingType);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}

int32 UIStaticText::GetFittingOption() const
{
    return textBlock->GetFittingOption();
}

void UIStaticText::SetFont(Font* _font)
{
    if (textBlock->GetFont() != _font)
    {
        textBlock->SetRectSize(size);
        textBlock->SetFont(_font);
        if (textBlock->NeedCalculateCacheParams())
        {
            SetLayoutDirty();
            PrepareSprite();
        }
    }
}

void UIStaticText::SetTextColor(const Color& color)
{
    textBg->SetColor(color);
}

void UIStaticText::SetShadowOffset(const Vector2& offset)
{
    shadowOffset = offset;
}

void UIStaticText::SetShadowColor(const Color& color)
{
    shadowBg->SetColor(color);
}

void UIStaticText::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    textBlock->SetRectSize(size);
    textBlock->SetMultiline(_isMultilineEnabled, bySymbol);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}

bool UIStaticText::GetMultiline() const
{
    return textBlock->GetMultiline();
}

bool UIStaticText::GetMultilineBySymbol() const
{
    return textBlock->GetMultilineBySymbol();
}

void UIStaticText::SetTextAlign(int32 _align)
{
    textBlock->SetAlign(_align);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}

int32 UIStaticText::GetTextAlign() const
{
    return textBlock->GetAlign();
}

int32 UIStaticText::GetTextVisualAlign() const
{
    return textBlock->GetVisualAlign();
}

const WideString& UIStaticText::GetVisualText() const
{
    return textBlock->GetVisualText();
}

bool UIStaticText::GetTextIsRtl() const
{
    return textBlock->IsRtl();
}

void UIStaticText::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
    textBlock->SetUseRtlAlign(useRtlAlign);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}

TextBlock::eUseRtlAlign UIStaticText::GetTextUseRtlAlign() const
{
    return textBlock->GetUseRtlAlign();
}

void UIStaticText::SetTextUseRtlAlignFromInt(int32 value)
{
    SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(value));
}

int32 UIStaticText::GetTextUseRtlAlignAsInt() const
{
    return GetTextUseRtlAlign();
}

const Vector2& UIStaticText::GetTextSize()
{
    return textBlock->GetTextSize();
}

Vector2 UIStaticText::GetContentPreferredSize(const Vector2& constraints) const
{
    return textBlock->GetPreferredSizeForWidth(constraints.x);
}

bool UIStaticText::IsHeightDependsOnWidth() const
{
    return textBlock->GetMultiline();
}

const Color& UIStaticText::GetTextColor() const
{
    return textBg->GetColor();
}

const Color& UIStaticText::GetShadowColor() const
{
    return shadowBg->GetColor();
}

const Vector2& UIStaticText::GetShadowOffset() const
{
    return shadowOffset;
}

void UIStaticText::Draw(const UIGeometricData& geometricData)
{
    if (GetText().empty())
    {
        UIControl::Draw(geometricData);
        return;
    }
    Rect textBlockRect = CalculateTextBlockRect(geometricData);
    if (textBlock->GetFont() && textBlock->GetFont()->GetFontType() == Font::TYPE_DISTANCE)
    {
        // Correct rect and setup position and scale for distance fonts
        textBlockRect.dx *= geometricData.scale.dx;
        textBlockRect.dy *= geometricData.scale.dy;
        textBlock->SetScale(geometricData.scale);
        textBlock->SetAngle(geometricData.angle);
        textBlock->SetPivot(GetPivotPoint() * geometricData.scale);
    }
    textBlock->SetRectSize(textBlockRect.GetSize());
    textBlock->SetPosition(textBlockRect.GetPosition());
    textBlock->PreDraw();
    PrepareSprite();
    textBg->SetAlign(textBlock->GetVisualAlign());

    UIControl::Draw(geometricData);

    UIGeometricData textGeomData;
    textGeomData.position = textBlock->GetSpriteOffset();
    textGeomData.size = GetSize();
    textGeomData.AddGeometricData(geometricData);

    if (!FLOAT_EQUAL(shadowBg->GetDrawColor().a, 0.0f) && (!FLOAT_EQUAL(shadowOffset.dx, 0.0f) || !FLOAT_EQUAL(shadowOffset.dy, 0.0f)))
    {
        textBlock->Draw(shadowBg->GetDrawColor(), &shadowOffset);
        UIGeometricData shadowGeomData;
        shadowGeomData.position = shadowOffset;
        shadowGeomData.size = GetSize();
        shadowGeomData.AddGeometricData(textGeomData);

        shadowBg->SetAlign(textBg->GetAlign());
        shadowBg->Draw(shadowGeomData);
    }

    textBlock->Draw(textBg->GetDrawColor());
  
    
#if defined(LOCALIZATION_DEBUG)
    UIGeometricData elementGeomData;
    textBg->Draw(textGeomData);
    const Sprite::DrawState& lastDrawStae = textBg->GetLastDrawState();
    elementGeomData.position = lastDrawStae.position;
    elementGeomData.angle = lastDrawStae.angle;
    elementGeomData.scale = lastDrawStae.scale;
    elementGeomData.pivotPoint = lastDrawStae.pivotPoint;

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LINEBREAK_ERRORS) || Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_WARINGS))
    {
        RecalculateDebugColoring();
        DrawLocalizationDebug(geometricData);
    }
    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_ERRORS))
    {
        DrawLocalizationErrors(geometricData, elementGeomData);
    }
#else
    textBg->Draw(textGeomData);
#endif
}

void UIStaticText::SetParentColor(const Color& parentColor)
{
    UIControl::SetParentColor(parentColor);
    shadowBg->SetParentColor(parentColor);
    textBg->SetParentColor(parentColor);
}

const Vector<WideString>& UIStaticText::GetMultilineStrings() const
{
    return textBlock->GetMultilineStrings();
}

const WideString& UIStaticText::GetText() const
{
    return textBlock->GetText();
}

Animation* UIStaticText::TextColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
{
    LinearAnimation<Color>* animation = new LinearAnimation<Color>(this, &textBg->color, finalColor, time, interpolationFunc);
    animation->Start(track);
    return animation;
}

Animation* UIStaticText::ShadowColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 1*/)
{
    LinearAnimation<Color>* animation = new LinearAnimation<Color>(this, &shadowBg->color, finalColor, time, interpolationFunc);
    animation->Start(track);
    return animation;
}

const Vector<float32>& UIStaticText::GetStringSizes() const
{
    return textBlock->GetStringSizes();
}

void UIStaticText::SetForceBiDiSupportEnabled(bool value)
{
    textBlock->SetForceBiDiSupportEnabled(value);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}

void UIStaticText::PrepareSprite()
{
    if (textBlock->IsSpriteReady())
    {
        Sprite* sprite = textBlock->GetSprite();
        shadowBg->SetSprite(sprite, 0);
        textBg->SetSprite(sprite, 0);

        Texture* tex = sprite->GetTexture();
        if (tex && tex->GetFormat() == FORMAT_A8)
        {
            textBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL);
            shadowBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL);
        }
        else
        {
            textBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
            shadowBg->SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
        }
    }
    else
    {
        shadowBg->SetSprite(NULL, 0);
        textBg->SetSprite(NULL, 0);
    }
}

Rect UIStaticText::CalculateTextBlockRect(const UIGeometricData& geometricData) const
{
    Rect resultRect(geometricData.position, geometricData.size);
    return resultRect;
}

String UIStaticText::GetFontPresetName() const
{
    Font* font = GetFont();
    if (!font)
        return "";
    return FontManager::Instance()->GetFontName(font);
}

void UIStaticText::SetFontByPresetName(const String& presetName)
{
    Font* font = NULL;

    if (!presetName.empty())
    {
        font = FontManager::Instance()->GetFont(presetName);
    }

    SetFont(font);
}

int32 UIStaticText::GetTextColorInheritType() const
{
    return GetTextBackground()->GetColorInheritType();
}

void UIStaticText::SetTextColorInheritType(int32 type)
{
    GetTextBackground()->SetColorInheritType(static_cast<UIControlBackground::eColorInheritType>(type));
    GetShadowBackground()->SetColorInheritType(static_cast<UIControlBackground::eColorInheritType>(type));
}

int32 UIStaticText::GetTextPerPixelAccuracyType() const
{
    return GetTextBackground()->GetPerPixelAccuracyType();
}

void UIStaticText::SetTextPerPixelAccuracyType(int32 type)
{
    GetTextBackground()->SetPerPixelAccuracyType(static_cast<UIControlBackground::ePerPixelAccuracyType>(type));
    GetShadowBackground()->SetPerPixelAccuracyType(static_cast<UIControlBackground::ePerPixelAccuracyType>(type));
}

int32 UIStaticText::GetMultilineType() const
{
    if (GetMultiline())
        return GetMultilineBySymbol() ? MULTILINE_ENABLED_BY_SYMBOL : MULTILINE_ENABLED;
    else
        return MULTILINE_DISABLED;
}

void UIStaticText::SetMultilineType(int32 multilineType)
{
    switch (multilineType)
    {
    case MULTILINE_DISABLED:
        SetMultiline(false);
        break;

    case MULTILINE_ENABLED:
        SetMultiline(true, false);
        break;

    case MULTILINE_ENABLED_BY_SYMBOL:
        SetMultiline(true, true);
        break;

    default:
        DVASSERT(false);
        break;
    }
}

#if defined(LOCALIZATION_DEBUG)
void UIStaticText::DrawLocalizationErrors(const UIGeometricData& geometricData, const UIGeometricData& elementGeomData) const
{
    TextBlockSoftwareRender* rendereTextBlock = dynamic_cast<TextBlockSoftwareRender*>(textBlock->GetRenderer());
    if (rendereTextBlock != NULL)
    {
        DAVA::Matrix3 transform;
        elementGeomData.BuildTransformMatrix(transform);

        UIGeometricData textGeomData(elementGeomData);

        Vector3 x3 = Vector3(1.0f, 0.0f, 0.0f) * transform, y3 = Vector3(0.0f, 1.0f, 0.0f) * transform;
        Vector2 x(x3.x, x3.y), y(y3.x, y3.y);

        //reduce size by 1 pixel from each size for polygon to fit into control hence +1.0f and -1.0f
        //getTextOffsetTL and getTextOffsetBR are in physical coordinates but draw is still in virtual
        textGeomData.position += (x * UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualX(rendereTextBlock->getTextOffsetTL().x + 1.0f));
        textGeomData.position += (y * UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualY(rendereTextBlock->getTextOffsetTL().y + 1.0f));

        textGeomData.size = Vector2(0.0f, 0.0f);
        textGeomData.size.x += UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualX((rendereTextBlock->getTextOffsetBR().x - rendereTextBlock->getTextOffsetTL().x) - 1.0f);
        textGeomData.size.y += UIControlSystem::Instance()->vcs->ConvertPhysicalToVirtualY((rendereTextBlock->getTextOffsetBR().y - rendereTextBlock->getTextOffsetTL().y) - 1.0f);

        DAVA::Polygon2 textPolygon;
        textGeomData.GetPolygon(textPolygon);

        DAVA::Polygon2 controllPolygon;
        geometricData.GetPolygon(controllPolygon);

        //polygons will have te same transformation so just compare them
        if (!controllPolygon.IsPointInside(textPolygon.GetPoints()[0]) ||
            !controllPolygon.IsPointInside(textPolygon.GetPoints()[1]) ||
            !controllPolygon.IsPointInside(textPolygon.GetPoints()[2]) ||
            !controllPolygon.IsPointInside(textPolygon.GetPoints()[3]))
        {
            RenderSystem2D::Instance()->DrawPolygon(textPolygon, true, HIGHLIGHT_COLORS[MAGENTA]);
            RenderSystem2D::Instance()->FillPolygon(controllPolygon, HIGHLIGHT_COLORS[RED]);
        }
        if (textBlock->IsVisualTextCroped())
        {
            RenderSystem2D::Instance()->FillPolygon(textPolygon, HIGHLIGHT_COLORS[YELLOW]);
        }
    }
}
void UIStaticText::DrawLocalizationDebug(const UIGeometricData& textGeomData) const
{
    if (warningColor != NONE && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_WARINGS))
    {
        DAVA::Polygon2 polygon;
        textGeomData.GetPolygon(polygon);
        RenderSystem2D::Instance()->DrawPolygon(polygon, true, HIGHLIGHT_COLORS[warningColor]);
    }
    if (lineBreakError != NONE && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LINEBREAK_ERRORS))
    {
        DAVA::Polygon2 polygon;
        textGeomData.GetPolygon(polygon);
        RenderSystem2D::Instance()->FillPolygon(polygon, HIGHLIGHT_COLORS[lineBreakError]);
    }
    if (textBlock->GetFittingOption() != 0 && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DRAW_LOCALIZATION_WARINGS))
    {
        Color color = HIGHLIGHT_COLORS[WHITE];
        if (textBlock->GetFittingOptionUsed() != 0)
        {
            if (textBlock->GetFittingOptionUsed() & TextBlock::FITTING_REDUCE)
                color = HIGHLIGHT_COLORS[RED];
            if (textBlock->GetFittingOptionUsed() & TextBlock::FITTING_ENLARGE)
                color = HIGHLIGHT_COLORS[YELLOW];
            if (textBlock->GetFittingOptionUsed() & TextBlock::FITTING_POINTS)
                color = HIGHLIGHT_COLORS[BLUE];
        }
        DAVA::Polygon2 polygon;
        textGeomData.GetPolygon(polygon);
        DVASSERT(polygon.GetPointCount() == 4);
        RenderSystem2D::Instance()->DrawLine(polygon.GetPoints()[0], polygon.GetPoints()[2], color);
    }
}
void UIStaticText::RecalculateDebugColoring()
{
    warningColor = NONE;
    lineBreakError = NONE;
    if (textBlock->GetFont() == NULL)
        return;

    if (textBlock->GetMultiline())
    {
        const Vector<WideString>& strings = textBlock->GetMultilineStrings();
        const WideString& text = textBlock->GetText();
        float32 accumulatedHeight = 0.0f;
        float32 maxWidth = 0.0f;

        if (!text.empty())
        {
            WideString textNoSpaces = StringUtils::RemoveNonPrintable(text, 1);
            // StringUtils::IsWhitespace function has 2 overloads and compiler cannot deduce predicate parameter for std::remove_if
            // So help compiler to choose correct overload of StringUtils::IsWhitespace function using static_cast
            auto res = remove_if(textNoSpaces.begin(), textNoSpaces.end(), static_cast<bool (*)(WideString::value_type)>(&StringUtils::IsWhitespace));
            textNoSpaces.erase(res, textNoSpaces.end());

            WideString concatinatedStringsNoSpaces = L"";
            for (Vector<WideString>::const_iterator string = strings.begin();
                 string != strings.end(); string++)
            {
                WideString toFilter = *string;
                toFilter.erase(remove_if(toFilter.begin(), toFilter.end(), static_cast<bool (*)(WideString::value_type)>(&StringUtils::IsWhitespace)), toFilter.end());
                concatinatedStringsNoSpaces += toFilter;
            }

            if (concatinatedStringsNoSpaces != textNoSpaces)
            {
                lineBreakError = RED;
            }
        }
    }
}

#endif

DAVA::Font* UIStaticText::GetFont() const
{
    return textBlock->GetFont();
}

DAVA::float32 UIStaticText::GetFontSize() const
{
    return textBlock->GetRenderSize();
}

void UIStaticText::SetFontSize(float32 newSize)
{
    textBlock->SetRenderSize(newSize);
    if (textBlock->NeedCalculateCacheParams())
    {
        SetLayoutDirty();
    }
}
};
