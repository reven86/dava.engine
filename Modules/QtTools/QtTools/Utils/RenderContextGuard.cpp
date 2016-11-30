#include "RenderContextGuard.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Engine/WindowNativeService.h"

RenderContextGuard::RenderContextGuard()
{
    DAVA::Engine* engine = DAVA::Engine::Instance();
    DVASSERT(engine != nullptr);
    if (!engine->IsConsoleMode())
    {
        DAVA::Window* window = engine->PrimaryWindow();
        window->GetNativeService()->AcqureContext();
    }
}

RenderContextGuard::~RenderContextGuard()
{
    DAVA::Engine* engine = DAVA::Engine::Instance();
    DVASSERT(engine != nullptr);
    if (!engine->IsConsoleMode())
    {
        DAVA::Window* window = engine->PrimaryWindow();
        window->GetNativeService()->ReleaseContext();
    }
}

#endif