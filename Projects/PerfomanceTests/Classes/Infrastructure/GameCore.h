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
#include "Platform/DeviceInfo.h"

#include "Infrastructure/Controller/TestFlowController.h"
#include "Infrastructure/Controller/TestChainFlowController.h"
#include "Infrastructure/Controller/SingleTestFlowController.h"
#include "Infrastructure/Settings/GraphicsDetect.h"

#include <fstream>

class GameCore : public ApplicationCore
{
public:    
    GameCore();

    static GameCore * Instance() 
    { 
        return (GameCore*) DAVA::Core::GetApplicationCore();
    };
    
    void OnAppStarted() override;
    void OnAppFinished() override;
    
    void OnSuspend() override;
    void OnResume() override;
    
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    void OnBackground() override;
    void OnForeground() override;
    void OnDeviceLocked() override;
#endif

    void BeginFrame() override;
	void EndFrame() override;
    
private:
    void InitScreenController();
	void RegisterTests();
    void ReadSingleTestParams(BaseTest::TestParams& params);
    void LoadMaps(const String& testName, Vector<std::pair<String, String>>& maps);
    
    String GetDeviceName();

	Vector<BaseTest*> testChain;
    std::unique_ptr<TestFlowController> testFlowController;

    TeamcityTestsOutput teamCityOutput;
    BaseTest::TestParams defaultTestParams;
};



#endif // __GAMECORE_H__