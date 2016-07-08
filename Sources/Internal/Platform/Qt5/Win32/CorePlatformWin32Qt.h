#ifndef __DAVAENGINE_CORE_PLATFORM_WIN32_QT_H__
#define __DAVAENGINE_CORE_PLATFORM_WIN32_QT_H__

#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN32__)

//#include "Platform/TemplateWin32/CoreWin32PlatformBase.h"
#include "Core/Core.h"

namespace DAVA
{
class CoreWin32PlatformQt : public Core
{
public:
    void InitArgs();
    void Quit() override;
};
};

#endif // #if defined(__DAVAENGINE_WIN32__)
#endif // __DAVAENGINE_CORE_PLATFORM_WIN32_QT_H__