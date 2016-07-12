#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)

#include "Functional/Function.h"

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
class WindowBackend final
{
public:
    static WindowBackend* Create(Window* w);
    static void Destroy(WindowBackend* nativeWindow);

    void Resize(float32 width, float32 height);
    void* Handle() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

private:
    WindowBackend(Window* w);
    ~WindowBackend();

    bool CreateNativeWindow();

private:
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
