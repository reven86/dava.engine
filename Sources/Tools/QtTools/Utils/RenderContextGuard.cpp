#include "RenderContextGuard.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/WindowNativeService.h"

RenderContextGuard::RenderContextGuard()
{
#if defined(__DAVAENGINE_QT__)
    DAVA::Window* window = DAVA::Engine::Instance()->PrimaryWindow();
    window->GetNativeService()->AcqureContext();
#endif
}

RenderContextGuard::~RenderContextGuard()
{
#if defined(__DAVAENGINE_QT__)
    DAVA::Window* window = DAVA::Engine::Instance()->PrimaryWindow();
    window->GetNativeService()->ReleaseContext();
#endif
}

#endif