#ifndef __DAVAENGINE_UI_FLOW_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_FLOW_LAYOUT_COMPONENT_H__

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include <bitset>

namespace DAVA
{
class UIFlowLayoutComponent : public UIBaseComponent<UIComponent::FLOW_LAYOUT_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIFlowLayoutComponent, UIBaseComponent<UIComponent::FLOW_LAYOUT_COMPONENT>);

public:
    enum eOrientation
    {
        ORIENTATION_LEFT_TO_RIGHT,
        ORIENTATION_RIGHT_TO_LEFT
    };

public:
    UIFlowLayoutComponent();
    UIFlowLayoutComponent(const UIFlowLayoutComponent& src);

protected:
    virtual ~UIFlowLayoutComponent();

private:
    UIFlowLayoutComponent& operator=(const UIFlowLayoutComponent&) = delete;

public:
    virtual UIFlowLayoutComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enabled);

    eOrientation GetOrientation() const;
    void SetOrientation(eOrientation orientation);

    float32 GetHorizontalPadding() const;
    void SetHorizontalPadding(float32 padding);

    float32 GetHorizontalSpacing() const;
    void SetHorizontalSpacing(float32 spacing);

    bool IsDynamicHorizontalPadding() const;
    void SetDynamicHorizontalPadding(bool dynamic);

    bool IsDynamicHorizontalInLinePadding() const;
    void SetDynamicHorizontalInLinePadding(bool dynamic);

    bool IsDynamicHorizontalSpacing() const;
    void SetDynamicHorizontalSpacing(bool dynamic);

    float32 GetVerticalPadding() const;
    void SetVerticalPadding(float32 padding);

    float32 GetVerticalSpacing() const;
    void SetVerticalSpacing(float32 spacing);

    bool IsDynamicVerticalPadding() const;
    void SetDynamicVerticalPadding(bool dynamic);

    bool IsDynamicVerticalSpacing() const;
    void SetDynamicVerticalSpacing(bool dynamic);

    float32 GetPaddingByAxis(int32 axis);
    float32 GetSpacingByAxis(int32 axis);

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
        FLAG_DYNAMIC_HORIZONTAL_PADDING,
        FLAG_DYNAMIC_HORIZONTAL_IN_LINE_PADDING,
        FLAG_DYNAMIC_HORIZONTAL_SPACING,
        FLAG_DYNAMIC_VERTICAL_PADDING,
        FLAG_DYNAMIC_VERTICAL_SPACING,
        FLAG_USE_RTL,
        FLAG_SKIP_INVISIBLE_CONTROLS,
        FLAG_IS_RIGHT_TO_LEFT,
        FLAG_COUNT
    };

    Array<float32, Vector2::AXIS_COUNT> padding;
    Array<float32, Vector2::AXIS_COUNT> spacing;

    std::bitset<eFlags::FLAG_COUNT> flags;
};
}


#endif //__DAVAENGINE_UI_FLOW_LAYOUT_COMPONENT_H__
