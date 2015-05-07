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

#include "MainWindow.h"

#include "DAVAEngine.h"
#include "QtTools/FrameworkBinding/FrameworkLoop.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"

void RunGui(int argc, char *argv[]);

int main(int argc, char *argv[])
{
#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
#elif defined (__DAVAENGINE_WIN32__)
    HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(nullptr);
    DAVA::Core::Run(argc, argv, hInstance);
#else
    DVASSERT(false && "Wrong platform")
#endif

    RunGui(argc, argv);

    return 0;
}

void RunGui(int argc, char *argv[])
{
    new DAVA::QtLayer();

#ifdef Q_OS_MAC
    // Must be called before creating QApplication instance
    DAVA::QtLayer::MakeAppForeground(false);
    QTimer::singleShot(0, []{DAVA::QtLayer::MakeAppForeground();});
    QTimer::singleShot(0, []{DAVA::QtLayer::RestoreMenuBar(););
#endif

    new DavaLoop();
    new FrameworkLoop();

    QApplication a(argc, argv);

    MainWindow *w = new MainWindow();
    w->show();

    DavaLoop::Instance()->StartLoop(FrameworkLoop::Instance());

    QApplication::exec();

    FrameworkLoop::Instance()->Release();
    DAVA::QtLayer::Instance()->Release();
    DavaLoop::Instance()->Release();

    delete w;
}
