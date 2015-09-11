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


#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Render/Cursor.h"
#include "Platform/TemplateWin32/CoreWin32PlatformBase.h"
#include "FileSystem/FileSystem.h"

void DAVA::Cursor::SetCursorPinning(bool pin)
{
    static DAVA::Point2i lastCursorPosition;

    ShowSystemCursor(!pin);

    CoreWin32PlatformBase * winCore = static_cast<CoreWin32PlatformBase *>(Core::Instance());
    if (pin)
    {
        lastCursorPosition = winCore->GetCursorPosition();
        winCore->SetCursorPositionCenter();
    }
    else
    {
        winCore->SetCursorPosition(lastCursorPosition);
    }
}

void DAVA::Cursor::ShowSystemCursor( bool show )
{
    CURSORINFO ci = { sizeof( ci ), 0 };
    if ( GetCursorInfo( &ci ) != 0 )
    {
        const auto isVisible = ( ci.flags & CURSOR_SHOWING ) == CURSOR_SHOWING; // In Windows 8 will be added other flags
        if ( show != isVisible )
        {
            ShowCursor( show );
        }
    }
    else
    {
        ShowCursor( show ); // No cursor info available, just call
    }
}

#endif
