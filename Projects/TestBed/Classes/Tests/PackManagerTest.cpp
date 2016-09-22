#include "Tests/PackManagerTest.h"
#include "Infrastructure/GameCore.h"

#include <Engine/Engine.h>
#include <UI/Focus/UIFocusComponent.h>
#include <PackManager/PackManager.h>
#include <FileSystem/DynamicMemoryFile.h>
#include <typeinfo>

using namespace DAVA;

PackManagerTest::PackManagerTest(GameCore* g)
    : BaseScreen(g, "PackManagerTest")
    , engine(*g->GetEngine())
{
}

void PackManagerTest::TextFieldOnTextChanged(UITextField* textField, const WideString& newText, const WideString& /*oldText*/)
{
    if (url == textField)
    {
        urlToServerSuperpack = UTF8Utils::EncodeToUTF8(newText);
        UpdateDescription();
    }
}

void PackManagerTest::UpdateDescription()
{
    String message = DAVA::Format("type name of pack you want to download\n"
                                  "Directory to downloaded packs: \"%s\"\nUrl to common packs: \"%s\"\n",
                                  folderWithDownloadedPacks.GetAbsolutePathname().c_str(),
                                  urlToServerSuperpack.c_str());
    description->SetText(UTF8Utils::EncodeToWideString(message));
}

void PackManagerTest::LoadResources()
{
    BaseScreen::LoadResources();

    eGPUFamily gpu = DeviceInfo::GetGPUFamily();
    switch (gpu)
    {
    case GPU_ADRENO:
        gpuArchitecture = "adreno";
        break;
    case GPU_DX11:
        gpuArchitecture = "dx11";
        break;
    case GPU_MALI:
        gpuArchitecture = "mali";
        break;
    case GPU_POWERVR_IOS:
        gpuArchitecture = "pvr_ios";
        break;
    case GPU_POWERVR_ANDROID:
        gpuArchitecture = "pvr_android";
        break;
    case GPU_TEGRA:
        gpuArchitecture = "tegra";
        break;
    default:
        throw std::runtime_error("unknown gpu famili");
    }

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    font->SetSize(14);

    packInput = new UITextField(Rect(5, 10, 400, 20));
    packInput->SetFont(font);
    packInput->SetText(L"allpacks");
    packInput->SetFontSize(14);
    packInput->SetDebugDraw(true);
    packInput->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    packInput->SetInputEnabled(true);
    packInput->GetOrCreateComponent<UIFocusComponent>();
    packInput->SetDelegate(this);
    packInput->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packInput);

    packNextInput = new UITextField(Rect(5, 40, 400, 20));
    packNextInput->SetFont(font);
    packNextInput->SetText(L"pack2");
    packNextInput->SetFontSize(14);
    packNextInput->SetDebugDraw(true);
    packNextInput->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    packNextInput->SetInputEnabled(true);
    packNextInput->GetOrCreateComponent<UIFocusComponent>();
    packNextInput->SetDelegate(this);
    packNextInput->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packNextInput);

    loadPack = new UIButton(Rect(420, 10, 100, 20));
    loadPack->SetDebugDraw(true);
    loadPack->SetStateFont(0xFF, font);
    loadPack->SetStateFontColor(0xFF, Color::White);
    loadPack->SetStateText(0xFF, L"start loading");
    loadPack->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartDownloadClicked));
    AddControl(loadPack);

    loadNext = new UIButton(Rect(420, 40, 100, 20));
    loadNext->SetDebugDraw(true);
    loadNext->SetStateFont(0xFF, font);
    loadNext->SetStateFontColor(0xFF, Color::White);
    loadNext->SetStateText(0xFF, L"next loading");
    loadNext->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartNextPackClicked));
    AddControl(loadNext);

    startServerButton = new UIButton(Rect(420, 70, 100, 20));
    startServerButton->SetDebugDraw(true);
    startServerButton->SetStateFont(0xFF, font);
    startServerButton->SetStateFontColor(0xFF, Color::White);
    startServerButton->SetStateText(0xFF, L"start server");
    startServerButton->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartStopLocalServerClicked));
    AddControl(startServerButton);

    stopServerButton = new UIButton(Rect(420, 100, 100, 20));
    stopServerButton->SetDebugDraw(true);
    stopServerButton->SetStateFont(0xFF, font);
    stopServerButton->SetStateFontColor(0xFF, Color::White);
    stopServerButton->SetStateText(0xFF, L"stop server");
    stopServerButton->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartStopLocalServerClicked));
    AddControl(stopServerButton);

    packNameLoading = new UIStaticText(Rect(5, 530, 500, 200));
    packNameLoading->SetFont(font);
    packNameLoading->SetTextColor(Color::White);
    packNameLoading->SetMultiline(true);
    packNameLoading->SetText(L"loading: ");
    packNameLoading->SetDebugDraw(true);
    packNameLoading->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(packNameLoading);

    redControl = new UIControl(Rect(5, 360, 500, 10));
    redControl->SetDebugDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    redControl->SetDebugDraw(true);
    AddControl(redControl);

    greenControl = new UIControl(Rect(5, 360, 0, 10));
    greenControl->SetDebugDrawColor(Color(0.f, 1.f, 0.f, 1.f));
    greenControl->SetDebugDraw(true);
    AddControl(greenControl);

    description = new UIStaticText(Rect(5, 70, 400, 200));
    description->SetFont(font);
    description->SetTextColor(Color::White);
    description->SetMultiline(true);
    description->SetDebugDraw(true);
    description->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    UpdateDescription();
    AddControl(description);

    url = new UITextField(Rect(5, 250, 400, 20));
    url->SetFont(font);
    url->SetFontSize(14);
    url->SetText(UTF8Utils::EncodeToWideString(urlToServerSuperpack));
    url->SetDebugDraw(true);
    url->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    url->SetInputEnabled(true);
    url->GetOrCreateComponent<UIFocusComponent>();
    url->SetDelegate(this);
    url->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(url);

    filePathField = new UITextField(Rect(5, 380, 400, 20));
    filePathField->SetFont(font);
    filePathField->SetFontSize(14);
    filePathField->SetText(UTF8Utils::EncodeToWideString("~res:/3d/LandscapeTest/landscapetest.sc2"));
    filePathField->SetDebugDraw(true);
    filePathField->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    filePathField->SetInputEnabled(true);
    filePathField->GetOrCreateComponent<UIFocusComponent>();
    filePathField->SetDelegate(this);
    filePathField->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(filePathField);

    checkFile = new UIButton(Rect(420, 380, 100, 20));
    checkFile->SetDebugDraw(true);
    checkFile->SetStateFont(0xFF, font);
    checkFile->SetStateFontColor(0xFF, Color::White);
    checkFile->SetStateText(0xFF, L"check file");
    checkFile->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnCheckFileClicked));
    AddControl(checkFile);

    startInit = new UIButton(Rect(420, 410, 100, 20));
    startInit->SetDebugDraw(true);
    startInit->SetStateFont(0xFF, font);
    startInit->SetStateFontColor(0xFF, Color::White);
    startInit->SetStateText(0xFF, L"PM init");
    startInit->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartInitClicked));
    AddControl(startInit);

    startSync = new UIButton(Rect(420, 440, 100, 20));
    startSync->SetDebugDraw(true);
    startSync->SetStateFont(0xFF, font);
    startSync->SetStateFontColor(0xFF, Color::White);
    startSync->SetStateText(0xFF, L"PM sync");
    startSync->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartSyncClicked));
    AddControl(startSync);

    clearDocs = new UIButton(Rect(420, 470, 100, 20));
    clearDocs->SetDebugDraw(true);
    clearDocs->SetStateFont(0xFF, font);
    clearDocs->SetStateFontColor(0xFF, Color::White);
    clearDocs->SetStateText(0xFF, L"rm dvpk's");
    clearDocs->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnClearDocsClicked));
    AddControl(clearDocs);

    lsDvpks = new UIButton(Rect(420, 500, 100, 20));
    lsDvpks->SetDebugDraw(true);
    lsDvpks->SetStateFont(0xFF, font);
    lsDvpks->SetStateFontColor(0xFF, Color::White);
    lsDvpks->SetStateText(0xFF, L"ls dvpk's");
    lsDvpks->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnListPacksClicked));
    AddControl(lsDvpks);

    dirToListFiles = new UITextField(Rect(5, 300, 400, 20));
    dirToListFiles->SetFont(font);
    dirToListFiles->SetFontSize(14);
    dirToListFiles->SetText(UTF8Utils::EncodeToWideString("~res:/3d/"));
    dirToListFiles->SetDebugDraw(true);
    dirToListFiles->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    dirToListFiles->SetInputEnabled(true);
    dirToListFiles->GetOrCreateComponent<UIFocusComponent>();
    dirToListFiles->SetDelegate(this);
    dirToListFiles->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(dirToListFiles);

    lsDirFromPacks = new UIButton(Rect(420, 300, 100, 20));
    lsDirFromPacks->SetDebugDraw(true);
    lsDirFromPacks->SetStateFont(0xFF, font);
    lsDirFromPacks->SetStateFontColor(0xFF, Color::White);
    lsDirFromPacks->SetStateText(0xFF, L"ls in dvpk");
    lsDirFromPacks->AddEvent(EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnListInDvpkClicked));
    AddControl(lsDirFromPacks);
}

void PackManagerTest::UnloadResources()
{
    SafeRelease(loadNext);
    SafeRelease(packNextInput);
    SafeRelease(lsDvpks);
    SafeRelease(startSync);
    SafeRelease(clearDocs);
    SafeRelease(packInput);
    SafeRelease(loadPack);
    SafeRelease(startServerButton);
    SafeRelease(stopServerButton);
    SafeRelease(packNameLoading);
    SafeRelease(redControl);
    SafeRelease(greenControl);
    SafeRelease(description);
    SafeRelease(url);
    SafeRelease(filePathField);
    SafeRelease(checkFile);
    SafeRelease(startInit);
    SafeRelease(dirToListFiles);
    SafeRelease(lsDirFromPacks);

    BaseScreen::UnloadResources();
}

void PackManagerTest::OnPackStateChange(const DAVA::IPackManager::Pack& pack)
{
    if (pack.state == IPackManager::Pack::Status::Mounted)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + pack.name + " done!"));
    }
    else if (pack.state == IPackManager::Pack::Status::ErrorLoading || pack.state == IPackManager::Pack::Status::OtherError)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("error: %s, %d, %s", pack.name.c_str(), pack.downloadError, pack.otherErrorMsg.c_str())));
    }
    else if (pack.state == IPackManager::Pack::Status::Downloading)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("downloading: %s %d", pack.name.c_str(), pack.downloadError,
                                                                            static_cast<int>(100 * pack.downloadProgress))));
    }
}

void PackManagerTest::OnPackDownloadChange(const DAVA::IPackManager::Pack& pack)
{
    packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + pack.name));
}

void PackManagerTest::OnRequestChange(const DAVA::IPackManager::IRequest& request)
{
    // change total download progress
    uint64 total = request.GetFullSizeWithDependencies();
    uint64 current = request.GetDownloadedSize();
    float32 progress = static_cast<float32>(current) / total;

    auto rect = redControl->GetRect();
    rect.dx = rect.dx * progress;
    greenControl->SetRect(rect);
}

void PackManagerTest::OnInitChange(IPackManager& packManager)
{
    // To visualise on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading
    StringStream ss;

    ss << "init change state: " << static_cast<uint32>(packManager.GetInitState()) << '\n';

    if (packManager.GetInitError() != IPackManager::InitError::AllGood)
    {
        ss << "error: " << static_cast<uint32>(packManager.GetInitError()) << " message: " << packManager.GetInitErrorMessage() << '\n';

        if (packManager.CanRetryInit())
        {
            ss << "do you want to retry?\n";
            packManager.PauseInit(); // wait for user decide what to do! User can - PackManager.GetInit().Retry()
        }
    }

    Logger::Info("%s", ss.str().c_str());

    packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + ss.str()));
}

void PackManagerTest::OnStartInitClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
#if defined(__DAVAENGINE_COREV2__)
    IPackManager& pm = *engine.GetContext()->packManager;
#else
    IPackManager& pm = Core::Instance()->GetPackManager();
#endif

    if (pm.IsRequestingEnabled())
    {
        return;
    }
    // do every time full reinitialization
    FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks);

    packNameLoading->SetText(L"done: start init");

    pm.asyncConnectStateChanged.Connect(this, &PackManagerTest::OnInitChange);

    String dbFile = sqliteDbFile;
    dbFile.replace(dbFile.find("{gpu}"), 5, gpuArchitecture);

    pm.Initialize(gpuArchitecture, folderWithDownloadedPacks, dbFile, urlToServerSuperpack, IPackManager::Hints());

    pm.EnableRequesting();

    packNameLoading->SetText(L"done: finish init");
}

void PackManagerTest::OnStartSyncClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    /*
    packNameLoading->SetText(L"done: start sync");
    IPackManager& pm = Core::Instance()->GetPackManager();
    */
}

void PackManagerTest::OnClearDocsClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
#if defined(__DAVAENGINE_COREV2__)
    IPackManager& pm = *engine.GetContext()->packManager;
#else
    IPackManager& pm = Core::Instance()->GetPackManager();
#endif
    const Vector<IPackManager::Pack>& packs = pm.GetPacks();

    std::for_each(begin(packs), end(packs), [&pm](const IPackManager::Pack& pack)
                  {
                      if (pack.state == IPackManager::Pack::Status::Mounted && pack.isReadOnly == false)
                      {
                          pm.DeletePack(pack.name);
                      }
                  });

    FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks, true);
    FileSystem::Instance()->CreateDirectory(folderWithDownloadedPacks, true);

    packNameLoading->SetText(L"done: unmount all dvpk's, and remove dir with downloaded dvpk's");
}

void PackManagerTest::OnListPacksClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
#if defined(__DAVAENGINE_COREV2__)
    IPackManager& pm = *engine.GetContext()->packManager;
#else
    IPackManager& pm = Core::Instance()->GetPackManager();
#endif

    std::stringstream ss;

    for (auto& pack : pm.GetPacks())
    {
        if (pack.state == IPackManager::Pack::Status::Mounted)
        {
            ss << pack.name << ", ";
        }
    }

    String s = ss.str();
    if (!s.empty())
    {
        s = s.substr(0, s.size() - 2);
    }
    packNameLoading->SetText(UTF8Utils::EncodeToWideString(s));
}

void PackManagerTest::OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    // To visualise on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading

#if defined(__DAVAENGINE_COREV2__)
    IPackManager& pm = *engine.GetContext()->packManager;
#else
    IPackManager& pm = Core::Instance()->GetPackManager();
#endif

    if (pm.GetInitState() < IPackManager::InitState::MountingLocalPacks)
    {
        return;
    }

    pm.packStateChanged.DisconnectAll();

    pm.packStateChanged.Connect(this, &PackManagerTest::OnPackStateChange);
    pm.requestProgressChanged.Connect(this, &PackManagerTest::OnRequestChange);

    WideString packName = packInput->GetText();

    try
    {
        packNameLoading->SetText(L"loading: " + packName);
        pm.RequestPack(UTF8Utils::EncodeToUTF8(packName));
    }
    catch (std::exception& ex)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString(ex.what()));
    }
}

void PackManagerTest::OnStartNextPackClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
#if defined(__DAVAENGINE_COREV2__)
    IPackManager& pm = *engine.GetContext()->packManager;
#else
    IPackManager& pm = Core::Instance()->GetPackManager();
#endif
    WideString packName = packNextInput->GetText();

    pm.packStateChanged.DisconnectAll();

    pm.packStateChanged.Connect(this, &PackManagerTest::OnPackStateChange);
    pm.requestProgressChanged.Connect(this, &PackManagerTest::OnRequestChange);

    try
    {
        packNameLoading->SetText(L"loading: " + packName);
        String pName = UTF8Utils::EncodeToUTF8(packName);
        pm.RequestPack(pName);
        pm.SetRequestOrder(pName, 0.f);
    }
    catch (std::exception& ex)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString(ex.what()));
    }
}

void PackManagerTest::OnStartStopLocalServerClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    if (sender == startServerButton)
    {
        if (gpuArchitecture == "dx11")
        {
#ifdef __DAVAENGINE_MACOS__
            String macExec = "open -a /usr/bin/osascript --args -e 'tell application \"Terminal\" to do script \"";
            String cdCommand = "cd " + FilePath("~res:/").GetAbsolutePathname() + "..; ";
            String serverCommand = "python scripts/start_local_http_server.py";
            String cdAndPyCommand = cdCommand + serverCommand;
            macExec += cdAndPyCommand + "\"'";
            int result = std::system(macExec.c_str());
            if (result != 0)
            {
                Logger::Debug("start local server return: %d, return code: %d, stop sig: %d",
                              result, WEXITSTATUS(result), WSTOPSIG(result));
                auto fs = FileSystem::Instance();
                Logger::Debug("CWD: %s", fs->GetCurrentWorkingDirectory().GetAbsolutePathname().c_str());
                Logger::Debug("APP_DIR: %s", fs->GetCurrentExecutableDirectory().GetAbsolutePathname().c_str());
                Logger::Debug("DATA_DIR: %s", FilePath("~res:/").GetAbsolutePathname().c_str());
                Logger::Debug("COMMAND: %s", macExec.c_str());
            }
#elif defined(__DAVAENGINE_WIN32__)
            std::system("python scripts/start_local_http_server.py");
#endif
        }
    }
    else if (sender == stopServerButton)
    {
        if (gpuArchitecture == "dx11")
        {
#ifdef __DAVAENGINE_MACOS__
            String cdAndPyCommand = "cd " + FilePath("~res:/").GetAbsolutePathname() + "..; python scripts/stop_local_http_server.py";
            std::system(cdAndPyCommand.c_str());
            String closeTerminalCommand = "osascript -e 'tell application \"Terminal\" to quit'";
            std::system(closeTerminalCommand.c_str());
#elif defined(__DAVAENGINE_WIN32__)
            std::system("python scripts/stop_local_http_server.py");
#endif
        }
    }
}

void PackManagerTest::OnCheckFileClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DAVA::WideString text = filePathField->GetText();
    DAVA::String fileName = UTF8Utils::EncodeToUTF8(text);

    FilePath path(fileName);

    ScopedPtr<File> f(File::Create(path, File::OPEN | File::READ));
    // if we read file from pack - it will be DynamicMemoryFile
    if (f && typeid(*f) == typeid(DynamicMemoryFile))
    {
        packNameLoading->SetText(L"file loaded successfully");
    }
    else
    {
        packNameLoading->SetText(L"can't load file");
    }
}

void PackManagerTest::OnListInDvpkClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    WideString text = dirToListFiles->GetText();
    FilePath path(text);

    ScopedPtr<FileList> fileList(new FileList(path));

    StringStream ss;

    for (uint32 i = 0; i < fileList->GetCount(); ++i)
    {
        const FilePath& nextPath = fileList->GetPathname(i);
        ss << nextPath.GetStringValue() << '\n';
    }

    WideString out(UTF8Utils::EncodeToWideString(ss.str()));

    packNameLoading->SetText(out);
}
