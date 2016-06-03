#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_app.h>
#include "UI/Private/CEFDavaResourceHandler.h"

#include "Base/TypeHolders.h"
#include "Concurrency/Thread.h"
#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"
#include "UI/Private/CEFController.h"

namespace DAVA
{
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
        fs->DeleteFile(logPath);
    }
}

// Clean cache
void CEFControllerImpl::PostCleanUp()
{
    const size_t cacheSizeLimit = 150 * 1024 * 1024; // 150 MB
    FileSystem* fs = FileSystem::Instance();
    Vector<FilePath> cacheDirContent = fs->EnumerateFilesInDirectory(GetCachePath());

    size_t cacheSize = 0;
    for (const FilePath& path : cacheDirContent)
    {
        uint32 size = 0;
        fs->GetFileSize(path, size);
        cacheSize += size;
    }

    if (cacheSize > cacheSizeLimit)
    {
        fs->DeleteDirectory(GetCachePath());
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

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
