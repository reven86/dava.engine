#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Engine/Private/EnginePrivateFwd.h"

class QApplication;
namespace DAVA
{

namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* engineBackend);
    ~PlatformCore();

    PlatformCore(const PlatformCore&) = delete;
    PlatformCore& operator=(const PlatformCore&) = delete;

    void Init();
    void Run();
    void Quit();

    QApplication* GetApplication();
    NativeService* GetNativeService();

private:
    WindowBackend* CreateNativeWindow(Window* w, float32 width, float32 height);

private:
    EngineBackend* engineBackend = nullptr;
    WindowBackend* windowBackend = nullptr;
    std::unique_ptr<NativeService> nativeService;
};

inline NativeService* PlatformCore::GetNativeService()
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
