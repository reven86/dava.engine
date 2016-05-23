#ifndef __DAVAENGINE_UI_LINEAR_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_LINEAR_LAYOUT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UILinearLayoutComponent : public UIBaseComponent<UIComponent::LINEAR_LAYOUT_COMPONENT>
{
public:
    enum eOrientation
    {
        LEFT_TO_RIGHT = 0,
        RIGHT_TO_LEFT = 1,
        TOP_DOWN = 2,
        BOTTOM_UP = 3,
    };

public:
    UILinearLayoutComponent();
    UILinearLayoutComponent(const UILinearLayoutComponent& src);

protected:
    virtual ~UILinearLayoutComponent();

private:
    UILinearLayoutComponent& operator=(const UILinearLayoutComponent&) = delete;

public:
    virtual UILinearLayoutComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enabled);

    eOrientation GetOrientation() const;
    void SetOrientation(eOrientation orientation);

    Vector2::eAxis GetAxis() const;
    bool IsInverse() const;

    float32 GetPadding() const;
    void SetPadding(float32 padding);

    float32 GetSpacing() const;
    void SetSpacing(float32 spacing);

    bool IsDynamicPadding() const;
    void SetDynamicPadding(bool dynamic);

    bool IsDynamicSpacing() const;
    void SetDynamicSpacing(bool dynamic);
    bool IsUseRtl() const;
    void SetUseRtl(bool use);

    bool IsSkipInvisibleControls() const;
    void SetSkipInvisibleControls(bool skip);

private:
    int32 GetOrientationAsInt() const;
    void SetOrientationFromInt(int32 orientation);

    void SetLayoutDirty();

private:
    enum eFlags
    {
        FLAG_ENABLED,
        FLAG_ORIENTATION_VERTICAL,
        FLAG_ORIENTATION_INVERSE,
        FLAG_DYNAMIC_PADDING,
        FLAG_DYNAMIC_SPACING,
        FLAG_RTL,
        FLAG_SKIP_INVISIBLE_CONTROLS,
        FLAG_COUNT
    };

    Bitset<eFlags::FLAG_COUNT> flags;
    float32 padding = 0.0f;
    float32 spacing = 0.0f;

public:
    INTROSPECTION_EXTEND(UILinearLayoutComponent, UIComponent,
                         PROPERTY("enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("orientation", InspDesc("Orientation", GlobalEnumMap<eOrientation>::Instance()), GetOrientationAsInt, SetOrientationFromInt, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("padding", "Padding", GetPadding, SetPadding, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("dynamicPadding", "Dynamic Padding", IsDynamicPadding, SetDynamicPadding, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("spacing", "Spacing", GetSpacing, SetSpacing, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("dynamicSpacing", "Dynamic Spacing", IsDynamicSpacing, SetDynamicSpacing, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("skipInvisible", "Skip Invisible Controls", IsSkipInvisibleControls, SetSkipInvisibleControls, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("useRtl", "Use Rtl Align", IsUseRtl, SetUseRtl, I_SAVE | I_VIEW | I_EDIT | I_LOAD));
};
}


#endif //__DAVAENGINE_UI_LINEAR_LAYOUT_COMPONENT_H__
