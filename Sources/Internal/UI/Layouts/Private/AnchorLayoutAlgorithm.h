#ifndef __DAVAENGINE_ANCHOR_LAYOUT_ALGORITHM_H__
#define __DAVAENGINE_ANCHOR_LAYOUT_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
class AnchorLayoutAlgorithm
{
public:
    AnchorLayoutAlgorithm(Vector<ControlLayoutData>& layoutData_, bool isRtl_);
    ~AnchorLayoutAlgorithm();

    void Apply(ControlLayoutData& data, Vector2::eAxis axis, bool onlyForIgnoredControls);
    void Apply(ControlLayoutData& data, Vector2::eAxis axis, bool onlyForIgnoredControls, int32 firstIndex, int32 lastIndex);

    static void ApplyAnchor(ControlLayoutData& data, Vector2::eAxis axis, float32 min, float32 max, bool isRtl);

private:
    Vector<ControlLayoutData>& layoutData;

    const bool isRtl;
};
}


#endif //__DAVAENGINE_ANCHOR_LAYOUT_ALGORITHM_H__
