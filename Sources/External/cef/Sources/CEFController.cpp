#include <cef/include/cef_app.h>
#include <cef/include/cef_client.h>
#include "CEFDavaResourceHandler.h"

#include "Base/TypeHolders.h"
#include "Concurrency/Thread.h"
#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Signal.h"
#include "CEFController.h"

#include "Engine/EngineModule.h"

namespace DAVA
{
uint32 cacheSizeLimit = 50 * 1024 * 1024; // 50 MB default
RefPtr<class CEFControllerImpl> cefControllerGlobal;

//--------------------------------------------------------------------------------------------------
// CEF application delegate
// Adds custom url scheme to scheme registrar
//--------------------------------------------------------------------------------------------------
class CEFDavaApp : public CefApp
{
    IMPLEMENT_REFCOUNTING(CEFDavaApp);

    void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) override
    {
        registrar->AddCustomScheme("dava", false, false, false);
    }
};

//--------------------------------------------------------------------------------------------------
// CEF controller private implementation
//--------------------------------------------------------------------------------------------------
class CEFControllerImpl : public RefCounter
{
public:
    CEFControllerImpl();
    ~CEFControllerImpl();

    bool IsCEFAvailable();
    void AddClient(const CefRefPtr<CefClient>& client);

    FilePath GetCachePath();
    FilePath GetLogPath();

    void Update();
    void CleanLogs();
    void CleanCache();

private:
    void DoUpdate();

    List<CefRefPtr<CefClient>> clients;
    SigConnectionID appFinishedConnectionID = SigConnectionID();
    SigConnectionID updateConnectionID = SigConnectionID();
    bool isInitialized = false;
    bool schemeRegistered = false;
};

CEFControllerImpl::CEFControllerImpl()
{
    CleanLogs();

    CefSettings settings;
    settings.no_sandbox = 1;
    settings.windowless_rendering_enabled = 1;
    settings.log_severity = LOGSEVERITY_DISABLE;

    CefString(&settings.cache_path).FromString(GetCachePath().GetAbsolutePathname());
    CefString(&settings.log_file).FromString(GetLogPath().GetAbsolutePathname());
    CefString(&settings.browser_subprocess_path).FromASCII("CEFHelperProcess.exe");

    isInitialized = CefInitialize(CefMainArgs(), settings, new CEFDavaApp, nullptr);

    // CefInitialize replaces thread name, so we need to restore it
    // Restore name only on Main Thread
    if (isInitialized && Thread::IsMainThread())
    {
        Thread::SetCurrentThreadName(Thread::davaMainThreadName);
    }

    // Register custom url scheme for dava-based applications
    if (isInitialized)
    {
        CefRefPtr<CefSchemeHandlerFactory> davaFactory = new CEFDavaResourceHandlerFactory;
        schemeRegistered = CefRegisterSchemeHandlerFactory("dava", "", davaFactory);

        CefRefPtr<CefCookieManager> cookieMan = CefCookieManager::GetGlobalManager(nullptr);
        cookieMan->SetSupportedSchemes({ "dava" }, nullptr);
    }

    if (isInitialized && schemeRegistered)
    {
        auto shutdown = [] { cefControllerGlobal = nullptr; };
        auto update = [this](float32) { Update(); };
#if defined(__DAVAENGINE_COREV2__)
        Engine* e = Engine::Instance();
        updateConnectionID = e->update.Connect(update);
        appFinishedConnectionID = e->gameLoopStopped.Connect(shutdown);
#else
        Core* core = Core::Instance();
        appFinishedConnectionID = core->systemAppFinished.Connect(shutdown);
        updateConnectionID = core->updated.Connect(update);
#endif
    }
    else
    {
        Logger::Error("%s: cannot initialize CEF", __FUNCTION__);
        DVASSERT_MSG(false, "CEF cannot be initialized");
    }
}

CEFControllerImpl::~CEFControllerImpl()
{
    if (isInitialized)
    {
        if (schemeRegistered)
        {
#if defined(__DAVAENGINE_COREV2__)
            Engine* e = Engine::Instance();
            e->update.Disconnect(updateConnectionID);
            e->gameLoopStopped.Disconnect(appFinishedConnectionID);
#else
            Core* core = Core::Instance();
            core->systemAppFinished.Disconnect(appFinishedConnectionID);
            core->updated.Disconnect(updateConnectionID);
#endif
        }

        do
        {
            DoUpdate();
        } while (!clients.empty());

        CefShutdown();
        CleanCache();
    }
}

bool CEFControllerImpl::IsCEFAvailable()
{
    return isInitialized && schemeRegistered;
}

void CEFControllerImpl::AddClient(const CefRefPtr<CefClient>& client)
{
    clients.push_back(client);
}

FilePath CEFControllerImpl::GetCachePath()
{
    return "~doc:/cef_data/cache/";
}

FilePath CEFControllerImpl::GetLogPath()
{
    return "~doc:/cef_data/log.txt";
}

void CEFControllerImpl::Update()
{
    if (IsCEFAvailable() && !clients.empty())
    {
        DoUpdate();
    }
}

void CEFControllerImpl::DoUpdate()
{
    CefDoMessageLoopWork();

    // Check active clients
    clients.remove_if([](const CefRefPtr<CefClient>& client) { return client->HasOneRef(); });
}

void CEFControllerImpl::CleanLogs()
{
    FileSystem* fs = FileSystem::Instance();
    FilePath logPath = GetLogPath();

    if (fs->Exists(logPath))
    {
        fs->DeleteFile(logPath);
    }
}

void CEFControllerImpl::CleanCache()
{
    FileSystem* fs = FileSystem::Instance();
    FilePath cachePath = GetCachePath();
    Vector<FilePath> cacheDirContent = fs->EnumerateFilesInDirectory(cachePath);

    size_t cacheSize = 0;
    for (const FilePath& path : cacheDirContent)
    {
        uint32 size = 0;
        fs->GetFileSize(path, size);
        cacheSize += size;

        if (cacheSize > cacheSizeLimit)
        {
            break;
        }
    }

    if (cacheSize > cacheSizeLimit)
    {
        bool result = fs->DeleteDirectory(cachePath);

        // can't remove whole directory -> try to remove something
        if (!result)
        {
            for (const FilePath& path : cacheDirContent)
            {
                fs->DeleteFile(path);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
// CEF controller public implementation
//--------------------------------------------------------------------------------------------------
CEFController::CEFController(const CefRefPtr<CefClient>& client)
{
    if (!cefControllerGlobal)
    {
        cefControllerGlobal.Set(new CEFControllerImpl);
    }

    cefControllerImpl = cefControllerGlobal;
    if (IsCEFAvailable())
    {
        cefControllerImpl->AddClient(client);
    }
}

CEFController::~CEFController() = default;

bool CEFController::IsCEFAvailable()
{
    return cefControllerGlobal && cefControllerGlobal->IsCEFAvailable();
}

uint32 CEFController::GetCacheLimitSize()
{
    return cacheSizeLimit;
}

void CEFController::SetCacheLimitSize(uint32 size)
{
    cacheSizeLimit = size;
}

} // namespace DAVA
