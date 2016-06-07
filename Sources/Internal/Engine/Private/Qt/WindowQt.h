#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

#include "Engine/Private/EngineFwd.h"

#if defined(__DAVAENGINE_QT__)

namespace DAVA
{
class Window;

namespace Private
{
class WindowQt final
{
public:
    static WindowQt* Create(Window* w);
    static void Destroy(WindowQt* nativeWindow);

    void Resize(float32 width, float32 height);
    void* Handle() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

private:
    WindowQt(Window* w);
    ~WindowQt();

    bool CreateNativeWindow();

private:
    Dispatcher* dispatcher = nullptr;
    Window* window = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_QT__
#endif // __DAVAENGINE_COREV2__
