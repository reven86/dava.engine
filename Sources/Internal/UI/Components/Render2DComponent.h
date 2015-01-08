#ifndef __DAVAENGINE_RENDER_2D_COMPONENT_H__
#define __DAVAENGINE_RENDER_2D_COMPONENT_H__

#if 0

#include "UIComponent.h"
#include "UI/UIControl.h"
#include "UI/UIControlBackground.h"
#include "UI/UIControlSystem.h"
#include <Input/InputSystem.h>

namespace DAVA
{

class UIGeometricData
{
    friend class Render2DComponent;

public:
    UIGeometricData()
        : scale(1.0f, 1.0f)
        , angle(0.0f)
        , cosA(1.0f)
        , sinA(0.0f)
        , calculatedAngle(0.0f)
    {
    }
    Vector2 position;
    Vector2 size;

    Vector2 pivotPoint;
    Vector2 scale;
    float32 angle;

    float32 cosA;
    float32 sinA;

    void AddGeometricData(const UIGeometricData &data)
    {
        position.x = data.position.x - data.pivotPoint.x * data.scale.x + position.x * data.scale.x;
        position.y = data.position.y - data.pivotPoint.y * data.scale.y + position.y * data.scale.y;
        if (data.angle != 0)
        {
            float tmpX = position.x;
            position.x = (tmpX - data.position.x) * data.cosA + (data.position.y - position.y) * data.sinA + data.position.x;
            position.y = (tmpX - data.position.x) * data.sinA + (position.y - data.position.y) * data.cosA + data.position.y;
        }
        scale.x *= data.scale.x;
        scale.y *= data.scale.y;
        angle += data.angle;
        if (angle != calculatedAngle)
        {
            if (angle != data.angle)
            {
                cosA = cosf(angle);
                sinA = sinf(angle);
            }
            else
            {
                cosA = data.cosA;
                sinA = data.sinA;
            }
            calculatedAngle = angle;
        }

        unrotatedRect.x = position.x - pivotPoint.x * scale.x;
        unrotatedRect.y = position.y - pivotPoint.y * scale.y;
        unrotatedRect.dx = size.x * scale.x;
        unrotatedRect.dy = size.y * scale.y;
    }

    DAVA_DEPRECATED(void AddToGeometricData(const UIGeometricData &data))
    {
        AddGeometricData(data);
    }

    void BuildTransformMatrix(Matrix3 &transformMatr) const
    {
        Matrix3 pivotMatr;
        pivotMatr.BuildTranslation(-pivotPoint);

        Matrix3 translateMatr;
        translateMatr.BuildTranslation(position);

        Matrix3 rotateMatr;
        rotateMatr.BuildRotation(cosA, sinA);

        Matrix3 scaleMatr;
        scaleMatr.BuildScale(scale);

        transformMatr = pivotMatr * scaleMatr * rotateMatr * translateMatr;
    }

    void GetPolygon(Polygon2 &polygon) const
    {
        polygon.Clear();
        polygon.points.reserve(4);
        polygon.AddPoint(Vector2());
        polygon.AddPoint(Vector2(size.x, 0));
        polygon.AddPoint(size);
        polygon.AddPoint(Vector2(0, size.y));

        Matrix3 transformMtx;
        BuildTransformMatrix(transformMtx);
        polygon.Transform(transformMtx);
    }

    const Rect &GetUnrotatedRect() const
    {
        return unrotatedRect;
    }

    Rect GetAABBox() const
    {
        Polygon2 polygon;
        GetPolygon(polygon);

        AABBox2 aabbox;
        for (int32 i = 0; i < polygon.GetPointCount(); ++i)
        {
            aabbox.AddPoint(polygon.GetPoints()[i]);
        }
        Rect bboxRect = Rect(aabbox.min, aabbox.max - aabbox.min);
        return bboxRect;
    }


private:
    Rect unrotatedRect;
    float32 calculatedAngle;

};

class Render2DComponent : public UIComponent
{
public:
    enum eDebugDrawPivotMode
    {
        DRAW_NEVER = 1, //!<Never draw the Pivot Point.
        DRAW_ONLY_IF_NONZERO,    //!<Draw the Pivot Point only if it is defined (nonzero).
        DRAW_ALWAYS              //!<Always draw the Pivot Point mark.
    };

protected:
    virtual ~Render2DComponent();

public:
    Render2DComponent();
    virtual Component * Clone(UIControl* toControl);
    virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    virtual uint32 GetType();

    virtual UIControlBackground::eDrawType GetSpriteDrawType() const;
    virtual void SetSpriteDrawType(UIControlBackground::eDrawType drawType);

    virtual Sprite* GetSprite() const;
    virtual int32 GetFrame() const;
    virtual void SetSprite(const FilePath &spriteName, int32 spriteFrame);
    virtual void SetSprite(Sprite *newSprite, int32 spriteFrame);
    virtual void SetSpriteFrame(int32 spriteFrame);
    virtual void SetSpriteFrame(const FastName& frameName);
    
    virtual int32 GetSpriteAlign() const;
    virtual void SetSpriteAlign(int32 align);

    virtual Rect GetRect() const;
    virtual void SetRect(const Rect &rect);

    virtual Rect GetAbsoluteRect();
    virtual void SetAbsoluteRect(const Rect &rect);

    virtual const Vector2& GetPosition() const;
    virtual void SetPosition(const Vector2 &position);

    virtual Vector2 GetAbsolutePosition();
    virtual void SetAbsolutePosition(const Vector2 &position);

    virtual const Vector2& GetSize() const;
    virtual void SetSize(const Vector2 &newSize);

    virtual const Vector2 &GetPivotPoint() const;
    virtual void SetPivotPoint(const Vector2 &newPivot);

    virtual eDebugDrawPivotMode GetDrawPivotPointMode() const;
    virtual void SetDrawPivotPointMode(eDebugDrawPivotMode mode);

    virtual Vector2 GetPivot() const;
    virtual void SetPivot(const Vector2 &newPivot);

    virtual const Vector2& GetScale() const;
    virtual void SetScale(const Vector2 &newScale);

    virtual const UIGeometricData& GetGeometricData() const;
    virtual UIGeometricData GetLocalGeometricData() const;

    virtual float32 GetAngle() const;
    virtual float32 GetAngleInDegrees() const;

    virtual void SetAngle(float32 angleInRad);
    virtual void SetAngleInDegrees(float32 angle);

    virtual bool GetVisible() const;
    virtual void SetVisible(bool isVisible);

    virtual bool GetClipContents() const;
    virtual void SetClipContents(bool isNeedToClipContents);

    bool GetDebugDraw() const;
    void SetDebugDraw(bool debugDraw);
    const Color &GetDebugDrawColor() const;
    void SetDebugDrawColor(const Color& color);

    virtual bool GetVisibleForUIEditor() const;
    virtual void SetVisibleForUIEditor(bool value);

    virtual bool GetSystemVisible() const;

    /****************************************************/

    virtual void SetSizeFromBg(bool pivotToCenter = true);

private:
    UIControlBackground* background;
    Color debugDrawColor;
    eDebugDrawPivotMode drawPivotPointMode;

    Vector2 relativePosition;
    Vector2 size;
    Vector2 pivotPoint;
    Vector2 scale;
    float32 angle;

    bool visible : 1;
    bool clipContents : 1;
    bool debugDrawEnabled : 1;
    bool visibleForUIEditor : 1;

    mutable UIGeometricData tempGeometricData;

};

inline uint32 Render2DComponent::GetType()
{
    return Component::RENDER_2D_COMPONENT;
}

inline UIControlBackground::eDrawType Render2DComponent::GetSpriteDrawType() const
{
    return background->GetDrawType();
}

inline void Render2DComponent::SetSpriteDrawType(UIControlBackground::eDrawType drawType)
{
    background->SetDrawType(drawType);
}

inline Sprite* Render2DComponent::GetSprite() const
{
    return background->GetSprite();
}

inline int32 Render2DComponent::GetFrame() const
{
    return background->GetFrame();
}

inline void Render2DComponent::SetSprite(const FilePath& spriteName, int32 spriteFrame)
{
    background->SetSprite(spriteName, spriteFrame);
}

inline void Render2DComponent::SetSprite(Sprite* newSprite, int32 spriteFrame)
{
    background->SetSprite(newSprite, spriteFrame);
}

inline void Render2DComponent::SetSpriteFrame(int32 spriteFrame)
{
    background->SetFrame(spriteFrame);
}

inline void Render2DComponent::SetSpriteFrame(const FastName& frameName)
{
    background->SetFrame(frameName);
}

inline int32 Render2DComponent::GetSpriteAlign() const
{
    return background->GetAlign();
}

inline void Render2DComponent::SetSpriteAlign(int32 align)
{
    background->SetAlign(align);
}

inline Rect Render2DComponent::GetRect() const
{
    return Rect(GetPosition() - GetPivotPoint(), GetSize());
}

inline Render2DComponent::eDebugDrawPivotMode Render2DComponent::GetDrawPivotPointMode() const
{
    return drawPivotPointMode;
}

inline void Render2DComponent::SetRect(const Rect& rect)
{
    SetSize(rect.GetSize());
    SetPosition(rect.GetPosition() + GetPivotPoint());

    // Update aligns if control was resized manually
    GetControl()->CalculateAlignSettings();
}

inline Rect Render2DComponent::GetAbsoluteRect()
{
    return Rect(GetAbsolutePosition() - GetPivotPoint(), size);
}

inline void Render2DComponent::SetAbsoluteRect(const Rect& rect)
{
    if (!GetControl()->GetParent())
    {
        SetRect(rect);
        return;
    }

    Rect localRect = rect;
    const UIGeometricData &parentGD = GetControl()->GetParent()->GetGeometricData();
    localRect.SetPosition(rect.GetPosition() - parentGD.position + parentGD.pivotPoint);
    SetRect(localRect);
}

inline const Vector2& Render2DComponent::GetPosition() const
{
    return relativePosition;
}

inline void Render2DComponent::SetPosition(const Vector2& position)
{
    relativePosition = position;
}

inline Vector2 Render2DComponent::GetAbsolutePosition()
{
    return GetGeometricData().position;
}

inline void Render2DComponent::SetAbsolutePosition(const Vector2& position)
{
    if (GetControl()->GetParent())
    {
        const UIGeometricData &parentGD = GetControl()->GetParent()->GetGeometricData();
        SetPosition(position - parentGD.position + parentGD.pivotPoint);
    }
    else
    {
        SetPosition(position);
    }
}

inline const Vector2& Render2DComponent::GetSize() const
{
    return size;
}

inline void Render2DComponent::SetSize(const Vector2& newSize)
{
    if (size == newSize)
        return;

    Vector2 oldPivot = GetPivot();
    size = newSize;
    SetPivot(oldPivot);

    // Update size and align of childs
    GetControl()->UpdateChildrenLayout();
}

inline const Vector2& Render2DComponent::GetPivotPoint() const
{
    return pivotPoint;
}

inline void Render2DComponent::SetPivotPoint(const Vector2& newPivot)
{
    pivotPoint = newPivot;
}

inline void Render2DComponent::SetDrawPivotPointMode(eDebugDrawPivotMode mode)
{
    drawPivotPointMode = mode;
}

inline Vector2 Render2DComponent::GetPivot() const
{
    Vector2 pivot;
    pivot.x = (size.x == 0.0f) ? 0.0f : (pivotPoint.x / size.x);
    pivot.y = (size.y == 0.0f) ? 0.0f : (pivotPoint.y / size.y);
    return pivot;
}

inline void Render2DComponent::SetPivot(const Vector2& newPivot)
{
    SetPivotPoint(size * newPivot);
}

inline const Vector2& Render2DComponent::GetScale() const
{
    return scale;
}

inline void Render2DComponent::SetScale(const Vector2& newScale)
{
    scale = newScale;
}

inline const UIGeometricData& Render2DComponent::GetGeometricData() const
{
    tempGeometricData.position = relativePosition;
    tempGeometricData.size = size;
    tempGeometricData.pivotPoint = pivotPoint;
    tempGeometricData.scale = scale;
    tempGeometricData.angle = angle;
    tempGeometricData.unrotatedRect.x = relativePosition.x - relativePosition.x * scale.x;
    tempGeometricData.unrotatedRect.y = relativePosition.y - pivotPoint.y * scale.y;
    tempGeometricData.unrotatedRect.dx = size.x * scale.x;
    tempGeometricData.unrotatedRect.dy = size.y * scale.y;

    if (!GetControl()->GetParent())
    {
        tempGeometricData.AddGeometricData(UIControlSystem::Instance()->GetBaseGeometricData());
        return tempGeometricData;
    }
    tempGeometricData.AddGeometricData(GetControl()->GetParent()->GetGeometricData());
    return tempGeometricData;
}

inline UIGeometricData Render2DComponent::GetLocalGeometricData() const
{
    UIGeometricData drawData;
    drawData.position = relativePosition;
    drawData.size = size;
    drawData.pivotPoint = pivotPoint;
    drawData.scale = scale;
    drawData.angle = angle;
    return drawData;
}

inline float32 Render2DComponent::GetAngle() const
{
    return angle;
}

inline float32 Render2DComponent::GetAngleInDegrees() const
{
    return RadToDeg(angle);
}

inline void Render2DComponent::SetAngle(float32 angleInRad)
{
    angle = angleInRad;
}

inline void Render2DComponent::SetAngleInDegrees(float32 angle)
{
    SetAngle(DegToRad(angle));
}

inline bool Render2DComponent::GetVisible() const
{
    return visible;
}

inline void Render2DComponent::SetVisible(bool isVisible)
{
    if (visible == isVisible)
    {
        return;
    }

    bool oldSystemVisible = GetSystemVisible();
    visible = isVisible;
    if (GetSystemVisible() == oldSystemVisible)
    {
        return;
    }

    GetControl()->SystemNotifyVisibilityChanged();
}

inline bool Render2DComponent::GetClipContents() const
{
    return clipContents;
}

inline void Render2DComponent::SetClipContents(bool isNeedToClipContents)
{
    clipContents = isNeedToClipContents;
}

inline bool Render2DComponent::GetDebugDraw() const
{
    return debugDrawEnabled;
}

inline void Render2DComponent::SetDebugDraw(bool debugDraw)
{
    debugDrawEnabled = debugDraw;
}

inline void Render2DComponent::SetDebugDrawColor(const Color& color)
{
    debugDrawColor = color;
}

inline const Color& Render2DComponent::GetDebugDrawColor() const
{
    return debugDrawColor;
}

inline bool Render2DComponent::GetVisibleForUIEditor() const
{
    return visibleForUIEditor;
}

inline void Render2DComponent::SetVisibleForUIEditor(bool value)
{
    if (visibleForUIEditor == value)
    {
        return;
    }

    bool oldSystemVisible = GetSystemVisible();
    visibleForUIEditor = value;
    if (GetSystemVisible() == oldSystemVisible)
    {
        return;
    }

    GetControl()->SystemNotifyVisibilityChanged();
}

inline bool Render2DComponent::GetSystemVisible() const
{
    return visible & visibleForUIEditor;
}

inline void Render2DComponent::SetSizeFromBg(bool pivotToCenter)
{
    SetSize(GetSprite()->GetSize());
    if (pivotToCenter)
    {
        SetPivot(Vector2(0.5f, 0.5f));
    }
}

}

#endif //0

#endif //__DAVAENGINE_RENDER_2D_COMPONENT_H__