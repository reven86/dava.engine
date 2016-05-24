#ifndef __DAVAENGINE_UI_SLIDER_H__
#define __DAVAENGINE_UI_SLIDER_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA
{
class UIButton;
class UISlider : public UIControl
{
protected:
    virtual ~UISlider();

public:
    UISlider(const Rect& rect = Rect());

    void SetSize(const DAVA::Vector2& newSize) override;

    inline float32 GetMinValue() const;
    inline float32 GetMaxValue() const;
    void SetMinMaxValue(float32 _minValue, float32 _maxValue);

    void AddControl(UIControl* control) override;
    void RemoveControl(UIControl* control) override;

    void Draw(const UIGeometricData& geometricData) override;

    inline bool IsEventsContinuos() const;
    inline void SetEventsContinuos(bool isEventsContinuos);
    inline float32 GetValue() const;

    void SetMaxValue(float32 value);
    void SetMinValue(float32 value);
    void SetValue(float32 value);

    void SetThumb(UIControl* newThumb);
    inline UIControl* GetThumb() const;

    inline UIControlBackground* GetBgMin() const;
    inline UIControlBackground* GetBgMax() const;

    void LoadFromYamlNodeCompleted() override;

    UISlider* Clone() override;
    void CopyDataFrom(UIControl* srcControl) override;

    // Synchronize thumb size/position according to the thumb sprite.
    void SyncThumbWithSprite();

protected:
    bool isEventsContinuos;

    int32 leftInactivePart;
    int32 rightInactivePart;

    float32 minValue;
    float32 maxValue;

    float32 currentValue;

    void Input(UIEvent* currentInput) override;

    void RecalcButtonPos();

    UIControlBackground* minBackground;
    UIControlBackground* maxBackground;
    UIControl* thumbButton;

    Vector2 relTouchPoint;

    void InitThumb();

    void AttachToSubcontrols();
    void InitInactiveParts(Sprite* spr);

public:
    int32 GetBackgroundComponentsCount() const override;
    UIControlBackground* GetBackgroundComponent(int32 index) const override;
    UIControlBackground* CreateBackgroundComponent(int32 index) const override;
    void SetBackgroundComponent(int32 index, UIControlBackground* bg) override;
    String GetBackgroundComponentName(int32 index) const override;

    INTROSPECTION_EXTEND(UISlider, UIControl,
                         PROPERTY("minValue", "Min Value", GetMinValue, SetMinValue, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("maxValue", "Max Value", GetMaxValue, SetMaxValue, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("value", "Value", GetValue, SetValue, I_SAVE | I_VIEW | I_EDIT)
                         );

private:
    static const int32 BACKGROUND_COMPONENTS_COUNT = 3;
};

inline UIControl* UISlider::GetThumb() const
{
    return thumbButton;
}

inline UIControlBackground* UISlider::GetBgMin() const
{
    return minBackground;
}

inline UIControlBackground* UISlider::GetBgMax() const
{
    return maxBackground;
}

inline bool UISlider::IsEventsContinuos() const
{
    return isEventsContinuos;
}

inline void UISlider::SetEventsContinuos(bool _isEventsContinuos)
{
    isEventsContinuos = _isEventsContinuos;
}

inline float32 UISlider::GetValue() const
{
    return currentValue;
}

inline float32 UISlider::GetMinValue() const
{
    return minValue;
}

inline float32 UISlider::GetMaxValue() const
{
    return maxValue;
}
};

#endif
