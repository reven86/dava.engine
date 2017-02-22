#pragma once

#include "Engine/Private/Qt/RenderWidgetBackend.h"

#include <Base/BaseTypes.h>

#include <QQuickWidget>

namespace DAVA
{
class RenderWidgetOGL : public RenderWidgetBackendImpl<QQuickWidget>
{
    using TBase = RenderWidgetBackendImpl<QQuickWidget>;

public:
    RenderWidgetOGL(IWindowDelegate* widgetDelegate_, uint32 width, uint32 height, QWidget* parent);

    void ActivateRendering() override;
    bool IsInitialized() const override;
    void Update() override;
    void InitCustomRenderParams(rhi::InitParam& params) override;
    void AcquireContext() override;
    void ReleaseContext() override;

protected:
    bool eventFilter(QObject* object, QEvent* e) override;
    bool IsInFullScreen() const override;

    virtual QWindow* GetQWindow() override;

private:
    void OnCreated();
    void OnFrame();
    void OnActiveFocusItemChanged();
    void OnSceneGraphInvalidated();

private:
    bool keyEventRecursiveGuard = false;
    bool isInPaint = false;

    class OGLContextBinder;
    friend void AcquireContextImpl();
    friend void ReleaseContextImpl();
    std::unique_ptr<OGLContextBinder> contextBinder;
};

} // namespace DAVA