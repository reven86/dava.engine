/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN32__)

#include "Platform/TemplateWin32/CorePlatformWin32.h"
#include "Concurrency/Thread.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"

#include <shellapi.h>

namespace DAVA
{

CoreWin32PlatformBase::CoreWin32PlatformBase()
{
}

void CoreWin32PlatformBase::InitArgs()
{
    int argc = 0;
    LPWSTR *szArglist = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    if (argc > 0 && NULL != szArglist)
    {
        Vector<String> args;
        args.reserve(argc);
        for (int i = 0; i < argc; ++i)
        {
            args.emplace_back(WStringToString(szArglist[i]));
        }

        SetCommandLine(std::move(args));
    }

    ::LocalFree(szArglist);
}

void CoreWin32PlatformBase::Quit()
{
    PostQuitMessage(0);
	exit(0);
}

void CoreWin32PlatformBase::SetCursorPosCenterInternal(HWND hWnd)
{
    RECT wndRect;
    GetWindowRect(hWnd, &wndRect);
    int centerX = (int)((wndRect.left + wndRect.right) >> 1);
    int centerY = (int)((wndRect.bottom + wndRect.top) >> 1);
    SetCursorPos(centerX, centerY);
}

void CoreWin32PlatformBase::SetCursorPositionCenter()
{
    SetCursorPosCenterInternal((HWND)GetNativeView());
}

void CoreWin32PlatformBase::SetCursorPosition(Point2i position)
{
    SetCursorPos(position.x, position.y);
}

Point2i CoreWin32PlatformBase::GetCursorPosition()
{
    POINT p;
    GetCursorPos(&p);
    return Point2i(p.x, p.y);
}

}

#endif // #if defined(__DAVAENGINE_WIN32__)
