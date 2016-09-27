#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Qt/RenderWidget.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/WindowBackendBase.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final : public WindowBackendBase,
                            private RenderWidget::Delegate
{
public:
    WindowBackend(EngineBackend* engineBackend, Window* window);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    void Update();
    RenderWidget* GetRenderWidget();

    void Resize(float32 width, float32 height);
    void Close(bool appIsTerminating);
    void SetTitle(const String& title);

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
    void OnMouseDBClick(QMouseEvent* e) override;
    void OnWheel(QWheelEvent* e) override;
    void OnKeyPressed(QKeyEvent* e) override;
    void OnKeyReleased(QKeyEvent* e) override;

    uint32 ConvertButtons(Qt::MouseButton button);
#if defined(Q_OS_OSX)
    uint32 ConvertQtKeyToSystemScanCode(int key);
#endif

private:
    // TODO: may be keep RenderWidget in QPointer?
    RenderWidget* renderWidget = nullptr;
    std::unique_ptr<WindowNativeService> nativeService;

    bool closeRequestByApp = false;

    class QtEventListener;
    QtEventListener* qtEventListener = nullptr;

    class OGLContextBinder;
    friend void AcqureContext();
    friend void ReleaseContext();

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
