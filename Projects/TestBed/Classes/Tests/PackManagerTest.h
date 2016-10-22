#pragma once

#include "Infrastructure/BaseScreen.h"
#include <FileSystem/FilePath.h>
#include <PackManager/PackManager.h>

class TestBed;
class PackManagerTest : public BaseScreen, DAVA::UITextFieldDelegate
{
public:
    PackManagerTest(TestBed& app);

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

    void OnPackStateChange(const DAVA::IPackManager::Pack& pack);
    void OnPackDownloadChange(const DAVA::IPackManager::Pack& pack);
    void OnRequestChange(const DAVA::IPackManager::IRequest& request);
    void OnInitChange(DAVA::IPackManager& init);

    DAVA::Engine& engine;

    DAVA::String sqliteDbFile = "db_{gpu}.db.zip";
    DAVA::FilePath folderWithDownloadedPacks = "~doc:/PackManagerTest/packs/";
    // TODO quick and dirty way to test download on all platforms, in future replace with local http server
    DAVA::String urlToServerSuperpack = "http://127.0.0.1:2424/superpack.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/s000001/superpack.dvpk";
    //"http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/superpack.dvpk";
    //"http://127.0.0.1:2424/superpack.dvpk"
    DAVA::String gpuArchitecture;

    DAVA::UIStaticText* packNameLoading = nullptr;
    DAVA::UIButton* loadPack = nullptr;
    DAVA::UIButton* loadNext = nullptr;

    DAVA::UIButton* startServerButton = nullptr;
    DAVA::UIButton* stopServerButton = nullptr;

    DAVA::UIControl* progressBar = nullptr;
    DAVA::UITextField* packInput = nullptr;
    DAVA::UITextField* packNextInput = nullptr;
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
