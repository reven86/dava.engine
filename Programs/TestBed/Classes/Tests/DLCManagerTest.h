#pragma once

#include "Infrastructure/BaseScreen.h"
#include <FileSystem/FilePath.h>
#include <DLCManager/DLCManager.h>

class TestBed;
class DLCManagerTest : public BaseScreen, DAVA::UITextFieldDelegate
{
public:
    DLCManagerTest(TestBed& app);
    ~DLCManagerTest();

private:
    void TextFieldOnTextChanged(DAVA::UITextField* textField, const DAVA::WideString& newText, const DAVA::WideString& /*oldText*/) override;
    void UpdateDescription();

    void LoadResources() override;
    void UnloadResources() override;

    void OnStartInitClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartSyncClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnClearDocsClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnListPacksClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartNextPackClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartStopLocalServerClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnCheckFileClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnListInDvpkClicked(DAVA::BaseObject* sender, void* data, void* callerData);

    void WriteErrorOnDevice(const DAVA::String& filePath, DAVA::int32 errVal);
    void OnRequestUpdated(const DAVA::DLCManager::IRequest& request);
    void OnNetworkReady(bool isReady);
    void OnInitializeFinished(size_t numDownloaded, size_t numTotalFiles);

    DAVA::Engine& engine;

    DAVA::FilePath folderWithDownloadedPacks = "~doc:/DLCManagerTest/packs/";
    // TODO quick and dirty way to test download on all platforms, in future replace with local http server
    DAVA::String urlToServerSuperpack = "http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk";
    //"http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk";
    //"http://127.0.0.1:8080/superpack_for_unittests.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/superpack_for_unittests.dvpk";
    //"http://127.0.0.1:2424/superpack.dvpk";

    DAVA::UIStaticText* packNameLoading = nullptr;
    DAVA::UIStaticText* logPring = nullptr;
    DAVA::UIButton* loadPack = nullptr;
    DAVA::UIButton* loadNext = nullptr;

    DAVA::UIButton* startServerButton = nullptr;
    DAVA::UIButton* stopServerButton = nullptr;

    DAVA::UITextField* packInput = nullptr;
    DAVA::UITextField* packNextInput = nullptr;
    DAVA::UITextField* numHandlesInput = nullptr;
    DAVA::UIControl* redControl = nullptr;
    DAVA::UIControl* greenControl = nullptr;
    DAVA::UIStaticText* description = nullptr;
    DAVA::UITextField* url = nullptr;
    DAVA::UITextField* filePathField = nullptr;
    DAVA::UIButton* checkFile = nullptr;
    DAVA::UIButton* startInit = nullptr;
    DAVA::UIButton* startSync = nullptr;
    DAVA::UIButton* clearDocs = nullptr;
    DAVA::UIButton* lsDvpks = nullptr;
    DAVA::UITextField* dirToListFiles = nullptr;
    DAVA::UIButton* lsDirFromPacks = nullptr;
};
