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


#include <QApplication>
#include "UI/mainwindow.h"

#include "DAVAEngine.h"

#if defined (__DAVAENGINE_MACOS__)
#include "Platform/Qt5/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
#include "Platform/Qt5/Win32/QtLayerWin32.h"
#endif

//#include "GeneratedFiles/MocSourcesHeader.h"
//#include "GeneratedFiles/QrcSourcesHeader.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
	new DAVA::QtLayerMacOS();
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
	new DAVA::QtLayerWin32();
#else
	DVASSERT(false && "Wrong platform")
#endif

    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
    int result = 0;
    // MainWindow have to be released prior to the framework, so use separate scope for it.
    {
        MainWindow w;
        w.show();
        result = a.exec();
    }

#if defined (__DAVAENGINE_MACOS__)
    DAVA::QtLayerMacOS::Instance()->Release();
#elif defined (__DAVAENGINE_WIN32__)
    DAVA::QtLayerWin32::Instance()->Release();
#else
	DVASSERT(false && "Wrong platform")
#endif

    return result;
}
