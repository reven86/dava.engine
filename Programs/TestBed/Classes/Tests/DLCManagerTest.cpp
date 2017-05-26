#include "Tests/DLCManagerTest.h"
#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <FileSystem/DynamicMemoryFile.h>
#include <FileSystem/Private/CheckIOError.h>
#include <DLCManager/DLCManager.h>
#include <UI/Focus/UIFocusComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <EmbeddedWebServer.h>
#include <DLCManager/DLCDownloader.h>

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
    packInput->SetUtf8Text("all_level_packs");
    packInput->SetFontSize(14);
    packInput->GetOrCreateComponent<UIDebugRenderComponent>();
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
    packNextInput->GetOrCreateComponent<UIDebugRenderComponent>();
    packNextInput->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    packNextInput->SetInputEnabled(true);
    packNextInput->GetOrCreateComponent<UIFocusComponent>();
    packNextInput->SetDelegate(this);
    packNextInput->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packNextInput);

    loadPack = new UIButton(Rect(420, 10, 100, 20));
    loadPack->GetOrCreateComponent<UIDebugRenderComponent>();
    loadPack->SetStateFont(0xFF, font);
    loadPack->SetStateFontColor(0xFF, Color::White);
    loadPack->SetStateText(0xFF, L"start loading");
    loadPack->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartDownloadClicked));
    AddControl(loadPack);

    loadNext = new UIButton(Rect(420, 40, 100, 20));
    loadNext->GetOrCreateComponent<UIDebugRenderComponent>();
    loadNext->SetStateFont(0xFF, font);
    loadNext->SetStateFontColor(0xFF, Color::White);
    loadNext->SetStateText(0xFF, L"next loading");
    loadNext->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartNextPackClicked));
    AddControl(loadNext);

    startServerButton = new UIButton(Rect(420, 70, 100, 20));
    startServerButton->GetOrCreateComponent<UIDebugRenderComponent>();
    startServerButton->SetStateFont(0xFF, font);
    startServerButton->SetStateFontColor(0xFF, Color::White);
    startServerButton->SetStateText(0xFF, L"start server");
    startServerButton->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartStopLocalServerClicked));
    AddControl(startServerButton);

    stopServerButton = new UIButton(Rect(420, 100, 100, 20));
    stopServerButton->GetOrCreateComponent<UIDebugRenderComponent>();
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
    packNameLoading->GetOrCreateComponent<UIDebugRenderComponent>();
    packNameLoading->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(packNameLoading);

    logPring = new UIStaticText(Rect(540, 530, 500, 200));
    logPring->SetFont(font);
    logPring->SetTextColor(Color::White);
    logPring->SetMultiline(true);
    logPring->SetUtf8Text("");
    logPring->GetOrCreateComponent<UIDebugRenderComponent>();
    logPring->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(logPring);

    redControl = new UIControl(Rect(5, 360, 500, 10));
    redControl->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    AddControl(redControl);

    greenControl = new UIControl(Rect(5, 360, 0, 10));
    greenControl->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color(0.f, 1.f, 0.f, 1.f));
    AddControl(greenControl);

    description = new UIStaticText(Rect(5, 70, 400, 200));
    description->SetFont(font);
    description->SetTextColor(Color::White);
    description->SetMultiline(true);
    description->GetOrCreateComponent<UIDebugRenderComponent>();
    description->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    UpdateDescription();
    AddControl(description);

    url = new UITextField(Rect(5, 250, 400, 20));
    url->SetFont(font);
    url->SetFontSize(14);
    url->SetText(UTF8Utils::EncodeToWideString(urlToServerSuperpack));
    url->GetOrCreateComponent<UIDebugRenderComponent>();
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
    filePathField->GetOrCreateComponent<UIDebugRenderComponent>();
    filePathField->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    filePathField->SetInputEnabled(true);
    filePathField->GetOrCreateComponent<UIFocusComponent>();
    filePathField->SetDelegate(this);
    filePathField->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(filePathField);

    checkFile = new UIButton(Rect(420, 380, 100, 20));
    checkFile->GetOrCreateComponent<UIDebugRenderComponent>();
    checkFile->SetStateFont(0xFF, font);
    checkFile->SetStateFontColor(0xFF, Color::White);
    checkFile->SetStateText(0xFF, L"check file");
    checkFile->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnCheckFileClicked));
    AddControl(checkFile);

    startInit = new UIButton(Rect(420, 410, 100, 20));
    startInit->GetOrCreateComponent<UIDebugRenderComponent>();
    startInit->SetStateFont(0xFF, font);
    startInit->SetStateFontColor(0xFF, Color::White);
    startInit->SetStateText(0xFF, L"PM init");
    startInit->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnStartInitClicked));
    AddControl(startInit);

    genIOError = new UIButton(Rect(420, 440, 100, 20));
    genIOError->GetOrCreateComponent<UIDebugRenderComponent>();
    genIOError->SetStateFont(0xFF, font);
    genIOError->SetStateFontColor(0xFF, Color::White);
    genIOError->SetStateText(0xFF, L"IO error");
    genIOError->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnIOErrorClicked));
    AddControl(genIOError);

    clearDocs = new UIButton(Rect(420, 470, 100, 20));
    clearDocs->GetOrCreateComponent<UIDebugRenderComponent>();
    clearDocs->SetStateFont(0xFF, font);
    clearDocs->SetStateFontColor(0xFF, Color::White);
    clearDocs->SetStateText(0xFF, L"rm dvpk's");
    clearDocs->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnClearDocsClicked));
    AddControl(clearDocs);

    lsDvpks = new UIButton(Rect(420, 500, 100, 20));
    lsDvpks->GetOrCreateComponent<UIDebugRenderComponent>();
    lsDvpks->SetStateFont(0xFF, font);
    lsDvpks->SetStateFontColor(0xFF, Color::White);
    lsDvpks->SetStateText(0xFF, L"ls dvpk's");
    lsDvpks->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnListPacksClicked));
    AddControl(lsDvpks);

    OnOffRequesting = new UIButton(Rect(420, 230, 100, 20));
    OnOffRequesting->GetOrCreateComponent<UIDebugRenderComponent>();
    OnOffRequesting->SetStateFont(0xFF, font);
    OnOffRequesting->SetStateFontColor(0xFF, Color::White);
    OnOffRequesting->SetStateText(0xFF, L"On/Off");
    OnOffRequesting->AddEvent(EVENT_TOUCH_DOWN, Message(this, &DLCManagerTest::OnOffRequestingClicked));
    AddControl(OnOffRequesting);

    dirToListFiles = new UITextField(Rect(5, 300, 400, 20));
    dirToListFiles->SetFont(font);
    dirToListFiles->SetFontSize(14);
    dirToListFiles->SetText(UTF8Utils::EncodeToWideString("~res:/3d/"));
    dirToListFiles->GetOrCreateComponent<UIDebugRenderComponent>();
    dirToListFiles->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    dirToListFiles->SetInputEnabled(true);
    dirToListFiles->GetOrCreateComponent<UIFocusComponent>();
    dirToListFiles->SetDelegate(this);
    dirToListFiles->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(dirToListFiles);

    DLCDownloader::Hints hintsDefault;
    std::stringstream ss;
    ss << "handles " << hintsDefault.numOfMaxEasyHandles << " buf_size " << hintsDefault.chunkMemBuffSize;

    numHandlesInput = new UITextField(Rect(5, 410, 400, 20));
    numHandlesInput->SetFont(font);
    numHandlesInput->SetFontSize(14);
    numHandlesInput->SetUtf8Text(ss.str());
    numHandlesInput->GetOrCreateComponent<UIDebugRenderComponent>();
    numHandlesInput->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    numHandlesInput->SetInputEnabled(true);
    numHandlesInput->GetOrCreateComponent<UIFocusComponent>();
    numHandlesInput->SetDelegate(this);
    numHandlesInput->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(numHandlesInput);

    lsDirFromPacks = new UIButton(Rect(420, 300, 100, 20));
    lsDirFromPacks->GetOrCreateComponent<UIDebugRenderComponent>();
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
    SafeRelease(genIOError);
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
    SafeRelease(numHandlesInput);
    SafeRelease(lsDirFromPacks);
    SafeRelease(OnOffRequesting);

    BaseScreen::UnloadResources();
}

void DLCManagerTest::WriteErrorOnDevice(const String& filePath, int32 errnoVal)
{
    StringStream ss(logPring->GetUtf8Text());
    ss << "Error: can't write file: " << filePath << " errno: " << strerror(errnoVal) << std::endl;
    std::string str = ss.str();
    logPring->SetUtf8Text(str);
    Logger::Info("%s", str.c_str());
}

void DLCManagerTest::OnRequestUpdated(const DAVA::DLCManager::IRequest& request)
{
    const String& packName = request.GetRequestedPackName();
    // change total download progress
    uint64 total = request.GetSize();
    uint64 current = request.GetDownloadedSize();
    float32 progress = static_cast<float32>(current) / total;

    std::stringstream ss;
    ss << "downloading: " << packName << " : (" << (progress * 100) << ")%";

    DLCManager& dm = *engine.GetContext()->dlcManager;
    auto p = dm.GetProgress();
    if (p.total > 0)
    {
        ss << "\n total:" << (100.0 * p.alreadyDownloaded) / p.total << '%';
    }

    std::string str = ss.str();
    packNameLoading->SetUtf8Text(str);

    Logger::Info("DLC %s", str.c_str());

    auto rect = redControl->GetRect();
    rect.dx = rect.dx * progress;
    greenControl->SetRect(rect);
}

void DLCManagerTest::OnNetworkReady(bool isReady)
{
    // To visualize on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading
    std::stringstream ss;
    ss << "network ready = " << std::boolalpha << isReady;
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

    std::stringstream ss(numHandlesInput->GetUtf8Text());
    int numHandles = 0;
    int bufSize = 0;
    String text;
    ss >> text >> numHandles; // skip "handles " // last read num
    ss >> text >> bufSize; // skip " buf_size " // last read num

    DVASSERT(numHandles > 0);
    DVASSERT(bufSize > 0);

    DLCManager::Hints hints;
    hints.downloaderMaxHandles = static_cast<uint32>(numHandles);
    hints.downloaderChankBufSize = static_cast<uint32>(bufSize);

    dm.Initialize(folderWithDownloadedPacks, urlToServerSuperpack, hints);

    dm.SetRequestingEnabled(true);

    packNameLoading->SetText(L"done: start initialize PackManager");
}

void DLCManagerTest::OnIOErrorClicked(BaseObject*, void*, void*)
{
    DebugFS::IOErrorTypes ioErr;
    ioErr.closeFailed = true;
    ioErr.openOrCreateFailed = true;
    ioErr.seekFailed = true;
    ioErr.truncateFailed = true;
    ioErr.readFailed = true;
    ioErr.writeFailed = true;
    ioErr.ioErrorCode = ENOSPC; // no space on device
    GenerateIOErrorOnNextOperation(ioErr);
}

void DLCManagerTest::OnClearDocsClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& dm = *engine.GetContext()->dlcManager;

    FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks, true);

    packNameLoading->SetText(L"done: unmount all dvpk's, and remove dir with downloaded dvpk's");
}

void DLCManagerTest::OnListPacksClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& dm = *engine.GetContext()->dlcManager;
    std::stringstream ss;
    String s = ss.str();

    packNameLoading->SetText(UTF8Utils::EncodeToWideString(s));
}

void DLCManagerTest::OnOffRequestingClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DLCManager& dm = *engine.GetContext()->dlcManager;
    if (dm.IsRequestingEnabled())
    {
        dm.SetRequestingEnabled(false);
    }
    else
    {
        dm.SetRequestingEnabled(true);
    }
}

void DLCManagerTest::OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    // To visualise on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading

    DLCManager& dm = *engine.GetContext()->dlcManager;

    dm.requestUpdated.DisconnectAll();
    dm.requestUpdated.Connect(this, &DLCManagerTest::OnRequestUpdated);
    dm.fileErrorOccured.Connect(this, &DLCManagerTest::WriteErrorOnDevice);

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
        FileSystem* fs = FileSystem::Instance();

        FilePath resPath("~res:/TestData/PackManagerTest/superpack_for_unittests.dvpk");
        FilePath docPath("~doc:/DLCManagerTest/superpack_for_unittests.dvpk");

        fs->CopyFile(resPath, docPath, true);

        docPath = docPath.GetDirectory();

        String absPath = docPath.GetAbsolutePathname();

        const char* docRoot = absPath.c_str();
        const char* ports = "8080";

        if (!StartEmbeddedWebServer(docRoot, ports))
        {
            StringStream ss(logPring->GetUtf8Text());
            ss << "Error starting embedded web server" << std::endl;
            logPring->SetUtf8Text(ss.str());
        }
        else
        {
            logPring->SetUtf8Text("start embedded web server done");
        }
    }
    else if (sender == stopServerButton)
    {
        try
        {
            StopEmbeddedWebServer();
        }
        catch (std::exception& ex)
        {
            StringStream ss(logPring->GetUtf8Text());
            ss << "Error: " << ex.what() << std::endl;
            logPring->SetUtf8Text(ss.str());
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
