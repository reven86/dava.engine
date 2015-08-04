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


#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "Core/ApplicationCore.h"
#include "Core/Core.h"

class TestData;
class BaseScreen;
class TestListScreen;
class GameCore : public DAVA::ApplicationCore
{
    struct ErrorData
    {
        DAVA::int32 line;
        DAVA::String command;
        DAVA::String filename;
        DAVA::String testName;
        DAVA::String testMessage;
    };

protected:
    virtual ~GameCore();
public:    
    GameCore();

    static GameCore * Instance() 
    { 
        return (GameCore*) DAVA::Core::GetApplicationCore();
    };
    
    void OnAppStarted() override;
    void OnAppFinished() override;

    void BeginFrame() override;

    void RegisterScreen(BaseScreen *screen);
    void ShowStartScreen();
    
protected:
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    virtual void OnBackground() {};
    
    virtual void OnForeground() {};
    
    virtual void OnDeviceLocked() {};
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    void RegisterTests();
    void RunTests();
    
    void CreateDocumentsFolder();
    DAVA::File * CreateDocumentsFile(const DAVA::String &filePathname);
    
private:
    void RunOnlyThisTest();
    void OnError();
    bool IsNeedSkipTest(const BaseScreen& screen) const;

    DAVA::String runOnlyThisTest;

    BaseScreen *currentScreen;
    TestListScreen *testListScreen;
    
    DAVA::Vector<BaseScreen *> screens;
};



#endif // __GAMECORE_H__