#include "Tests/PackManagerTest.h"
#include <UI/Focus/UIFocusComponent.h>
#include <PackManager/PackManager.h>

using namespace DAVA;

PackManagerTest::PackManagerTest()
    : BaseScreen("PackManagerTest")
{
}

void PackManagerTest::TextFieldOnTextChanged(UITextField* textField, const WideString& newText, const WideString& /*oldText*/)
{
    if (url == textField)
    {
        urlPacksCommon = UTF8Utils::EncodeToUTF8(newText);
        UpdateDescription();
    }
}

void PackManagerTest::UpdateDescription()
{
    String message = DAVA::Format("type name of pack you want to download\n"
                                  "Directory to downloaded packs: \"%s\"\nUrl to common packs: \"%s\"\n"
                                  "Url to gpu packs: \"%s\"\n"
                                  "When you press \"start loading\" full reinitializetion begins",
                                  folderWithDownloadedPacks.GetAbsolutePathname().c_str(),
                                  urlPacksCommon.c_str(),
                                  urlPacksGpu.c_str());
    description->SetText(UTF8Utils::EncodeToWideString(message));
}

void PackManagerTest::LoadResources()
{
    BaseScreen::LoadResources();

    eGPUFamily gpu = DeviceInfo::GetGPUFamily();
    switch (gpu)
    {
    case GPU_ADRENO:
        gpuName = "adreno";
        break;
    case GPU_DX11:
        gpuName = "dx11";
        break;
    case GPU_MALI:
        gpuName = "mali";
        break;
    case GPU_POWERVR_IOS:
        gpuName = "pvr_ios";
        break;
    case GPU_POWERVR_ANDROID:
        gpuName = "pvr_android";
        break;
    case GPU_TEGRA:
        gpuName = "tegra";
        break;
    default:
        throw std::runtime_error("unknown gpu famili");
    }

    if (auto startPos = urlPacksGpu.find("{gpu}") != String::npos)
    {
        urlPacksGpu.replace(startPos, 5, gpuName);
    }

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    packInput = new UITextField(Rect(5, 10, 400, 20));
    packInput->SetFont(font);
    packInput->SetText(L"vpack");
    packInput->SetDebugDraw(true);
    packInput->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    packInput->SetInputEnabled(true);
    packInput->GetOrCreateComponent<UIFocusComponent>();
    packInput->SetDelegate(this);
    packInput->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packInput);

    startLoadingButton = new UIButton(Rect(420, 10, 100, 20));
    startLoadingButton->SetDebugDraw(true);
    startLoadingButton->SetStateFont(0xFF, font);
    startLoadingButton->SetStateFontColor(0xFF, Color::White);
    startLoadingButton->SetStateText(0xFF, L"start loading");
    startLoadingButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartDownloadClicked));
    AddControl(startLoadingButton);

    startServerButton = new UIButton(Rect(420, 40, 100, 20));
    startServerButton->SetDebugDraw(true);
    startServerButton->SetStateFont(0xFF, font);
    startServerButton->SetStateFontColor(0xFF, Color::White);
    startServerButton->SetStateText(0xFF, L"start server");
    startServerButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartStopLocalServerClicked));
    AddControl(startServerButton);

    stopServerButton = new UIButton(Rect(420, 70, 100, 20));
    stopServerButton->SetDebugDraw(true);
    stopServerButton->SetStateFont(0xFF, font);
    stopServerButton->SetStateFontColor(0xFF, Color::White);
    stopServerButton->SetStateText(0xFF, L"stop server");
    stopServerButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &PackManagerTest::OnStartStopLocalServerClicked));
    AddControl(stopServerButton);

    packNameLoading = new UIStaticText(Rect(5, 300, 500, 20));
    packNameLoading->SetFont(font);
    packNameLoading->SetTextColor(Color::White);
    packNameLoading->SetMultiline(true);
    packNameLoading->SetText(L"loading: ");
    packNameLoading->SetDebugDraw(true);
    packNameLoading->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(packNameLoading);

    redControl = new UIControl(Rect(5, 360, 500, 10));
    redControl->SetDebugDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    redControl->SetDebugDraw(true);
    AddControl(redControl);

    greenControl = new UIControl(Rect(5, 360, 0, 10));
    greenControl->SetDebugDrawColor(Color(0.f, 1.f, 0.f, 1.f));
    greenControl->SetDebugDraw(true);
    AddControl(greenControl);

    description = new UIStaticText(Rect(5, 40, 400, 200));
    description->SetFont(font);
    description->SetTextColor(Color::White);
    description->SetMultiline(true);
    description->SetDebugDraw(true);
    description->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    UpdateDescription();
    AddControl(description);

    url = new UITextField(Rect(5, 250, 400, 20));
    url->SetFont(font);
    url->SetText(UTF8Utils::EncodeToWideString(urlPacksCommon));
    url->SetDebugDraw(true);
    url->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    url->SetInputEnabled(true);
    url->GetOrCreateComponent<UIFocusComponent>();
    url->SetDelegate(this);
    url->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    AddControl(url);
}

void PackManagerTest::UnloadResources()
{
    SafeRelease(packInput);
    SafeRelease(startLoadingButton);
    SafeRelease(startServerButton);
    SafeRelease(stopServerButton);
    SafeRelease(packNameLoading);
    SafeRelease(redControl);
    SafeRelease(greenControl);
    SafeRelease(description);
    SafeRelease(url);

    BaseScreen::UnloadResources();
}

void PackManagerTest::OnPackStateChange(const DAVA::PackManager::Pack& pack, DAVA::PackManager::Pack::Change change)
{
    if (change == PackManager::Pack::Change::DownloadProgress)
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + pack.name));

        auto rect = redControl->GetRect();
        rect.dx = rect.dx * pack.downloadProgress;
        greenControl->SetRect(rect);
    }
    else if (change == PackManager::Pack::Change::State)
    {
        if (pack.state == PackManager::Pack::Status::Mounted)
        {
            packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + pack.name + " done!"));
        }
        else if (pack.state == PackManager::Pack::Status::ErrorLoading || pack.state == PackManager::Pack::Status::OtherError)
        {
            packNameLoading->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("error: %s, %d, %s", pack.name.c_str(), pack.downloadError, pack.otherErrorMsg.c_str())));
        }
    }
}

void PackManagerTest::OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData)
{
    // To visualise on MacOS DownloadManager::Instance()->SetDownloadSpeedLimit(100000);
    // on MacOS slowly connect and then fast downloading

    PackManager& packManager = Core::Instance()->GetPackManager();

    const Vector<PackManager::Pack>& packs = packManager.GetPacks();

    std::for_each(begin(packs), end(packs), [&packManager](const PackManager::Pack& pack)
                  {
                      if (pack.state == PackManager::Pack::Status::Mounted)
                      {
                          packManager.DeletePack(pack.name);
                      }
                  });

    FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks, true);
    FileSystem::Instance()->CreateDirectory(folderWithDownloadedPacks, true);

    String dbFile = sqliteDbFile.GetStringValue();
    dbFile.replace(dbFile.find("{gpu}"), 5, gpuName);
    FilePath dbPath(dbFile);

    // clear and renew all packs state
    packManager.Initialize(dbPath, folderWithDownloadedPacks, FilePath(""), urlPacksCommon, urlPacksGpu);
    packManager.EnableProcessing();

    packManager.onPackStateChanged.DisconnectAll();

    packManager.onPackStateChanged.Connect(this, &PackManagerTest::OnPackStateChange);

    String packName = UTF8Utils::EncodeToUTF8(packInput->GetText());

    try
    {
        packNameLoading->SetText(UTF8Utils::EncodeToWideString("loading: " + packName));
        packManager.RequestPack(packName);
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
        // TODO fix for uap
        // std::system("python scripts/start_local_http_server.py");
    }
    else if (sender == stopServerButton)
    {
        // TODO fix for uap
        // std::system("python scripts/stop_local_http_server.py");
    }
}
