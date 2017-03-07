#include "Tests/DLCManagerTest.h"
#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <FileSystem/DynamicMemoryFile.h>
#include <DLCManager/DLCManager.h>
#include <UI/Focus/UIFocusComponent.h>

using namespace DAVA;

DLCManagerTest::DLCManagerTest(TestBed& app)
    : BaseScreen(app, "DLCManagerTest")
    , engine(app.GetEngine())
{
}

DLCManagerTest::~DLCManagerTest()
{
    DLCManager& dm = *engine.GetContext()->dlcManager;

    dm.requestUpdated.DisconnectAll();
    dm.networkReady.DisconnectAll();
}

void DLCManagerTest::TextFieldOnTextChanged(UITextField* textField, const WideString& newText, const WideString& /*oldText*/)
{
    if (url == textField)
    {
        urlToServerSuperpack = UTF8Utils::EncodeToUTF8(newText);
        UpdateDescription();
    }
}

void DLCManagerTest::UpdateDescription()
{
    String message = DAVA::Format("type name of pack you want to download\n"
                                  "Directory to downloaded packs: \"%s\"\nUrl to common packs: \"%s\"\n",
                                  folderWithDownloadedPacks.GetAbsolutePathname().c_str(),
                                  urlToServerSuperpack.c_str());
    description->SetText(UTF8Utils::EncodeToWideString(message));
}

void DLCManagerTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    font->SetSize(14);

    packInput = new UITextField(Rect(5, 10, 400, 20));
    packInput->SetFont(font);
    packInput->SetText(L"0");
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
    packNextInput->SetText(L"1");
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
    loadPack->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartDownloadClicked));
    AddControl(loadPack);

    loadNext = new UIButton(Rect(420, 40, 100, 20));
    loadNext->SetDebugDraw(true);
    loadNext->SetStateFont(0xFF, font);
    loadNext->SetStateFontColor(0xFF, Color::White);
    loadNext->SetStateText(0xFF, L"next loading");
    loadNext->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartNextPackClicked));
    AddControl(loadNext);

    startServerButton = new UIButton(Rect(420, 70, 100, 20));
    startServerButton->SetDebugDraw(true);
    startServerButton->SetStateFont(0xFF, font);
    startServerButton->SetStateFontColor(0xFF, Color::White);
    startServerButton->SetStateText(0xFF, L"start server");
    startServerButton->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartStopLocalServerClicked));
    AddControl(startServerButton);

    stopServerButton = new UIButton(Rect(420, 100, 100, 20));
    stopServerButton->SetDebugDraw(true);
    stopServerButton->SetStateFont(0xFF, font);
    stopServerButton->SetStateFontColor(0xFF, Color::White);
    stopServerButton->SetStateText(0xFF, L"stop server");
    stopServerButton->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartStopLocalServerClicked));
    AddControl(stopServerButton);

    packNameLoading = new UIStaticText(Rect(5, 530, 500, 200));
    packNameLoading->SetFont(font);
    packNameLoading->SetTextColor(Color::White);
    packNameLoading->SetMultiline(true);
    packNameLoading->SetText(L"loading: ");
    packNameLoading->SetDebugDraw(true);
    packNameLoading->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(packNameLoading);

    logPring = new UIStaticText(Rect(540, 530, 500, 200));
    logPring->SetFont(font);
    logPring->SetTextColor(Color::White);
    logPring->SetMultiline(true);
    logPring->SetUtf8Text("");
    logPring->SetDebugDraw(true);
    logPring->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(logPring);

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
    checkFile->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnCheckFileClicked));
    AddControl(checkFile);

    startInit = new UIButton(Rect(420, 410, 100, 20));
    startInit->SetDebugDraw(true);
    startInit->SetStateFont(0xFF, font);
    startInit->SetStateFontColor(0xFF, Color::White);
    startInit->SetStateText(0xFF, L"PM init");
    startInit->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartInitClicked));
    AddControl(startInit);

    startSync = new UIButton(Rect(420, 440, 100, 20));
    startSync->SetDebugDraw(true);
    startSync->SetStateFont(0xFF, font);
    startSync->SetStateFontColor(0xFF, Color::White);
    startSync->SetStateText(0xFF, L"PM sync");
    startSync->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartSyncClicked));
    AddControl(startSync);

    clearDocs = new UIButton(Rect(420, 470, 100, 20));
    clearDocs->SetDebugDraw(true);
    clearDocs->SetStateFont(0xFF, font);
    clearDocs->SetStateFontColor(0xFF, Color::White);
    clearDocs->SetStateText(0xFF, L"rm dvpk's");
    clearDocs->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnClearDocsClicked));
    AddControl(clearDocs);

    lsDvpks = new UIButton(Rect(420, 500, 100, 20));
    lsDvpks->SetDebugDraw(true);
    lsDvpks->SetStateFont(0xFF, font);
    lsDvpks->SetStateFontColor(0xFF, Color::White);
    lsDvpks->SetStateText(0xFF, L"ls dvpk's");
    lsDvpks->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnListPacksClicked));
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
    lsDirFromPacks->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnListInDvpkClicked));
    AddControl(lsDirFromPacks);
}

void DLCManagerTest::UnloadResources()
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
    SafeRelease(logPring);
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

void DLCManagerTest::WriteErrorOnDevice(const String& filePath, int32 errnoVal)
{
    StringStream ss(logPring->GetUtf8Text());
    ss << filePath << std::endl;
    logPring->SetUtf8Text(ss.str());
}

void DLCManagerTest::OnRequestUpdated(const DAVA::DLCManager::IRequest& request)
{
    const String& packName = request.GetRequestedPackName();
    // change total download progress
    uint64 total = request.GetSize();
    uint64 current = request.GetDownloadedSize();
    float32 progress = static_cast<float32>(current) / total;

    std::stringstream ss;
    ss << "downloading: " << packName << " : " << current << "/" << total << " (" << (progress * 100) << ")%";

    if (request.IsDownloaded())
    {
        ss << " DOWNLOADED!!!";
    }

    packNameLoading->SetUtf8Text(ss.str());

    auto rect = redControl->GetRect();
    rect.dx = rect.dx * progress;
    greenControl->SetRect(rect);
}

void DLCManagerTest::OnNetworkReady(bool isReady)
{
    // To visualise on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading
    std::stringstream ss;
    const char* boolName = isReady ? "True" : "False";
    ss << "nerwork ready = " << boolName;
    Logger::Info("%s", ss.str().c_str());

    packNameLoading->SetUtf8Text("loading: " + ss.str());
}

void DLCManagerTest::OnInitializeFinished(size_t numDownloaded, size_t numTotalFiles)
{
    std::stringstream ss;
    ss << "initialize finished: num_downloaded = " << numDownloaded << " num_total = " << numTotalFiles << std::endl;
    packNameLoading->SetUtf8Text(ss.str());
}

void DLCManagerTest::OnStartInitClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& dm = *engine.GetContext()->dlcManager;

    packNameLoading->SetText(L"done: start init");

    dm.networkReady.DisconnectAll();
    dm.networkReady.Connect(this, &DLCManagerTest::OnNetworkReady);
    dm.initializeFinished.Connect(this, &DLCManagerTest::OnInitializeFinished);

    dm.Initialize(folderWithDownloadedPacks, urlToServerSuperpack, DLCManager::Hints());

    dm.SetRequestingEnabled(true);

    packNameLoading->SetText(L"done: start initialize PackManager");
}

void DLCManagerTest::OnStartSyncClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    /*
    packNameLoading->SetText(L"done: start sync");
    IPackManager& pm = Core::Instance()->GetPackManager();
    */
}

void DLCManagerTest::OnClearDocsClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& dm = *engine.GetContext()->dlcManager;

    packNameLoading->SetText(L"done: unmount all dvpk's, and remove dir with downloaded dvpk's");
}

void DLCManagerTest::OnListPacksClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& dm = *engine.GetContext()->dlcManager;
    std::stringstream ss;

    // TODO do I need list loaded packs?

    String s = ss.str();
    if (!s.empty())
    {
        s = s.substr(0, s.size() - 2);
    }
    packNameLoading->SetText(UTF8Utils::EncodeToWideString(s));
}

void DLCManagerTest::OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    // To visualise on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading

    DLCManager& dm = *engine.GetContext()->dlcManager;

    dm.requestUpdated.DisconnectAll();
    dm.requestUpdated.Connect(this, &DLCManagerTest::OnRequestUpdated);
    dm.cantWriteToDisk.Connect(this, &DLCManagerTest::WriteErrorOnDevice);

    String packName = packInput->GetUtf8Text();

    try
    {
        if (dm.IsInitialized())
        {
            if (dm.IsPackDownloaded(packName))
            {
                packNameLoading->SetUtf8Text("already downloaded: " + packName);
                return;
            }
        }

        packNameLoading->SetUtf8Text("loading: " + packName);
        dm.RequestPack(packName);
    }
    catch (std::exception& ex)
    {
        packNameLoading->SetUtf8Text(ex.what());
    }
}

void DLCManagerTest::OnStartNextPackClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& pm = *engine.GetContext()->dlcManager;
    String packName = packNextInput->GetUtf8Text();

    pm.requestUpdated.DisconnectAll();
    pm.requestUpdated.Connect(this, &DLCManagerTest::OnRequestUpdated);

    try
    {
        if (pm.IsInitialized())
        {
            if (pm.IsPackDownloaded(packName))
            {
                packNameLoading->SetUtf8Text("already downloaded: " + packName);
                return;
            }
        }

        packNameLoading->SetUtf8Text("loading: " + packName);
        const DLCManager::IRequest* p = pm.RequestPack(packName);
        if (!p->IsDownloaded())
        {
            pm.SetRequestPriority(p);
        }
    }
    catch (std::exception& ex)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString(ex.what()));
    }
}

void DLCManagerTest::OnStartStopLocalServerClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    if (sender == startServerButton)
    {
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

void DLCManagerTest::OnCheckFileClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DAVA::WideString text = filePathField->GetText();
    DAVA::String fileName = UTF8Utils::EncodeToUTF8(text);

    FilePath path(fileName);

    ScopedPtr<File> f(File::Create(path, File::OPEN | File::READ));
    // if we read file from pack - it will be DynamicMemoryFile
    if (nullptr != dynamic_cast<DynamicMemoryFile*>(f.get()))
    {
        packNameLoading->SetUtf8Text("file loaded successfully");
    }
    else
    {
        packNameLoading->SetUtf8Text("can't load file");
    }
}

void DLCManagerTest::OnListInDvpkClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& pm = *engine.GetContext()->dlcManager;

    DLCManager::Progress progress = pm.GetProgress();

    StringStream ss;

    ss << "progress: requestingEnabled=" << progress.isRequestingEnabled
       << " total=" << progress.total << " inQueue=" << progress.inQueue
       << " alreadyDownloaded=" << progress.alreadyDownloaded;

    String t = logPring->GetUtf8Text();
    logPring->SetUtf8Text(t + '\n' + ss.str());
}
