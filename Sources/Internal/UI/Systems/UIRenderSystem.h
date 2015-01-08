#ifndef __DAVAENGINE_RENDER_2D_SYSTEM_H__
#define __DAVAENGINE_RENDER_2D_SYSTEM_H__

#include "UISystem.h"
#include "UI/UIControl.h"

namespace DAVA
{

class UIRenderSystem : public UISystem
{
public:
    static const uint32 TYPE = UISystem::UI_RENDER_SYSTEM;
    
    UIRenderSystem();
    virtual ~UIRenderSystem();

    virtual uint32 GetRequiredComponents() const;
    virtual uint32 GetType() const;

    virtual void Process();

    void SystemDraw(UIControl* control, const UIGeometricData& geometricData);
    void Draw(UIControl* control, const UIGeometricData &geometricData);

private:
    void DrawDebugRect(UIControl* control, const UIGeometricData &geometricData, bool useAlpha = false);
    void DrawPivotPoint(UIControl* control, const Rect &drawRect);

};

}


#endif //__DAVAENGINE_RENDER_2D_SYSTEM_H__