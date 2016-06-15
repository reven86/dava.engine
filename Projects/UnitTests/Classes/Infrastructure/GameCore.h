#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "Base/BaseTypes.h"
#include "Core/Core.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include "Network/NetCore.h"
#include "Network/Services/NetLogger.h"
#endif

#include "Logger/TeamCityTestsOutput.h"

class GameCore : public DAVA::ApplicationCore
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
    
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    void OnBackground() override
    {
    }
    void OnForeground() override;
    void OnDeviceLocked() override
    {
    }
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

#if defined(__DAVAENGINE_WIN_UAP__)
    void InitNetwork();
    void UnInitNetwork();
#endif

    void Update(DAVA::float32 update) override;

private:
    void ProcessCommandLine();
    void ProcessTests(DAVA::float32 timeElapsed);
    void FinishTests();

    void OnError();

    void OnTestClassStarted(const DAVA::String& testClassName);
    void OnTestClassFinished(const DAVA::String& testClassName);
    void OnTestClassDisabled(const DAVA::String& testClassName);
    void OnTestStarted(const DAVA::String& testClassName, const DAVA::String& testName);
    void OnTestFinished(const DAVA::String& testClassName, const DAVA::String& testName);
    void OnTestFailed(const DAVA::String& testClassName, const DAVA::String& testName, const DAVA::String& condition, const char* filename, int lineno, const DAVA::String& userMessage);

private:
    DAVA::TeamcityTestsOutput teamCityOutput;

#if defined(__DAVAENGINE_WIN_UAP__)

    class LogFlusher : public DAVA::LoggerOutput
    {
    public:
        LogFlusher(DAVA::Net::NetLogger* logger);
        ~LogFlusher();
        void FlushLogs();

    private:
        // LoggerOutput
        void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;
        DAVA::Net::NetLogger* netLogger = nullptr;
    };

    DAVA::Net::NetLogger netLogger;
    std::unique_ptr<LogFlusher> flusher;
    DAVA::Net::NetCore::TrackId netController;
    bool loggerInUse = false;
#endif
};

#endif // __GAMECORE_H__
