#ifndef __DAVAENGINE_UI_CONTROL_BACKGROUND_H__
#define __DAVAENGINE_UI_CONTROL_BACKGROUND_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/GlobalEnum.h"
#include "Components/UIComponent.h"
#include "Render/2D/Sprite.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class UIControl;
class UIGeometricData;
struct TiledDrawData;
struct StretchDrawData;
struct TiledMultilayerData;
class NMaterial;

/**
     \ingroup controlsystem
     \brief Control background.
        Responsible for control graphical representation.
        In most cases UIControlBackground draws sprite inside control rect in compliance
        with the set of requested rules.
     */

class UIControlBackground : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIControlBackground, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIControlBackground);

public:
    /**
     \enum Control draw types.
     */
    enum eDrawType
    {
        DRAW_ALIGNED = 0, //!<Align sprite inside ronctrol rect.
        DRAW_SCALE_TO_RECT, //!<Scale sprite along the all control rect.
        DRAW_SCALE_PROPORTIONAL, //!<Scale sprite to fit both width and height into the control rect but with keeping sprite proportions.
        DRAW_SCALE_PROPORTIONAL_ONE, //!<Scale sprite to fit width or height into control rect but with keeping sprite proportions.
        DRAW_FILL, //!<Fill control rect with the control color.
        DRAW_STRETCH_HORIZONTAL, //!<Stretch sprite horizontally along the control rect.
        DRAW_STRETCH_VERTICAL, //!<Stretch sprite vertically along the control rect.
        DRAW_STRETCH_BOTH, //!<Stretch sprite along the all control rect.
        DRAW_TILED, //!<Fill control with sprite tiles
        DRAW_TILED_MULTILAYER //!uses for texture - tiled background (withot stretch caps!), stretch mask and contour using same stratch caps, and full back gradient overlay
    };

    /**
     \enum Type of the color inheritnce from the parent control.
     */
    enum eColorInheritType
    {
        COLOR_MULTIPLY_ON_PARENT = 0, //!<Draw color = control color * parent color.
        COLOR_ADD_TO_PARENT, //!<Draw color = Min(control color + parent color, 1.0f).
        COLOR_REPLACE_TO_PARENT, //!<Draw color = parent color.
        COLOR_IGNORE_PARENT, //!<Draw color = control color.
        COLOR_MULTIPLY_ALPHA_ONLY, //!<Draw color = control color. Draw alpha = control alpha * parent alpha.
        COLOR_REPLACE_ALPHA_ONLY, //!<Draw color = control color. Draw alpha = parent alpha.
        COLOR_INHERIT_TYPES_COUNT
    };

    enum ePerPixelAccuracyType
    {
        PER_PIXEL_ACCURACY_DISABLED = 0,
        PER_PIXEL_ACCURACY_ENABLED,
        PER_PIXEL_ACCURACY_FORCED,

        PER_PIXEL_ACCURACY_TYPES
    };

    /**
     \brief Constructor.
     */
    UIControlBackground();

    UIControlBackground(const UIControlBackground& src);

    /**
     \brief Returns Sprite used for draw.
     \returns Sprite used for draw.
     */
    virtual Sprite* GetSprite() const;
    /**
     \brief Returns Sprite frame used for draw.
     \returns Sprite frame used for draw.
     */
    virtual int32 GetFrame() const;
    /**
     \brief Returns Sprite align in the control rect.
     \returns Sprite eAlign bit mask used for draw.
     */
    virtual int32 GetAlign() const;
    /**
     \brief Returns current draw type.
     \returns Draw type used for draw.
     */
    virtual UIControlBackground::eDrawType GetDrawType() const;
    /**
     \brief Returns horizontal or vertical sprite flips.
     \returns eSpriteModification bits.
     */
    virtual int32 GetModification() const;
    /**
     \brief Returns Sprite color used for draw.
     \returns Sprite color used for draw.
     */
    inline const Color& GetColor() const;

    /**
     \brief Sets control Sprite.
     \param[in] drawSprite Pointer for a Sprite.
     \param[in] drawFrame Sprite frame you want to use for draw.
     */
    virtual void SetSprite(Sprite* drawSprite, int32 drawFrame);
    /**
     \brief Sets control Sprite.
     \param[in] drawSprite Sprite path-name.
     \param[in] drawFrame Sprite frame you want to use for draw.
     */
    virtual void SetSprite(const FilePath& path, int32 drawFrame);

    /**
    \brief Sets bg Sprite.
    \param[in] drawSprite Pointer for a Sprite.
    */
    void SetSprite(Sprite* drawSprite);
    /**
    \brief Sets bg Sprite.
    \param[in] drawSprite Sprite path-name.
    */
    void SetSprite(const FilePath& path);
    /**
     \brief Sets Sprite align in the control rect you want to use for draw.
     \param[in] drawAlign Sprite eAlign bit mask.
     */
    virtual void SetAlign(int32 drawAlign);
    /**
     \brief Sets draw type you want to use.
     \param[in] drawType Draw type eDrawType to use for drawing.
     */
    virtual void SetDrawType(UIControlBackground::eDrawType drawType);
    /**
     \brief Sets Sprite frame you want to use.
     \param[in] drawFrame Sprite frame.
     */
    virtual void SetFrame(int32 drawFrame);
    /**
     \brief Sets Sprite frame you want to use.
     \param[in] frameName Sprite frame name.
     */
    virtual void SetFrame(const FastName& frameName);
    /**
     \brief Sets size of the left and right unscalable sprite part.
        Middle sprite part would be scaled along a full control width.
        Used for DRAW_STRETCH_HORIZONTAL, DRAW_STRETCH_BOTH draw types.
     \param[in] leftStretchCap Unscalable part in pixels.
     */
    virtual void SetLeftRightStretchCap(float32 leftStretchCap);
    /**
     \brief Sets size of the top and bottom unscalable sprite part.
        Middle sprite part would be scaled along a full control height.
        Used for DRAW_STRETCH_VERTICAL, DRAW_STRETCH_BOTH draw types.
     \param[in] topStretchCap Unscalable part in pixels.
     */
    virtual void SetTopBottomStretchCap(float32 topStretchCap);

    /*
     Getters for StretchCap
     */
    virtual float32 GetLeftRightStretchCap() const;
    virtual float32 GetTopBottomStretchCap() const;
    /**
     \brief Sets horizontal or vertical sprite flip modificators.
     \param[in] modification eSpriteModification bit mask.
     */
    virtual void SetModification(int32 modification);

    /**
     \brief Sets color inheritance type.
        Color inheritance type it's a rules of the calculating resulting draw color
        from the current control color and parent control color.
        Color inheritance type by default is COLOR_IGNORE_PARENT.
     \param[in] inheritType color inheritance type.
     */
    virtual void SetColorInheritType(UIControlBackground::eColorInheritType inheritType);

    /**
     \brief Returns current color inheritance type.
        Color inheritance type it's a rules of the calculating resulting draw color
        from the current control color and parent control color.
        Color inheritance type by default is COLOR_IGNORE_PARENT.
     \returns eColorInheritType color inheritance type.
     */
    virtual eColorInheritType GetColorInheritType() const;

    /**
     \brief Sets per pixel accuracy type. Enable per pixel accuracy if you want to draw controls using int coordinates instead of float. Disabled by default.
     \param[in] accuracyType. Use PER_PIXEL_ACCURACY_ENABLED to enable it for static controls. Use PER_PIXEL_ACCURACY_FORCED to enable it both for static controls and for control position animations.
     */
    void SetPerPixelAccuracyType(UIControlBackground::ePerPixelAccuracyType accuracyType);

    /**
     \brief Returns per pixel accuracy type. Enable per pixel accuracy if you want to draw controls using int coordinates instead of float. Disabled by default.
     \returns ePerPixelAccuracyType per pixel accuracy type
     */
    UIControlBackground::ePerPixelAccuracyType GetPerPixelAccuracyType() const;

    /**
     \brief Draw selected sprite by selected rules in the current control rect.
        Default color is Color(1,1,1,1).
     \param[in] geometricData Control geometric data.
     */

    virtual void Draw(const UIGeometricData& geometricData);

    /**
     \brief Creates the absoulutely identic copy of the background.
     \returns UIControlBackground copy
     */
    UIControlBackground* Clone() const override;

    /**
     \brief Returns final draw color. This color is affected by the parrent color.
     \returns Real draw color.
     */
    const Color& GetDrawColor() const;

    void SetDrawColor(const Color& c);

    /**
     \brief Sets parent control color.
     \param[in] parentColor parent control color.
     */
    void SetParentColor(const Color& parentColor);

    /**
     \brief Sets draw color.
        Default color is Color(1,1,1,1).
     \param[in] color control draw color.
     */
    inline void SetColor(const Color& color);

    // WTF? Probably we should move it to protected to avoid problems in future?
    Color color; //!<Control color. By default is Color(1,1,1,1).

    void SetMaterial(NMaterial* material);
    NMaterial* GetMaterial() const;

protected:
    RefPtr<Sprite> spr;
    int32 align = ALIGN_HCENTER | ALIGN_VCENTER;
    eDrawType type = DRAW_ALIGNED;
    int32 spriteModification = 0;
    float32 leftStretchCap = 0.0f;
    float32 topStretchCap = 0.0f;
    eColorInheritType colorInheritType = COLOR_IGNORE_PARENT;
    int32 frame = 0;

    Vector2 lastDrawPos;

    ePerPixelAccuracyType perPixelAccuracyType = PER_PIXEL_ACCURACY_DISABLED; //!<Is sprite should be drawn with per pixel accuracy. Used for texts, for example.

    RefPtr<Sprite> mask;
    RefPtr<Sprite> detail;
    RefPtr<Sprite> gradient;
    RefPtr<Sprite> contour;
    eGradientBlendMode gradientMode = GRADIENT_MULTIPLY;

private:
    TiledDrawData* tiledData = nullptr;
    StretchDrawData* stretchData = nullptr;
    TiledMultilayerData* tiledMultulayerData = nullptr;

public:
    void ReleaseDrawData(); // Delete all spec draw data
#if defined(LOCALIZATION_DEBUG)
    const Sprite::DrawState& GetLastDrawState() const;
#endif
protected:
    ~UIControlBackground();
    Color drawColor;

    NMaterial* material = nullptr;
#if defined(LOCALIZATION_DEBUG)
    Sprite::DrawState lastDrawState;
#endif

public:
    // for introspection

    FilePath GetBgSpritePath() const;

    FilePath GetMaskSpritePath() const;
    void SetMaskSpriteFromPath(const FilePath& path);
    void SetMaskSprite(Sprite* sprite);
    FilePath GetDetailSpritePath() const;
    void SetDetailSpriteFromPath(const FilePath& path);
    void SetDetailSprite(Sprite* sprite);
    FilePath GetGradientSpritePath() const;
    void SetGradientSpriteFromPath(const FilePath& path);
    void SetGradientSprite(Sprite* sprite);
    FilePath GetContourSpritePath() const;
    void SetContourSpriteFromPath(const FilePath& path);
    void SetContourSprite(Sprite* sprite);

    eGradientBlendMode GetGradientBlendMode() const;
    void SetGradientBlendMode(eGradientBlendMode mode);
};

// Implementation
inline void UIControlBackground::SetColor(const Color& _color)
{
    color = _color;
}

inline const Color& UIControlBackground::GetColor() const
{
    return color;
}

inline FilePath UIControlBackground::GetBgSpritePath() const
{
    if (GetSprite() == NULL)
        return "";
    else if (GetSprite()->GetRelativePathname().GetType() == FilePath::PATH_IN_MEMORY)
        return "";
    else
        return Sprite::GetPathString(GetSprite());
}

inline FilePath UIControlBackground::GetMaskSpritePath() const
{
    if ((mask != nullptr) && (mask->GetRelativePathname().GetType() != FilePath::PATH_IN_MEMORY))
        return Sprite::GetPathString(mask.Get());
    return "";
}
inline void UIControlBackground::SetMaskSpriteFromPath(const FilePath& path)
{
    if (path != "")
        mask.Set(Sprite::Create(path));
    else
        mask.Set(nullptr);
}
inline void UIControlBackground::SetMaskSprite(Sprite* sprite)
{
    mask = sprite;
}

inline FilePath UIControlBackground::GetDetailSpritePath() const
{
    if ((detail != nullptr) && (detail->GetRelativePathname().GetType() != FilePath::PATH_IN_MEMORY))
        return Sprite::GetPathString(detail.Get());
    return "";
}
inline void UIControlBackground::SetDetailSpriteFromPath(const FilePath& path)
{
    if (path != "")
        detail.Set(Sprite::Create(path));
    else
        detail.Set(nullptr);
}
inline void UIControlBackground::SetDetailSprite(Sprite* sprite)
{
    detail = sprite;
}

inline FilePath UIControlBackground::GetGradientSpritePath() const
{
    if ((gradient != nullptr) && (gradient->GetRelativePathname().GetType() != FilePath::PATH_IN_MEMORY))
        return Sprite::GetPathString(gradient.Get());
    return "";
}
inline void UIControlBackground::SetGradientSpriteFromPath(const FilePath& path)
{
    if (path != "")
        gradient.Set(Sprite::Create(path));
    else
        gradient.Set(nullptr);
}
inline void UIControlBackground::SetGradientSprite(Sprite* sprite)
{
    gradient = sprite;
}

inline FilePath UIControlBackground::GetContourSpritePath() const
{
    if ((contour != nullptr) && (contour->GetRelativePathname().GetType() != FilePath::PATH_IN_MEMORY))
        return Sprite::GetPathString(contour.Get());
    return "";
}
inline void UIControlBackground::SetContourSpriteFromPath(const FilePath& path)
{
    if (path != "")
        contour.Set(Sprite::Create(path));
    else
        contour.Set(nullptr);
}
inline void UIControlBackground::SetContourSprite(Sprite* sprite)
{
    contour = sprite;
}

inline eGradientBlendMode UIControlBackground::GetGradientBlendMode() const
{
    return gradientMode;
}
inline void UIControlBackground::SetGradientBlendMode(eGradientBlendMode mode)
{
    gradientMode = mode;
}
};

#endif
