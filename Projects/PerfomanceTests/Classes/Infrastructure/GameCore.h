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

#include "DAVAEngine.h"
#include "TeamCityTestsOutput.h"

#include "Tests/PerfomanceTest.h"

#include <list>
#include <fstream>

using namespace DAVA;

class GameCore : public ApplicationCore
{
    struct ErrorData
    {
        int32 line;
        String command;
        String filename;
        String testName;
        String testMessage;
    };

protected:
    virtual ~GameCore();
public:    
    GameCore();

    static GameCore * Instance() 
    { 
        return (GameCore*) DAVA::Core::GetApplicationCore();
    };
    
    virtual void OnAppStarted() override;
    virtual void OnAppFinished() override;
    
    virtual void OnSuspend() override;
    virtual void OnResume() override;

#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    virtual void OnBackground();
    virtual void OnForeground();
    virtual void OnDeviceLocked();
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    virtual void BeginFrame() override;
    virtual void Update(DAVA::float32 update) override;
    virtual void Draw() override;
	virtual void EndFrame() override;


    void LogMessage(const String &message);
    
protected:
    
	void RegisterTests();
    String CreateOutputLogFile();
    String ReadLogFile();
    
    void CreateDocumentsFolder();
    File * CreateDocumentsFile(const String &filePathname);
    
private:
    void InitLogging();
	void CreateReportScreen();

	String runOnlyThisTest;
	bool showReport;

	uint32 currentTestIndex;
	BaseTest* currentTest;

	uint32 fixedFramesCount;
	float32 fixedDelta;

	uint32 fixedTime;

	Vector<BaseTest*> testList;

	String logFilePath;
	std::ofstream logFile;

	TeamcityTestsOutput teamCityOutput;
};



#endif // __GAMECORE_H__