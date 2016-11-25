#pragma once

#if defined(__DAVAENGINE_COREV2__)

class RenderContextGuard
{
public:
    RenderContextGuard();
    ~RenderContextGuard();
};

#endif