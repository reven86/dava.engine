#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Qt/RenderWidget.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"
#include "Functional/SignalBase.h"

#include <QPointer>

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final : public TrackedObject, private RenderWidget::IWindowDelegate
{
public:
    WindowBackend(EngineBackend* engineBackend, Window* window);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    void AcqureContext();
    void ReleaseContext();
    void OnApplicationFocusChanged(bool isInFocus);

    void Update();
    void ActivateRendering();
    RenderWidget* GetRenderWidget();

    void Resize(float32 width, float32 height);
    void Close(bool appIsTerminating);
    void SetTitle(const String& title);

    void RunAsyncOnUIThread(const Function<void()>& task);

    void* GetHandle() const;
    WindowNativeService* GetNativeService() const;

    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void TriggerPlatformEvents();

private:
    void UIEventHandler(const UIDispatcherEvent& e);
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();
    void DoSetTitle(const char8* title);

    // RenderWidget::Delegate
    void OnCreated() override;
    bool OnUserCloseRequest() override;
    void OnDestroyed() override;
    void OnFrame() override;
    void OnResized(uint32 width, uint32 height, float32 dpi) override;
    void OnVisibilityChanged(bool isVisible) override;

    void OnMousePressed(QMouseEvent* e) override;
    void OnMouseReleased(QMouseEvent* e) override;
    void OnMouseMove(QMouseEvent* e) override;
    void OnDragMoved(QDragMoveEvent* e) override;
    void OnMouseDBClick(QMouseEvent* e) override;
    void OnWheel(QWheelEvent* e) override;
    void OnKeyPressed(QKeyEvent* e) override;
    void OnKeyReleased(QKeyEvent* e) override;

    uint32 ConvertButtons(Qt::MouseButton button);
#if defined(Q_OS_OSX)
    uint32 ConvertQtKeyToSystemScanCode(int key);
#endif

private:
    EngineBackend* engineBackend = nullptr;
    Window* window = nullptr; // Window frontend reference
    MainDispatcher* mainDispatcher = nullptr; // Dispatcher that dispatches events to DAVA main thread
    UIDispatcher uiDispatcher; // Dispatcher that dispatches events to window UI thread

    // Use QPointer as renderWidget can be deleted outside WindowBackend in embedded mode
    QPointer<RenderWidget> renderWidget;
    std::unique_ptr<WindowNativeService> nativeService;

    bool closeRequestByApp = false;

    class QtEventListener;
    QtEventListener* qtEventListener = nullptr;

    class OGLContextBinder;
    friend void AcqureContextImpl();
    friend void ReleaseContextImpl();

    std::unique_ptr<OGLContextBinder> contextBinder;
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
