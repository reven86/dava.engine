#pragma once

#if defined(__DAVAENGINE_MACOS__)
class MacOSRunLoopGuard
{
public:
    MacOSRunLoopGuard();
    ~MacOSRunLoopGuard();
};
#endif