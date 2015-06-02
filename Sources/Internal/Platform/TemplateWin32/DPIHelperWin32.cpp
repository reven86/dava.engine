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


#include "Platform/DPIHelper.h"


namespace DAVA
{
	uint32 DPIHelper::GetScreenDPI()
	{
		HDC screen = GetDC(NULL);

		// in common dpi is the same in horizontal and vertical demensions
		// in any case under win this value is 96dpi due to OS limitation
		uint32 hDPI = GetDeviceCaps(screen, LOGPIXELSX);
		ReleaseDC(NULL, screen);
		return hDPI;
	}

    float64 DPIHelper::GetDpiScaleFactor(int32 screenId)
    {
        return 1.0;
    }

    Size2i DPIHelper::GetScreenSize()
    {
        Size2i screenSize;
        HDC screen = GetDC(NULL);
        screenSize.dx = GetDeviceCaps(screen, HORZRES);
        screenSize.dy = GetDeviceCaps(screen, VERTRES);
        ReleaseDC(NULL, screen);
        return screenSize;
    }
}
