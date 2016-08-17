#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Public/Qt/RenderWidget.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Functional/Function.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final : private RenderWidget::Delegate
{
public:
    WindowBackend(EngineBackend* e, Window* w);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    RenderWidget* GetRenderWidget();
    void* GetHandle() const;
    WindowNativeService* GetNativeService() const;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();
    bool IsWindowReadyForRender() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();
    void InitCustomRenderParams(rhi::InitParam& params);

private:
    void PlatformEventHandler(const UIDispatcherEvent& e);
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    // RenderWidget::Delegate
    void OnCreated() override;
    void OnDestroyed() override;
    void OnFrame() override;
    void OnResized(uint32 width, uint32 height, float32 dpi) override;
    void OnVisibilityChanged(bool isVisible) override;

    void OnMousePressed(QMouseEvent* e) override;
    void OnMouseReleased(QMouseEvent* e) override;
    void OnMouseMove(QMouseEvent* e) override;
    void OnMouseDBClick(QMouseEvent* e) override;
    void OnWheel(QWheelEvent* e) override;
    void OnKeyPressed(QKeyEvent* e) override;
    void OnKeyReleased(QKeyEvent* e) override;

    uint32 ConvertButtons(Qt::MouseButton button);
#if defined(Q_OS_OSX)
    uint32 ConvertQtKeyToSystemScanCode(int key);
#endif

private:
    EngineBackend* engine = nullptr;
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;
    UIDispatcher platformDispatcher;
    std::unique_ptr<WindowNativeService> nativeService;
    RenderWidget* renderWidget = nullptr;

    class QtEventListener;
    QtEventListener* qtEventListener = nullptr;
};

inline void* WindowBackend::GetHandle() const
{
    return nullptr;
}

inline WindowNativeService* WindowBackend::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
