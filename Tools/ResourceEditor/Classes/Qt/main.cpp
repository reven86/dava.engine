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



#include <QtGui/QApplication>
#include "Main/mainwindow.h"
#include "Project/ProjectManager.h"
#include "DAVAEngine.h"

#if defined (__DAVAENGINE_MACOS__)
#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
#include "Platform/Qt/Win32/QtLayerWin32.h"
#include "Platform/Qt/Win32/CorePlatformWin32.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

#include "../CommandLine/CommandLineManager.h"

int main(int argc, char *argv[])
{
	int ret = 0;
    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
	new CommandLineManager();
	new DAVA::QtLayerMacOS();
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
	new CommandLineManager();
	new DAVA::QtLayerWin32();
#else
	DVASSERT(false && "Wrong platform")
#endif

    bool needInvalidateTimer = (CommandLineManager::Instance()->IsCommandLineModeEnabled() == false);
    new QtMainWindow(needInvalidateTimer);

    bool needToQuit = false;
    if(CommandLineManager::Instance()->IsCommandLineModeEnabled())
    {
        if(CommandLineManager::Instance()->IsToolInitialized())
        {
            CommandLineManager::Instance()->Process();
            CommandLineManager::Instance()->PrintResults();
            needToQuit = CommandLineManager::Instance()->NeedCloseApplication();
        }
        else
        {
            CommandLineManager::Instance()->PrintResults();
            CommandLineManager::Instance()->PrintUsageForActiveTool();
            needToQuit = true;
        }
    }
    
    if(!needToQuit)
    {
        QtMainWindow::Instance()->show();
		ProjectManager::Instance()->ProjectOpenLast();

        ret = a.exec();
    }

	QtMainWindow::Instance()->Release();
	CommandLineManager::Instance()->Release();

	DAVA::QtLayer::Instance()->Release();
	DAVA::Core::Instance()->ReleaseSingletons();
	DAVA::Core::Instance()->Release();

	return ret;
}
