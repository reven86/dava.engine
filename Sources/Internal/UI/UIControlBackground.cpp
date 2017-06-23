#include "UI/UIControlBackground.h"
#include "Debug/DVAssert.h"
#include "UI/UIControl.h"
#include "Core/Core.h"
#include "Render/RenderHelper.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/RenderHelper.h"
#include "Render/Renderer.h"
#include "Utils/StringFormat.h"
#include "Reflection/ReflectionRegistrator.h"

#include <limits>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIControlBackground)
{
    ReflectionRegistrator<UIControlBackground>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIControlBackground* o) { o->Release(); })
    .Field("drawType", &UIControlBackground::GetDrawType, &UIControlBackground::SetDrawType)[M::EnumT<eDrawType>()]
    .Field<FilePath (UIControlBackground::*)() const, void (UIControlBackground::*)(const FilePath&)>("sprite", &UIControlBackground::GetBgSpritePath, &UIControlBackground::SetSprite)
    .Field<int32 (UIControlBackground::*)() const, void (UIControlBackground::*)(int32)>("frame", &UIControlBackground::GetFrame, &UIControlBackground::SetFrame)
    .Field("mask", &UIControlBackground::GetMaskSpritePath, &UIControlBackground::SetMaskSpriteFromPath)
    .Field("detail", &UIControlBackground::GetDetailSpritePath, &UIControlBackground::SetDetailSpriteFromPath)
    .Field("gradient", &UIControlBackground::GetGradientSpritePath, &UIControlBackground::SetGradientSpriteFromPath)
    .Field("contour", &UIControlBackground::GetContourSpritePath, &UIControlBackground::SetContourSpriteFromPath)
    .Field("spriteModification", &UIControlBackground::GetModification, &UIControlBackground::SetModification)[M::FlagsT<eSpriteModification>()]
    .Field("gradientMode", &UIControlBackground::GetGradientBlendMode, &UIControlBackground::SetGradientBlendMode)[M::EnumT<eGradientBlendMode>()]
    .Field("color", &UIControlBackground::GetColor, &UIControlBackground::SetColor)
    .Field("colorInherit", &UIControlBackground::GetColorInheritType, &UIControlBackground::SetColorInheritType)[M::EnumT<eColorInheritType>()]
    .Field("perPixelAccuracy", &UIControlBackground::GetPerPixelAccuracyType, &UIControlBackground::SetPerPixelAccuracyType)[M::EnumT<ePerPixelAccuracyType>()]
    .Field("align", &UIControlBackground::GetAlign, &UIControlBackground::SetAlign)[M::FlagsT<eAlign>()]
    .Field("leftRightStretchCap", &UIControlBackground::GetLeftRightStretchCap, &UIControlBackground::SetLeftRightStretchCap)
    .Field("topBottomStretchCap", &UIControlBackground::GetTopBottomStretchCap, &UIControlBackground::SetTopBottomStretchCap)
    .End();
}

UIControlBackground::UIControlBackground()
    : color(Color::White)
    , lastDrawPos(0, 0)
    , drawColor(Color::White)
    , material(SafeRetain(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL))
{
}

UIControlBackground::UIControlBackground(const UIControlBackground& src)
    : UIComponent(src)
    , spr(src.spr)
    , align(src.align)
    , type(src.type)
    , spriteModification(src.spriteModification)
    , leftStretchCap(src.leftStretchCap)
    , topStretchCap(src.topStretchCap)
    , colorInheritType(src.colorInheritType)
    , frame(src.frame)
    , lastDrawPos(0, 0)
    , perPixelAccuracyType(src.perPixelAccuracyType)
    , mask(src.mask)
    , detail(src.detail)
    , gradient(src.gradient)
    , contour(src.contour)
    , gradientMode(src.gradientMode)
    , color(src.color)
    , drawColor(src.drawColor)
    , material(SafeRetain(src.material))
{
}

UIControlBackground* UIControlBackground::Clone() const
{
    return new UIControlBackground(*this);
}

UIControlBackground::~UIControlBackground()
{
    spr = nullptr;
    SafeRelease(material);
    ReleaseDrawData();
}

Sprite* UIControlBackground::GetSprite() const
{
    return spr.Get();
}
int32 UIControlBackground::GetFrame() const
{
    return frame;
}
int32 UIControlBackground::GetAlign() const
{
    return align;
}

int32 UIControlBackground::GetModification() const
{
    return spriteModification;
}

UIControlBackground::eColorInheritType UIControlBackground::GetColorInheritType() const
{
    return colorInheritType;
}

UIControlBackground::eDrawType UIControlBackground::GetDrawType() const
{
    return type;
}

void UIControlBackground::SetSprite(const FilePath& path, int32 drawFrame)
{
    SetSprite(path);
    SetFrame(drawFrame);
}

void UIControlBackground::SetSprite(Sprite* drawSprite, int32 drawFrame)
{
    SetSprite(drawSprite);
    SetFrame(drawFrame);
}

void UIControlBackground::SetSprite(Sprite* drawSprite)
{
    spr = drawSprite;

    if (GetControl()) //workaround for standalone backgrounds
    {
        GetControl()->SetLayoutDirty();
    }
}

void UIControlBackground::SetSprite(const FilePath& path)
{
    RefPtr<Sprite> tempSpr;
    if (!path.IsEmpty())
    {
        tempSpr.Set(Sprite::Create(path));
    }
    SetSprite(tempSpr.Get());
}

void UIControlBackground::SetFrame(int32 drawFrame)
{
    frame = drawFrame;
}

void UIControlBackground::SetFrame(const FastName& frameName)
{
    DVASSERT(spr.Get());
    int32 frameInd = spr->GetFrameByName(frameName);
    if (frameInd != Sprite::INVALID_FRAME_INDEX)
    {
        SetFrame(frameInd);
    }
}

void UIControlBackground::SetAlign(int32 drawAlign)
{
    align = drawAlign;
}

void UIControlBackground::SetDrawType(UIControlBackground::eDrawType drawType)
{
    if (type != drawType)
    {
        ReleaseDrawData();
    }

    type = drawType;

    if (GetControl()) //workaround for standalone backgrounds
    {
        GetControl()->SetLayoutDirty();
    }
}

void UIControlBackground::SetModification(int32 modification)
{
    spriteModification = modification;
}

void UIControlBackground::SetColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
    DVASSERT(inheritType >= 0 && inheritType < COLOR_INHERIT_TYPES_COUNT);
    colorInheritType = inheritType;
}

void UIControlBackground::SetPerPixelAccuracyType(ePerPixelAccuracyType accuracyType)
{
    perPixelAccuracyType = accuracyType;
}

UIControlBackground::ePerPixelAccuracyType UIControlBackground::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}

const Color& UIControlBackground::GetDrawColor() const
{
    return drawColor;
}

void UIControlBackground::SetDrawColor(const Color& c)
{
    drawColor = c;
}

void UIControlBackground::SetParentColor(const Color& parentColor)
{
    switch (colorInheritType)
    {
    case COLOR_MULTIPLY_ON_PARENT:
    {
        drawColor.r = color.r * parentColor.r;
        drawColor.g = color.g * parentColor.g;
        drawColor.b = color.b * parentColor.b;
        drawColor.a = color.a * parentColor.a;
    }
    break;
    case COLOR_ADD_TO_PARENT:
    {
        drawColor.r = Min(color.r + parentColor.r, 1.0f);
        drawColor.g = Min(color.g + parentColor.g, 1.0f);
        drawColor.b = Min(color.b + parentColor.b, 1.0f);
        drawColor.a = Min(color.a + parentColor.a, 1.0f);
    }
    break;
    case COLOR_REPLACE_TO_PARENT:
    {
        drawColor = parentColor;
    }
    break;
    case COLOR_IGNORE_PARENT:
    {
        drawColor = color;
    }
    break;
    case COLOR_MULTIPLY_ALPHA_ONLY:
    {
        drawColor = color;
        drawColor.a = color.a * parentColor.a;
    }
    break;
    case COLOR_REPLACE_ALPHA_ONLY:
    {
        drawColor = color;
        drawColor.a = parentColor.a;
    }
    break;
    default:
        DVASSERT(false, Format("Unknown colorInheritType: %d", static_cast<int32>(colorInheritType)).c_str());
        break;
    }
}

void UIControlBackground::Draw(const UIGeometricData& parentGeometricData)
{
    UIGeometricData geometricData;
    geometricData.size = parentGeometricData.size;

    geometricData.AddGeometricData(parentGeometricData);
    Rect drawRect = geometricData.GetUnrotatedRect();

    Sprite::DrawState drawState;

    drawState.SetMaterial(material);
    if (spr != nullptr)
    {
        //drawState.SetShader(shader);
        drawState.frame = Clamp(frame, 0, (spr->GetFrameCount() - 1));
        if (spriteModification)
        {
            drawState.flags = spriteModification;
        }
        //		spr->Reset();
        //		spr->SetFrame(frame);
        //		spr->SetModification(spriteModification);
    }
    switch (type)
    {
    case DRAW_ALIGNED:
    {
        if (spr == nullptr)
            break;
        if (align & ALIGN_LEFT)
        {
            drawState.position.x = drawRect.x;
        }
        else if (align & ALIGN_RIGHT)
        {
            drawState.position.x = drawRect.x + drawRect.dx - spr->GetWidth() * geometricData.scale.x;
        }
        else
        {
            drawState.position.x = drawRect.x + ((drawRect.dx - spr->GetWidth() * geometricData.scale.x) * 0.5f);
        }
        if (align & ALIGN_TOP)
        {
            drawState.position.y = drawRect.y;
        }
        else if (align & ALIGN_BOTTOM)
        {
            drawState.position.y = drawRect.y + drawRect.dy - spr->GetHeight() * geometricData.scale.y;
        }
        else
        {
            drawState.position.y = drawRect.y + ((drawRect.dy - spr->GetHeight() * geometricData.scale.y + spr->GetDefaultPivotPoint().y * geometricData.scale.y) * 0.5f);
        }
        if (geometricData.angle != 0)
        {
            float tmpX = drawState.position.x;
            drawState.position.x = (tmpX - geometricData.position.x) * geometricData.cosA + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x;
            drawState.position.y = (tmpX - geometricData.position.x) * geometricData.sinA + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y;
            //				spr->SetAngle(geometricData.angle);
            drawState.angle = geometricData.angle;
        }
        //			spr->SetPosition(x, y);
        drawState.scale = geometricData.scale;
        drawState.pivotPoint = spr->GetDefaultPivotPoint();
        //			spr->SetScale(geometricData.scale);
        //if (drawState.scale.x == 1.0 && drawState.scale.y == 1.0)
        {
            switch (perPixelAccuracyType)
            {
            case PER_PIXEL_ACCURACY_ENABLED:
                if (lastDrawPos == drawState.position)
                {
                    drawState.usePerPixelAccuracy = true;
                }
                break;
            case PER_PIXEL_ACCURACY_FORCED:
                drawState.usePerPixelAccuracy = true;
                break;
            default:
                break;
            }
        }

        lastDrawPos = drawState.position;
        RenderSystem2D::Instance()->Draw(spr.Get(), &drawState, drawColor);
    }
    break;

    case DRAW_SCALE_TO_RECT:
    {
        if (spr == nullptr)
            break;

        drawState.position = geometricData.position;
        drawState.flags = spriteModification;
        drawState.scale.x = drawRect.dx / spr->GetSize().dx;
        drawState.scale.y = drawRect.dy / spr->GetSize().dy;
        drawState.pivotPoint.x = geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx);
        drawState.pivotPoint.y = geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy);
        drawState.angle = geometricData.angle;
        {
            switch (perPixelAccuracyType)
            {
            case PER_PIXEL_ACCURACY_ENABLED:
                if (lastDrawPos == drawState.position)
                {
                    drawState.usePerPixelAccuracy = true;
                }
                break;
            case PER_PIXEL_ACCURACY_FORCED:
                drawState.usePerPixelAccuracy = true;
                break;
            default:
                break;
            }
        }

        lastDrawPos = drawState.position;

        //			spr->SetPosition(geometricData.position);
        //			spr->SetScale(drawRect.dx / spr->GetSize().dx, drawRect.dy / spr->GetSize().dy);
        //			spr->SetPivotPoint(geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx), geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy));
        //			spr->SetAngle(geometricData.angle);

        RenderSystem2D::Instance()->Draw(spr.Get(), &drawState, drawColor);
    }
    break;

    case DRAW_SCALE_PROPORTIONAL:
    case DRAW_SCALE_PROPORTIONAL_ONE:
    {
        if (spr == nullptr)
            break;
        float32 w, h;
        w = drawRect.dx / (spr->GetWidth() * geometricData.scale.x);
        h = drawRect.dy / (spr->GetHeight() * geometricData.scale.y);
        float ph = spr->GetDefaultPivotPoint().y;

        if (w < h)
        {
            if (type == DRAW_SCALE_PROPORTIONAL_ONE)
            {
                w = spr->GetWidth() * h * geometricData.scale.y;
                ph *= h;
                h = drawRect.dy;
            }
            else
            {
                h = spr->GetHeight() * w * geometricData.scale.x;
                ph *= w;
                w = drawRect.dx;
            }
        }
        else
        {
            if (type == DRAW_SCALE_PROPORTIONAL_ONE)
            {
                h = spr->GetHeight() * w * geometricData.scale.x;
                ph *= w;
                w = drawRect.dx;
            }
            else
            {
                w = spr->GetWidth() * h * geometricData.scale.y;
                ph *= h;
                h = drawRect.dy;
            }
        }

        if (align & ALIGN_LEFT)
        {
            drawState.position.x = drawRect.x;
        }
        else if (align & ALIGN_RIGHT)
        {
            drawState.position.x = (drawRect.x + drawRect.dx - w);
        }
        else
        {
            drawState.position.x = drawRect.x + static_cast<int32>((drawRect.dx - w) * 0.5f);
        }
        if (align & ALIGN_TOP)
        {
            drawState.position.y = drawRect.y;
        }
        else if (align & ALIGN_BOTTOM)
        {
            drawState.position.y = (drawRect.y + drawRect.dy - h);
        }
        else
        {
            drawState.position.y = (drawRect.y) + static_cast<int32>((drawRect.dy - h + ph) * 0.5f);
        }
        drawState.scale.x = w / spr->GetWidth();
        drawState.scale.y = h / spr->GetHeight();
        //			spr->SetScaleSize(w, h);
        if (geometricData.angle != 0)
        {
            float32 tmpX = drawState.position.x;
            drawState.position.x = ((tmpX - geometricData.position.x) * geometricData.cosA + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x);
            drawState.position.y = ((tmpX - geometricData.position.x) * geometricData.sinA + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y);
            drawState.angle = geometricData.angle;
            //				spr->SetAngle(geometricData.angle);
        }
        //			spr->SetPosition((float32)x, (float32)y);
        {
            switch (perPixelAccuracyType)
            {
            case PER_PIXEL_ACCURACY_ENABLED:
                if (lastDrawPos == drawState.position)
                {
                    drawState.usePerPixelAccuracy = true;
                }
                break;
            case PER_PIXEL_ACCURACY_FORCED:
                drawState.usePerPixelAccuracy = true;
                break;
            default:
                break;
            }
        }

        lastDrawPos = drawState.position;

        RenderSystem2D::Instance()->Draw(spr.Get(), &drawState, drawColor);
    }
    break;

    case DRAW_FILL:
        //RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
        if (geometricData.angle != 0.0f)
        {
            Polygon2 poly;
            geometricData.GetPolygon(poly);
            RenderSystem2D::Instance()->FillPolygon(poly, drawColor);
        }
        else
        {
            RenderSystem2D::Instance()->FillRect(geometricData.GetUnrotatedRect(), drawColor);
        }
        break;

    case DRAW_STRETCH_BOTH:
    case DRAW_STRETCH_HORIZONTAL:
    case DRAW_STRETCH_VERTICAL:
        drawState.usePerPixelAccuracy = (perPixelAccuracyType == PER_PIXEL_ACCURACY_FORCED) || ((perPixelAccuracyType == PER_PIXEL_ACCURACY_ENABLED) && (lastDrawPos == geometricData.position));
        lastDrawPos = geometricData.position;
        RenderSystem2D::Instance()->DrawStretched(spr.Get(), &drawState, Vector2(leftStretchCap, topStretchCap), type, geometricData, &stretchData, drawColor);
        break;

    case DRAW_TILED:
        RenderSystem2D::Instance()->DrawTiled(spr.Get(), &drawState, Vector2(leftStretchCap, topStretchCap), geometricData, &tiledData, drawColor);
        break;
    case DRAW_TILED_MULTILAYER:
        drawState.SetMaterial(RenderSystem2D::DEFAULT_COMPOSIT_MATERIAL[gradientMode]);
        drawState.usePerPixelAccuracy = (perPixelAccuracyType == PER_PIXEL_ACCURACY_FORCED) || ((perPixelAccuracyType == PER_PIXEL_ACCURACY_ENABLED) && (lastDrawPos == geometricData.position));
        lastDrawPos = geometricData.position;
        RenderSystem2D::Instance()->DrawTiledMultylayer(mask.Get(), detail.Get(), gradient.Get(), contour.Get(), &drawState, Vector2(leftStretchCap, topStretchCap), geometricData, &tiledMultulayerData, drawColor);
        break;
    default:
        break;
    }
#if defined(LOCALIZATION_DEBUG)
    lastDrawState = drawState;
#endif
}

#if defined(LOCALIZATION_DEBUG)
const Sprite::DrawState& UIControlBackground::GetLastDrawState() const
{
    return lastDrawState;
}
#endif

void UIControlBackground::ReleaseDrawData()
{
    SafeDelete(tiledData);
    SafeDelete(stretchData);
    SafeDelete(tiledMultulayerData);
}

void UIControlBackground::SetLeftRightStretchCap(float32 _leftStretchCap)
{
    leftStretchCap = _leftStretchCap;
}

void UIControlBackground::SetTopBottomStretchCap(float32 _topStretchCap)
{
    topStretchCap = _topStretchCap;
}

float32 UIControlBackground::GetLeftRightStretchCap() const
{
    return leftStretchCap;
}

float32 UIControlBackground::GetTopBottomStretchCap() const
{
    return topStretchCap;
}

void UIControlBackground::SetMaterial(NMaterial* _material)
{
    SafeRelease(material);
    material = SafeRetain(_material);
}

inline NMaterial* UIControlBackground::GetMaterial() const
{
    return material;
}
};
