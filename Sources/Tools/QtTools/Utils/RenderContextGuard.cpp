#include "RenderContextGuard.h"

#include "Engine/Public/Engine.h"
#include "Engine/Public/Window.h"
#include "Engine/Public/WindowNativeService.h"

#if defined(__DAVAENGINE_COREV2__)

RenderContextGuard::RenderContextGuard()
{
    DAVA::Window* window = DAVA::Engine::Instance()->PrimaryWindow();
    window->GetNativeService()->AcqureContext();
}

RenderContextGuard::~RenderContextGuard()
{
    DAVA::Window* window = DAVA::Engine::Instance()->PrimaryWindow();
    window->GetNativeService()->ReleaseContext();
}

#endif