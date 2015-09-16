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


#ifndef __DAVAENGINE_UI_CONTROL_BACKGROUND_H__
#define __DAVAENGINE_UI_CONTROL_BACKGROUND_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/2D/Sprite.h"
#include "FileSystem/FilePath.h"
#include "Base/GlobalEnum.h"


namespace DAVA
{

class UIControl;
class UIGeometricData;
struct TiledDrawData;
struct StretchDrawData;

    /**
     \ingroup controlsystem
     \brief Control background.
        Responsible for control graphical representation.
        In most cases UIControlBackground draws sprite inside control rect in compliance
        with the set of requested rules.
     */

class UIControlBackground : public BaseObject
{
public:
    /**
     \enum Control draw types.
     */
    enum eDrawType
    {
        DRAW_ALIGNED = 0,           //!<Align sprite inside ronctrol rect.
        DRAW_SCALE_TO_RECT,         //!<Scale sprite along the all control rect.
        DRAW_SCALE_PROPORTIONAL,    //!<Scale sprite to fit both width and height into the control rect but with keeping sprite proportions.
        DRAW_SCALE_PROPORTIONAL_ONE,//!<Scale sprite to fit width or height into control rect but with keeping sprite proportions.
        DRAW_FILL,                  //!<Fill control rect with the control color.
        DRAW_STRETCH_HORIZONTAL,    //!<Stretch sprite horizontally along the control rect.
        DRAW_STRETCH_VERTICAL,      //!<Stretch sprite vertically along the control rect.
        DRAW_STRETCH_BOTH,          //!<Stretch sprite along the all control rect.
        DRAW_TILED                  //!<Fill control with sprite tiles
    };

    /**
     \enum Type of the color inheritnce from the parent control.
     */
    enum eColorInheritType
    {
        COLOR_MULTIPLY_ON_PARENT = 0,   //!<Draw color = control color * parent color.
        COLOR_ADD_TO_PARENT,            //!<Draw color = Min(control color + parent color, 1.0f).
        COLOR_REPLACE_TO_PARENT,        //!<Draw color = parent color.
        COLOR_IGNORE_PARENT,            //!<Draw color = control color.
        COLOR_MULTIPLY_ALPHA_ONLY,      //!<Draw color = control color. Draw alpha = control alpha * parent alpha.
        COLOR_REPLACE_ALPHA_ONLY,       //!<Draw color = control color. Draw alpha = parent alpha.
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
    struct UIMargins
    {
        UIMargins() :
            top(0.0f), right(0.0f), bottom(0.0f), left(0.0f)
        {
        }

        UIMargins(const Vector4& value)
        {
            left = value.x;
            top = value.y;
            right = value.z;
            bottom = value.w;
        }
    
        inline bool operator == (const UIMargins & value) const;
        inline bool operator != (const UIMargins & value) const;
        inline bool empty() const;

        inline Vector4 AsVector4() const;
        
        float32 top;
        float32 right;
        float32 bottom;
        float32 left;
    };

    /**
     \brief Constructor.
     */
    UIControlBackground();

    virtual bool IsEqualTo(const UIControlBackground *back) const;

    /**
     \brief Returns Sprite used for draw.
     \returns Sprite used for draw.
     */
    virtual Sprite*	GetSprite() const;
    /**
     \brief Returns Sprite frame used for draw.
     \returns Sprite frame used for draw.
     */
    virtual int32	GetFrame() const;
    /**
     \brief Returns Sprite align in the control rect.
     \returns Sprite eAlign bit mask used for draw.
     */
    virtual int32	GetAlign() const;
    /**
     \brief Returns current draw type.
     \returns Draw type used for draw.
     */
    virtual UIControlBackground::eDrawType	GetDrawType() const;
    /**
     \brief Returns horizontal or vertical sprite flips.
     \returns eSpriteModification bits.
     */
    virtual int32	GetModification() const;
    /**
     \brief Returns Sprite color used for draw.
     \returns Sprite color used for draw.
     */
    inline const Color &GetColor() const;


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
    virtual void SetSprite(const FilePath &drawSprite, int32 drawFrame);
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

    virtual void Draw(const UIGeometricData &geometricData);

    /**
     \brief Creates the absoulutely identic copy of the background.
     \returns UIControlBackground copy
     */
    virtual UIControlBackground *Clone();
    /**
     \brief Copies all background parameters from the source.
     \param[in] srcBackground Source background to copy parameters from.
     */
    virtual void CopyDataFrom(UIControlBackground *srcBackground);

    /**
     \brief Returns final draw color. This color is affected by the parrent color.
     \returns Real draw color.
     */
    const Color & GetDrawColor() const;

    void SetDrawColor(const Color &c);

    /**
     \brief Sets parent control color.
     \param[in] parentColor parent control color.
     */
    void SetParentColor(const Color &parentColor);

    /**
     \brief Sets draw color.
        Default color is Color(1,1,1,1).
     \param[in] color control draw color.
     */
    inline void SetColor(const Color & color);

    // WTF? Probably we should move it to protected to avoid problems in future?
    Color color;//!<Control color. By default is Color(1,1,1,1).


    void SetShader(Shader *shader);
    
    /**
     \brief Sets the margins for drawing background. Positive values means inner
     offset, negative ones - outer.
     */
    void SetMargins(const UIMargins* uiMargins);
    
    /**
     \brief Returns the margins for drawing background. Can be NULL.
     */
    inline const UIMargins* GetMargins() const;

    void SetRenderState(UniqueHandle renderState);
    UniqueHandle GetRenderState() const;

protected:

    Sprite *spr;
    int32 align;
    eDrawType type;
    int32 spriteModification;
    float32 leftStretchCap;
    float32 topStretchCap;
    int colorInheritType;
    int32 frame;

    Vector2 lastDrawPos;

    ePerPixelAccuracyType perPixelAccuracyType;//!<Is sprite should be drawn with per pixel accuracy. Used for texts, for example.

private:
	TiledDrawData *tiledData;
    StretchDrawData *stretchData;
    
    UIMargins* margins;

public:
    void ReleaseDrawData(); // Delete all spec draw data
#if defined(LOCALIZATION_DEBUG)
    const Sprite::DrawState & GetLastDrawState()const;
#endif
protected:
    ~UIControlBackground();
    Color drawColor;

    Shader *shader;
    
    UniqueHandle renderState;
#if defined(LOCALIZATION_DEBUG)
    Sprite::DrawState lastDrawState;
#endif
    
public:
    
    // for introspection
    
    inline int GetBgDrawType() const;
    inline void SetBgDrawType(int type);
    inline FilePath GetBgSpritePath() const;
    inline void SetBgSpriteFromPath(const FilePath &path);
    inline int32 GetBgColorInherit() const;
    inline void SetBgColorInherit(int32 type);
    inline int32 GetBgPerPixelAccuracy() const;
    inline void SetBgPerPixelAccuracy(int32 type);
    Vector4 GetMarginsAsVector4() const;
    void SetMarginsAsVector4(const Vector4 &margins);
    
    INTROSPECTION_EXTEND(UIControlBackground, BaseObject,
                         PROPERTY("drawType", InspDesc("Draw Type", GlobalEnumMap<eDrawType>::Instance()), GetBgDrawType, SetBgDrawType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("sprite", "Sprite", GetBgSpritePath, SetBgSpriteFromPath, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("frame", "Sprite Frame", GetFrame, SetFrame, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("spriteModification", "Sprite Modification", GetModification, SetModification, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("color", "Color", GetColor, SetColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("colorInherit", InspDesc("Color Inherit", GlobalEnumMap<eColorInheritType>::Instance()), GetBgColorInherit, SetBgColorInherit, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("perPixelAccuracy", InspDesc("Per Pixel Accuracy", GlobalEnumMap<ePerPixelAccuracyType>::Instance()), GetBgPerPixelAccuracy, SetBgPerPixelAccuracy, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("align", InspDesc("Align", GlobalEnumMap<eAlign>::Instance(), InspDesc::T_FLAGS), GetAlign, SetAlign, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("leftRightStretchCap", "Left-Right Stretch Cap", GetLeftRightStretchCap, SetLeftRightStretchCap, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("topBottomStretchCap", "Top-Bottom Stretch Cap", GetTopBottomStretchCap, SetTopBottomStretchCap, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("margins", "Margins", GetMarginsAsVector4, SetMarginsAsVector4, I_SAVE | I_VIEW | I_EDIT)
                         );

};

// Implementation
inline void UIControlBackground::SetColor(const Color & _color)
{
    color = _color;
}

inline const Color &UIControlBackground::GetColor() const
{
    return color;
}
    
inline void UIControlBackground::SetRenderState(UniqueHandle _renderState)
{
    renderState = _renderState;
}
    
inline UniqueHandle UIControlBackground::GetRenderState() const
{
    return renderState;
}

inline const UIControlBackground::UIMargins* UIControlBackground::GetMargins() const
{
    return margins;
}

inline bool UIControlBackground::UIMargins::operator == (const UIControlBackground::UIMargins& value) const
{
    return FLOAT_EQUAL(left, value.left) && FLOAT_EQUAL(top, value.top) &&
    FLOAT_EQUAL(right, value.right) && FLOAT_EQUAL(bottom, value.bottom);
}

inline bool UIControlBackground::UIMargins::operator != (const UIControlBackground::UIMargins& value) const
{
    return !UIControlBackground::UIMargins::operator == (value);
}

inline bool UIControlBackground::UIMargins::empty() const
{
    return FLOAT_EQUAL(left, 0.0f)  && FLOAT_EQUAL(top, 0.0f) &&
    FLOAT_EQUAL(right, 0.0f) && FLOAT_EQUAL(bottom, 0.0f);
}

inline Vector4 UIControlBackground::UIMargins::AsVector4() const
{
    return Vector4(left, top, right, bottom);
}

int UIControlBackground::GetBgDrawType() const
{
    return GetDrawType();
}

void UIControlBackground::SetBgDrawType(int type)
{ // TODO: FIXME: type
    SetDrawType((UIControlBackground::eDrawType) type);
}

FilePath UIControlBackground::GetBgSpritePath() const
{
    if (GetSprite() == NULL)
        return "";
    else if (GetSprite()->GetRelativePathname().GetType() == FilePath::PATH_IN_MEMORY)
        return "";
    else
        return Sprite::GetPathString(GetSprite());
}

void UIControlBackground::SetBgSpriteFromPath(const FilePath &path)
{
    if (path == "")
        SetSprite(NULL, 0);
    else
        SetSprite(path, GetFrame());
}

int32 UIControlBackground::GetBgColorInherit() const
{
    return GetColorInheritType();
}

void UIControlBackground::SetBgColorInherit(int32 type)
{
    SetColorInheritType((UIControlBackground::eColorInheritType) type);
}

int32 UIControlBackground::GetBgPerPixelAccuracy() const
{
    return GetPerPixelAccuracyType();
}

void UIControlBackground::SetBgPerPixelAccuracy(int32 type)
{
    SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType) type);
}


};

#endif