#ifndef __DAVAENGINE_UI_3D_VIEW__
#define __DAVAENGINE_UI_3D_VIEW__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "Render/RHI/rhi_type.h"

namespace DAVA
{
/**
    \ingroup controlsystem
    \brief This control allow to put 3D View into any place of 2D hierarchy
 */

class Scene;
class UI3DView : public UIControl, public TrackedObject
{
public:
    UI3DView(const Rect& rect = Rect());

protected:
    virtual ~UI3DView();

public:
    void SetScene(Scene* scene);
    Scene* GetScene() const;

    inline const Rect& GetLastViewportRect()
    {
        return viewportRc;
    }

    void AddControl(UIControl* control) override;
    void Draw(const UIGeometricData& geometricData) override;

    void SetSize(const Vector2& newSize) override;
    UI3DView* Clone() override;
    void CopyDataFrom(UIControl* srcControl) override;

    void Input(UIEvent* currentInput) override;

    void OnVisible() override;
    void OnInvisible() override;

    void SetDrawToFrameBuffer(bool enable);
    bool GetDrawToFrameBuffer() const;
    void SetFrameBufferScaleFactor(float32 scale);
    float32 GetFrameBufferScaleFactor() const;
    const Vector2& GetFrameBufferRenderSize() const;

    int32 GetBasePriority();
    void SetBasePriority(int32 priority);

    bool IsClearRequested() const;
    void SetClearRequested(bool requested);

protected:
    Scene* scene;
    Rect viewportRc;
    bool registeredInUIControlSystem;

private:
    void Update(float32 timeElapsed);

    void PrepareFrameBuffer();

    bool drawToFrameBuffer;

    float32 fbScaleFactor;
    Vector2 fbRenderSize;
    Vector2 fbTexSize = Vector2(0.f, 0.f);
    Texture* frameBuffer = nullptr;

    int32 basePriority = PRIORITY_MAIN_3D;

    rhi::LoadAction colorLoadAction = rhi::LOADACTION_CLEAR;

public:
    INTROSPECTION_EXTEND(UI3DView, UIControl,
                         PROPERTY("drawToFrameBuffer", "Draw sceene draw through the frame buffer", GetDrawToFrameBuffer, SetDrawToFrameBuffer, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("frameBufferScaleFactor", "Set scale factor to draw the frame buffer", GetFrameBufferScaleFactor, SetFrameBufferScaleFactor, I_SAVE | I_VIEW | I_EDIT) nullptr)
};

inline bool UI3DView::GetDrawToFrameBuffer() const
{
    return drawToFrameBuffer;
}

inline float32 UI3DView::GetFrameBufferScaleFactor() const
{
    return fbScaleFactor;
}

inline const Vector2& UI3DView::GetFrameBufferRenderSize() const
{
    return fbRenderSize;
}

inline int32 UI3DView::GetBasePriority()
{
    return basePriority;
}
inline void UI3DView::SetBasePriority(int32 priority)
{
    basePriority = priority;
}
};

#endif
