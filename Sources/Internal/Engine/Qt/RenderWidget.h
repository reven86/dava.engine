#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Functional/Signal.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/EngineTypes.h"

#include <QWidget>

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
class IWindowDelegate;
class IClientDelegate;
class RenderWidgetBackend;

class RenderWidget final : public QWidget
{
    Q_OBJECT
public:
    void SetClientDelegate(IClientDelegate* delegate);
    Signal<uint32, uint32> resized;

private:
    friend class Private::WindowBackend;

    RenderWidget(IWindowDelegate* widgetDelegate, uint32 width, uint32 height);
    ~RenderWidget();

    void AcquireContext();
    void ReleaseContext();

    bool IsInitialized() const;

    void Update();
    void InitCustomRenderParams(rhi::InitParam& params);

    RenderWidgetBackend* renderWidgetBackend = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
