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

#include "Network/PeerDesription.h"
#include "Network/Services/NetLogger.h"
#include "Network/Services/MMNetServer.h"

#include <fstream>

using namespace DAVA;

class TestData;
class BaseScreen;
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

    void RegisterScreen(BaseScreen *screen);
    
    void RegisterError(const String &command, const String &fileName, int32 line, TestData *testData);

    void LogMessage(const String &message);
    
protected:
    
    void RegisterTests();
    void RunTests();
    void ProcessTests();
    void FinishTests();

    String CreateOutputLogFile();
    String ReadLogFile();


    int32 TestCount();
    
    void CreateDocumentsFolder();
    File * CreateDocumentsFile(const String &filePathname);
    
private:
    void InitLogging();
    
    //////////////////////////////////
    enum eServiceTypes
    {
        SERVICE_LOG = 0,
        SERVICE_MEMPROF = 1
    };
    
    static const uint16 ANNOUNCE_PORT = 9999;
    static const uint32 ANNOUNCE_TIME_PERIOD = 5;
    static const char8 announceMulticastGroup[];
    
    static const uint16 LOGGER_PORT = 9999;
    
    void InitNetwork();
    
    size_t AnnounceDataSupplier(size_t length, void* buffer);
    
    Net::IChannelListener* CreateLogger(uint32 serviceId, void* context);
    void DeleteLogger(Net::IChannelListener* obj, void* context);
    
    Net::IChannelListener* CreateMMServer(uint32 serviceId, void* context);
    void DeleteMMServer(Net::IChannelListener* obj, void* context);

    //////////////////////////////////

    void RunOnlyThisTest();
    void OnError();
    bool IsNeedSkipTest(const BaseScreen& screen) const;

    String runOnlyThisTest;

    String logFilePath;
    std::ofstream logFile;

    BaseScreen *currentScreen;

    int32 currentScreenIndex;
    Vector<BaseScreen *> screens;
    
    int32 currentTestIndex;

    TeamcityTestsOutput teamCityOutput;
    
    Net::NetLogger netLogger;
    Net::MMNetServer netMM;
    Net::PeerDescription peerDescr;
    
    bool loggerInUse;
    bool mmInUse;
};



#endif // __GAMECORE_H__