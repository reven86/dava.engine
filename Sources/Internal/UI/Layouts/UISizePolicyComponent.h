#ifndef __DAVAENGINE_UI_SIZE_POLICY_COMPONENT_H__
#define __DAVAENGINE_UI_SIZE_POLICY_COMPONENT_H__

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIControl;

class UISizePolicyComponent : public UIBaseComponent<UISizePolicyComponent>
{
    DAVA_VIRTUAL_REFLECTION(UISizePolicyComponent, UIComponent);

public:
    enum eSizePolicy
    {
        IGNORE_SIZE,
        FIXED_SIZE,
        PERCENT_OF_CHILDREN_SUM,
        PERCENT_OF_MAX_CHILD,
        PERCENT_OF_FIRST_CHILD,
        PERCENT_OF_LAST_CHILD,
        PERCENT_OF_CONTENT,
        PERCENT_OF_PARENT
    };

public:
    UISizePolicyComponent();
    UISizePolicyComponent(const UISizePolicyComponent& src);

protected:
    virtual ~UISizePolicyComponent();

private:
    UISizePolicyComponent& operator=(const UISizePolicyComponent&) = delete;

public:
    UISizePolicyComponent* Clone() const override;

    eSizePolicy GetHorizontalPolicy() const;
    void SetHorizontalPolicy(eSizePolicy policy);

    float32 GetHorizontalValue() const;
    void SetHorizontalValue(float32 value);

    float32 GetHorizontalMinValue() const;
    void SetHorizontalMinValue(float32 value);

    float32 GetHorizontalMaxValue() const;
    void SetHorizontalMaxValue(float32 value);

    eSizePolicy GetVerticalPolicy() const;
    void SetVerticalPolicy(eSizePolicy policy);

    float32 GetVerticalValue() const;
    void SetVerticalValue(float32 value);

    float32 GetVerticalMinValue() const;
    void SetVerticalMinValue(float32 value);

    float32 GetVerticalMaxValue() const;
    void SetVerticalMaxValue(float32 value);

    eSizePolicy GetPolicyByAxis(int32 axis) const;
    float32 GetValueByAxis(int32 axis) const;
    float32 GetMinValueByAxis(int32 axis) const;
    float32 GetMaxValueByAxis(int32 axis) const;

    bool IsDependsOnChildren(int32 axis) const;

private:
    void SetLayoutDirty();

private:
    struct AxisPolicy
    {
        eSizePolicy policy;
        float32 value;
        float32 min;
        float32 max;
    };

private:
    Array<AxisPolicy, Vector2::AXIS_COUNT> policy;
};
}


#endif //__DAVAENGINE_UI_SIZE_POLICY_COMPONENT_H__
