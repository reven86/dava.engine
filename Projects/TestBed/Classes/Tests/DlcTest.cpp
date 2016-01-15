/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "DlcTest.h"
#include "Base/GlobalEnum.h"
#include "Database/MongodbClient.h"
#include "Database/MongodbObject.h"
#include "Notification/LocalNotificationText.h"
#include "Notification/LocalNotificationProgress.h"
#include "Utils/StringUtils.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/DeviceInfo.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#endif

//#define DLC_TEST
#define DLC_TEST_MAX_RETRY_COUNT 10
#define DLC_MONGO_HOST String("by2-badava-mac-11.corp.wargaming.local")
#define DLC_MONGO_PORT 27017
#define DLC_MONGO_TEST_DB "dlc"
#define DLC_MONGO_TEST_COLLECTION "test.exit"

const DAVA::String gameVer = "1.9.0"; // "dlcdevtest";

#if defined(__DAVAENGINE_IPHONE__)
//const DAVA::String url = "http://by1-builddlc-01/DLC_Blitz";
const DAVA::String url = "http://dl.wargaming.net/wotblitz/dlc/";
#else
//const DAVA::String url = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz";
const DAVA::String url = "http://dl.wargaming.net/wotblitz/dlc/";
#endif

DlcTest::DlcTest()
    : BaseScreen("DlcTest")
    , dlc(nullptr)
{
}

void DlcTest::LoadResources()
{
    BaseScreen::LoadResources();

    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    Font* fontSmall = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(24.0f);
    fontSmall->SetSize(14.0f);

    angle = 0;
    lastUpdateTime = 0;

#ifdef __DAVAENGINE_ANDROID__
    List<DeviceInfo::StorageInfo> storageList = DeviceInfo::GetStoragesList();
    DeviceInfo::StorageInfo external;
    bool externalStorage(false);
    for (const auto& it : storageList)
    {
        if (DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL == it.type)
        {
            external = it;
            DVASSERT(false == externalStorage);
            externalStorage = true;
        }
    }
    workingDir = external.path + "DLC/";
    sourceDir = external.path + "DLC_Resources/";
    destinationDir = external.path + "DLC_Resources/";
#else
    workingDir = "~doc:/DLC/";
    sourceDir = "~doc:/Resources/";
    destinationDir = "~doc:/Resources/";
#endif

    WideString infoStr = L"DLCWorkingDir: ";
    infoStr += StringToWString(workingDir.GetAbsolutePathname());
    infoStr += L"\nResourcesDir: ";
    infoStr += StringToWString(destinationDir.GetAbsolutePathname());
    infoStr += L"\nURL: ";
    infoStr += StringToWString(url);
    infoStr += L"\nGameVer: ";
    infoStr += StringToWString(gameVer);

    UIStaticText* infoText = new UIStaticText(Rect(10.0f, 10.0f, 500.f, 190.f));
    infoText->SetTextColor(Color::White);
    infoText->SetFont(fontSmall);
    infoText->SetMultiline(true);
    infoText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    infoText->SetText(infoStr);
    AddControl(infoText);

    staticText = new UIStaticText(Rect(10.0f, 200.f, 400.f, 50.f));
    staticText->SetFont(font);
    staticText->SetTextColor(Color::White);
    staticText->SetDebugDraw(true);
    staticText->SetText(L"Starting DLC...");
    AddControl(staticText);

    progressControl = new UIControl(Rect(10.0f, 260.0f, 400.0f, 5.0f));
    progressControl->SetDebugDraw(true);
    AddControl(progressControl);

    animControl = new UIControl(Rect(470.0f, 235.0f, 50.f, 50.f));
    animControl->SetDebugDraw(true);
    animControl->SetPivotPoint(Vector2(25.0f, 25.0f));
    AddControl(animControl);

    returnButton = new UIButton(Rect(10.0f, 400.f, 500.f, 50.f));
    returnButton->SetStateFont(0xFF, font);
    returnButton->SetStateFontColor(0xFF, Color::White);
    returnButton->SetStateText(0xFF, L"Cancel operation");
    returnButton->SetDebugDraw(true);
    returnButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::Cancel));
    AddControl(returnButton);

    restartButton = new UIButton(Rect(10.0f, 300.f, 500.f, 50.f));
    restartButton->SetStateFont(0xFF, font);
    restartButton->SetStateFontColor(0xFF, Color::White);
    restartButton->SetStateText(0xFF, L"Restart DLC");
    restartButton->SetDebugDraw(true);
    restartButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DlcTest::Restart));
    AddControl(restartButton);

    DAVA::FilePath::AddResourcesFolder(destinationDir);

#ifdef DLC_TEST
    crashTest.Init(workingDir, destinationDir);
#endif

    DAVA::FileSystem::Instance()->CreateDirectory(workingDir);
    DAVA::FileSystem::Instance()->CreateDirectory(destinationDir);
    dlc = new DLC(url, sourceDir, destinationDir, workingDir, gameVer, destinationDir + "/version/resources.txt");

    lastDLCState = dlc->GetState();

    SafeRelease(font);
    SafeRelease(fontSmall);
}

void DlcTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    SafeRelease(staticText);
    SafeRelease(animControl);
    SafeDelete(dlc);
}

void DlcTest::WillAppear()
{
    dlc->Start();
}

void DlcTest::Update(float32 timeElapsed)
{
    BaseScreen::Update(timeElapsed);

    lastUpdateTime += timeElapsed;
    if (lastUpdateTime > 0.05f)
    {
        // update animation
        angle += 0.10f;
        lastUpdateTime = 0;

        animControl->SetAngle(angle);

        // update progress control
        Rect r = staticText->GetRect();

        if (nullptr == dlc)
        {
            return;
        }

        if (r.dx > 0)
        {
            float32 w = 0;
            switch (lastDLCState)
            {
            case DLC::DS_READY:
            case DLC::DS_DOWNLOADING:
            case DLC::DS_PATCHING:
            {
                uint64 cur = 0;
                uint64 total = 0;

                dlc->GetProgress(cur, total);
                if (total > 0)
                {
                    w = cur * r.dx / total;
                    if (0 == w && r.dx > 0)
                    {
                        w = 1;
                    }
                }
                else
                {
                    w = r.dx;
                }
            }
            break;
            default:
                w = r.dx;
                break;
            }

            Rect pr = progressControl->GetRect();
            progressControl->SetRect(Rect(pr.x, pr.y, w, pr.dy));
        }
    }

    uint32 dlcState = dlc->GetState();
    if (lastDLCState != dlcState)
    {
        lastDLCState = dlcState;

        switch (lastDLCState)
        {
        case DLC::DS_CHECKING_INFO:
            staticText->SetText(L"Checking version...");
            break;
        case DLC::DS_CHECKING_PATCH:
            staticText->SetText(L"Checking patch...");
            break;
        case DLC::DS_CHECKING_META:
            staticText->SetText(L"Checking meta information...");
            break;
        case DLC::DS_READY:
            staticText->SetText(L"Ready for download.");
            break;
        case DLC::DS_DOWNLOADING:
            staticText->SetText(L"Downloading...");
            break;
        case DLC::DS_PATCHING:
            staticText->SetText(L"Patching...");
            break;
        case DLC::DS_CANCELLING:
            staticText->SetText(L"Canceling...");
            break;
        case DLC::DS_DONE:
            if (dlc->GetError() == DLC::DE_NO_ERROR)
            {
                staticText->SetText(L"Done!");
            }
            else
            {
                DAVA::String errorText = DAVA::Format("Error %s!", GlobalEnumMap<DAVA::DLC::DLCError>::Instance()->ToString(dlc->GetError()));
                DAVA::WideString wErrorText;
                DAVA::UTF8Utils::EncodeToWideString((DAVA::uint8*)errorText.c_str(), errorText.size(), wErrorText);
                staticText->SetText(wErrorText);
            }
            break;
        default:
            break;
        }
    }

#ifdef DLC_TEST
    crashTest.Update(timeElapsed, dlc);
#endif
}

void DlcTest::Draw(const UIGeometricData& geometricData)
{
}

void DlcTest::Cancel(BaseObject* obj, void* data, void* callerData)
{
    staticText->SetText(L"Cancelling DLC...");

    dlc->Cancel();
}

void DlcTest::Restart(BaseObject* obj, void* data, void* callerData)
{
    volatile static bool isRestarting = false;
    if (!isRestarting)
    {
        staticText->SetText(L"Restarting DLC...");

        isRestarting = true;
        dlc->Cancel();

        FileSystem::Instance()->DeleteDirectory(workingDir);
        FileSystem::Instance()->DeleteDirectory(sourceDir);
        FileSystem::Instance()->DeleteDirectory(destinationDir);

        SafeDelete(dlc);

        DAVA::FileSystem::Instance()->CreateDirectory(workingDir);
        DAVA::FileSystem::Instance()->CreateDirectory(destinationDir);
        dlc = new DLC(url, sourceDir, destinationDir, workingDir, gameVer, destinationDir + "/version/resources.txt");

        lastDLCState = dlc->GetState();

        dlc->Start();
        isRestarting = false;
    }
}

void DLCCrashTest::Init(const DAVA::FilePath& workingDir, const DAVA::FilePath& destinationDir)
{
    forceExit = false;
    inExitMode = true;

    DAVA::Random::Instance()->Seed();
    cancelTimeout = DAVA::Random::Instance()->Rand(25);
    exitTimeout = DAVA::Random::Instance()->Rand(25);
    retryCount = DLC_TEST_MAX_RETRY_COUNT;

    testingFileFlag = workingDir + "run.test";
    if (!testingFileFlag.Exists())
    {
        DAVA::File* f = DAVA::File::Create(testingFileFlag, DAVA::File::CREATE | DAVA::File::WRITE);
        f->Write(&retryCount);

        DAVA::FileSystem::Instance()->DeleteDirectoryFiles(destinationDir, true);
        exitThread = Thread::Create(Message(this, &DLCCrashTest::ExitThread));
        exitThread->Start();

        DAVA::MongodbClient* dbClient = DAVA::MongodbClient::Create(DLC_MONGO_HOST, DLC_MONGO_PORT);
        if (dbClient->IsConnected())
        {
            dbClient->SetDatabaseName(DLC_MONGO_TEST_DB);
            dbClient->SetCollectionName(DLC_MONGO_TEST_COLLECTION);

            DAVA::MongodbObject* dbObject = new DAVA::MongodbObject();
            dbObject->SetUniqueObjectName();
            dbObject->AddString("state", "started");
            dbObject->AddInt32("retries", retryCount - DLC_TEST_MAX_RETRY_COUNT);
            dbObject->AddInt32("error", 0);
            dbObject->Finish();

            dbObjectId = dbObject->GetObjectName();
            f->WriteString(dbObjectId);

            dbClient->SaveDBObject(dbObject);
            SafeRelease(dbObject);
        }

        f->Release();
    }
    else
    {
        DAVA::File* f = DAVA::File::Create(testingFileFlag, DAVA::File::OPEN | DAVA::File::READ);
        if (nullptr != f && f->Read(&retryCount) > 0)
        {
            f->ReadString(dbObjectId);
        }
        else
        {
            retryCount = 0;
        }

        f->Release();
        inExitMode = false;
    }
}

void DLCCrashTest::Update(float32 timeElapsed, DLC* dlc)
{
    static float32 runtimeElapsed = 0;
    runtimeElapsed += timeElapsed;

    if (inExitMode)
    {
        if (runtimeElapsed > cancelTimeout)
        {
            dlc->Cancel();
        }

        if (forceExit)
        {
            exit(1);
        }
    }

    if (dlc->GetState() == DLC::DS_DONE)
    {
        int ret = 0;
        if (dlc->GetError() != DLC::DE_NO_ERROR)
        {
            ret = 1;
        }

        if (!inExitMode)
        {
            // finished without errors or there is no retry counts
            if (0 == ret || 0 == retryCount)
            {
                DAVA::FileSystem::Instance()->DeleteFile(testingFileFlag);
            }

            // dlc finished ok, write info into db
            if (0 == ret)
            {
                DAVA::MongodbClient* dbClient = DAVA::MongodbClient::Create(DLC_MONGO_HOST, DLC_MONGO_PORT);
                if (dbClient->IsConnected())
                {
                    dbClient->SetDatabaseName(DLC_MONGO_TEST_DB);
                    dbClient->SetCollectionName(DLC_MONGO_TEST_COLLECTION);

                    DAVA::MongodbObject* dbObject = dbClient->FindObjectByKey(dbObjectId);
                    if (nullptr != dbObject)
                    {
                        DAVA::MongodbObject* newDbObject = new DAVA::MongodbObject();
                        newDbObject->SetObjectName(dbObject->GetObjectName());
                        newDbObject->AddString("state", "finished");
                        newDbObject->AddInt32("retries", retryCount - DLC_TEST_MAX_RETRY_COUNT);
                        newDbObject->AddInt32("error", dlc->GetError());
                        newDbObject->Finish();

                        dbClient->SaveDBObject(newDbObject, dbObject);
                        SafeRelease(dbObject);
                        SafeRelease(newDbObject);
                    }
                }
            }
        }

        exit(ret);
    }
}

void DLCCrashTest::ExitThread(BaseObject* caller, void* callerData, void* userData)
{
    DAVA::uint64 start = DAVA::SystemTimer::Instance()->AbsoluteMS();
    DAVA::uint64 elapsed = 0;

    while (elapsed < exitTimeout)
    {
        elapsed = (DAVA::SystemTimer::Instance()->AbsoluteMS() - start) / 1000;
        Thread::Sleep(100);
    }

    forceExit = true;
}
