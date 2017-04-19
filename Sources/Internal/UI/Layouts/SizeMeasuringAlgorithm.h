#ifndef __DAVAENGINE_SIZE_MEASURING_ALGORITHM_H__
#define __DAVAENGINE_SIZE_MEASURING_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
class UISizePolicyComponent;
class UIFlowLayoutComponent;
class UILinearLayoutComponent;

class SizeMeasuringAlgorithm
{
public:
    SizeMeasuringAlgorithm(Vector<ControlLayoutData>& layoutData_);
    ~SizeMeasuringAlgorithm();

    void Apply(ControlLayoutData& data, Vector2::eAxis axis);

private:
    float32 CalculateFixedSize(ControlLayoutData& data, Vector2::eAxis axis);
    float32 CalculatePercentOfChildrenSum(ControlLayoutData& data, Vector2::eAxis axis);
    float32 CalculateDefaultPercentOfChildrenSum(ControlLayoutData& data, Vector2::eAxis axis);
    float32 CalculateHorizontalFlowLayoutPercentOfChildrenSum(ControlLayoutData& data);
    float32 CalculateVerticalFlowLayoutPercentOfChildrenSum(ControlLayoutData& data);
    float32 CalculatePercentOfMaxChild(ControlLayoutData& data, Vector2::eAxis axis);
    float32 CalculatePercentOfFirstChild(ControlLayoutData& data, Vector2::eAxis axis);
    float32 CalculatePercentOfLastChild(ControlLayoutData& data, Vector2::eAxis axis);
    float32 CalculatePercentOfContent(ControlLayoutData& data, Vector2::eAxis axis);

    void ApplySize(ControlLayoutData& data, float32 value, Vector2::eAxis axis);
    float32 GetSize(const ControlLayoutData& data, Vector2::eAxis axis);
    float32 GetLayoutPadding(Vector2::eAxis axis);
    float32 ClampValue(float32 value, Vector2::eAxis axis);

private:
    Vector<ControlLayoutData>& layoutData;

    UISizePolicyComponent* sizePolicy = nullptr;
    UILinearLayoutComponent* linearLayout = nullptr;
    UIFlowLayoutComponent* flowLayout = nullptr;

    bool skipInvisible = false;
};
}


#endif //__DAVAENGINE_SIZE_MEASURING_ALGORITHM_H__
