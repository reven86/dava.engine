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

#include "Base/BaseTypes.h"
#include "Core/Core.h"

#include "Infrastructure/TeamCityTestsOutput.h"

#include <fstream>

namespace Testing
{
    class TestClass;
}

using namespace DAVA;

class GameCore : public ApplicationCore
{
protected:
    virtual ~GameCore() = default;

public:    
    GameCore() = default;

    static GameCore* Instance()
    { 
        return static_cast<GameCore*>(DAVA::Core::GetApplicationCore());
    };
    
    void OnAppStarted() override;
    void OnAppFinished() override;

    void OnSuspend() override;
    void OnResume() override;
    
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    void OnBackground() override;
    void OnForeground() override;
    void OnDeviceLocked() override;
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    void Update(DAVA::float32 update) override;

    void RegisterError(const String &command, const String &fileName, int32 line, const String& userMessage);
    void LogMessage(const String &message);

protected:
    void ProcessTests(float32 timeElapsed);
    void FinishTests();

    String CreateOutputLogFile();
    String ReadLogFile();

    void CreateDocumentsFolder();
    File* CreateDocumentsFile(const String &filePathname);
    
private:
    void InitLogging();

    void RunOnlyThisTest();
    void OnError();

    String runOnlyThisTest;

    String logFilePath;
    std::ofstream logFile;

    TeamcityTestsOutput teamCityOutput;

    Testing::TestClass* curTestClass = nullptr;
    String curTestClassName;
    String curTestName;
    size_t curTestClassIndex = 0;
    size_t curTestIndex = 0;
    bool testSetUpInvoked = false;
};

#define TEST_VERIFY(condition)                                                                          \
    if (!(condition))                                                                                   \
    {                                                                                                   \
        GameCore::Instance()->RegisterError(String(#condition), __FILE__, __LINE__, DAVA::String());    \
    }

#define TEST_VERIFY_WITH_MESSAGE(condition, message)                                                        \
    if (!(condition))                                                                                       \
    {                                                                                                       \
        GameCore::Instance()->RegisterError(String(#condition), __FILE__, __LINE__, DAVA::String(message)); \
    }

#endif // __GAMECORE_H__
