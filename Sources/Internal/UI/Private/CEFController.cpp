#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_app.h>
#include "UI/Private/CEFDavaResourceHandler.h"

#include "Base/TypeHolders.h"
#include "Concurrency/Thread.h"
#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "UI/Private/CEFController.h"

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
    void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) override
    {
        registrar->AddCustomScheme("dava", false, false, false);
    }
    IMPLEMENT_REFCOUNTING(CEFDavaApp)
};

//--------------------------------------------------------------------------------------------------
// CEF controller private implementation
//--------------------------------------------------------------------------------------------------
class CEFControllerImpl : public RefCounter
{
public:
    CEFControllerImpl();
    ~CEFControllerImpl();

    FilePath GetCachePath();
    FilePath GetLogPath();

    void Update();
    void PreCleanUp();
    void PostCleanUp();
};

CEFControllerImpl::CEFControllerImpl()
{
    Core::Instance()->systemAppFinished.Connect([] { cefControllerGlobal = nullptr; });
    PreCleanUp();

    CefSettings settings;
    settings.no_sandbox = 1;
    settings.windowless_rendering_enabled = 1;
    settings.log_severity = LOGSEVERITY_DISABLE;

    CefString(&settings.cache_path).FromString(GetCachePath().GetAbsolutePathname());
    CefString(&settings.log_file).FromString(GetLogPath().GetAbsolutePathname());
    CefString(&settings.browser_subprocess_path).FromASCII("CEFHelperProcess.exe");

    bool result = CefInitialize(CefMainArgs(), settings, new CEFDavaApp, nullptr);

    // CefInitialize replaces thread name, so we need to restore it
    // Restore name only on Main Thread
    if (result && Thread::IsMainThread())
    {
        Thread::SetCurrentThreadName(Thread::davaMainThreadName);
    }

    // Register custom url scheme for dava-based applications
    result |= CefRegisterSchemeHandlerFactory("dava", "", new CEFDavaResourceHandlerFactory);
    DVASSERT_MSG(result == true, "CEF cannot be initialized");
}

CEFControllerImpl::~CEFControllerImpl()
{
    Update();
    CefShutdown();
    PostCleanUp();
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
    CefDoMessageLoopWork();
}

// Remove
void CEFControllerImpl::PreCleanUp()
{
    FileSystem* fs = FileSystem::Instance();
    FilePath logPath = GetLogPath();

    if (fs->Exists(logPath))
    {
        fs->DeleteDirectory(logPath);
    }
}

// Clean cache
void CEFControllerImpl::PostCleanUp()
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
CEFController::CEFController()
{
    if (!cefControllerGlobal)
    {
        cefControllerGlobal.Set(new CEFControllerImpl);
    }
    cefControllerImpl = cefControllerGlobal;
}

CEFController::~CEFController() = default;

void CEFController::Update()
{
    cefControllerImpl->Update();
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

#endif // ENABLE_CEF_WEBVIEW
