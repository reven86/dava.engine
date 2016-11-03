#pragma once

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
// Select right header for NativeService class depending on platfrom
#if defined(__DAVAENGINE_QT__)

class QApplication;
class RenderWidget;
class NativeService final
{
public:
    QApplication* GetApplication();
    RenderWidget* GetRenderWidget();

private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

#elif defined(__DAVAENGINE_WIN32__)

class NativeService final
{
public:
private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

#elif defined(__DAVAENGINE_WIN_UAP__)

class NativeService final
{
public:
private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

#elif defined(__DAVAENGINE_MACOS__)

class NativeService final
{
public:
private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

#elif defined(__DAVAENGINE_IPHONE__)

class NativeService final
{
public:
private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

#elif defined(__DAVAENGINE_ANDROID__)

class NativeService final
{
public:
private:
    NativeService(Private::PlatformCore* c);

private:
    Private::PlatformCore* core = nullptr;

    // Friends
    friend Private::PlatformCore;
};

#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation
#error "NativeService is not implemented"
#endif
#endif
}

#endif