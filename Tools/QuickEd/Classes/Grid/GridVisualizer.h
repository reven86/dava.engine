#ifndef __GRID_VISUALIZER__H__
#define __GRID_VISUALIZER__H__

#include "Base/Singleton.h"
#include "Math/Rect.h"
#include "Render/UniqueStateSet.h"

namespace DAVA
{
// This class helps us to visualize UI Editor Grid if needed.
class GridVisualizer : public Singleton<GridVisualizer>
{
public:
    // Construction/destruction.
    GridVisualizer();
    virtual ~GridVisualizer();

    // Set the current screen scale.
    void SetScale(float32 scale);

    // Draw the grid, if needed. Call this method from the Screen Control.
    void DrawGridIfNeeded(const Rect& rect, UniqueHandle renderState);

protected:
    // Current screen scale.
    float32 curScale;
};
};


#endif /* defined(__GRID_VISUALIZER__H__) */
