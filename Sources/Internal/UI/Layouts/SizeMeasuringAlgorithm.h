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
    void ProcessIgnoreSizePolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessFixedSizePolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessPercentOfChildrenSumPolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessDefaultPercentOfChildrenSumPolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessHorizontalFlowLayoutPercentOfChildrenSumPolicy(ControlLayoutData& data);
    void ProcessVerticalFlowLayoutPercentOfChildrenSumPolicy(ControlLayoutData& data);
    void ProcessPercentOfMaxChildPolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessPercentOfFirstChildPolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessPercentOfLastChildPolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessPercentOfContentPolicy(ControlLayoutData& data, Vector2::eAxis axis);
    void ProcessPercentOfParentPolicy(ControlLayoutData& data, Vector2::eAxis axis);

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
