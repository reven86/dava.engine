#include "Autotesting/AutotestingDB.h"

#ifdef __DAVAENGINE_AUTOTESTING__
#include "Platform/DeviceInfo.h"

// Work with MongoDb API
#define AUTOTESTING_TESTS "Tests"
#define AUTOTESTING_STEPS "Steps"
#define AUTOTESTING_LOG "Log"

namespace DAVA
{
const String AutotestingDB::DB_ERROR_STR_VALUE = "not_found";
const String MULTIPLAYER_ARCHIVE = "multiplayer";

AutotestingDB::AutotestingDB()
    : dbClient(nullptr)
    , logFilePath(FilePath(""))
    , logsFolder(FilePath(""))
    , autoSys(nullptr)
{
    autoSys = AutotestingSystem::Instance();
}

AutotestingDB::~AutotestingDB()
{
    CloseConnection();
}

bool AutotestingDB::ConnectToDB(const String& collection, const String& dbName, const String& dbHost, const int32 dbPort)
{
    DVASSERT(nullptr == dbClient);

    dbClient = MongodbClient::Create(dbHost, dbPort);
    if (dbClient)
    {
        dbClient->SetDatabaseName(dbName);
        dbClient->SetCollectionName(collection);
    }
    return (nullptr != dbClient);
}

void AutotestingDB::CloseConnection()
{
    if (dbClient)
    {
        dbClient->Disconnect();
        SafeRelease(dbClient);
    }
}

String AutotestingDB::GetStringTestParameter(const String& deviceName, const String& parameter)
{
    Logger::Info("AutotestingDB::GetStringTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());
    String result = DB_ERROR_STR_VALUE;
    if (dbClient)
    {
        RefPtr<MongodbUpdateObject> dbUpdateObject(new MongodbUpdateObject);
        KeyedArchive* currentRunArchive = FindBuildArchive(dbUpdateObject.Get(), "autotesting_system");

        DVASSERT(currentRunArchive != nullptr);
        KeyedArchive* deviceArchive = currentRunArchive->GetArchive(deviceName, nullptr);
        if (nullptr == deviceArchive)
        {
            autoSys->ForceQuit(Format("Couldn't find archive for %s device", deviceName.c_str()));
        }
        result = deviceArchive->GetString(parameter, DB_ERROR_STR_VALUE);
    }
    else
    {
        RefPtr<KeyedArchive> deviceArchive = AutotestingSystem::Instance()->GetIdYamlOptions();
        result = deviceArchive->GetString(parameter, DB_ERROR_STR_VALUE);
    }
    Logger::Info("AutotestingDB::GetStringTestParameter return value: %s", result.c_str());
    return result;
}

int32 AutotestingDB::GetIntTestParameter(const String& deviceName, const String& parameter)
{
    Logger::Info("AutotestingDB::GetIntTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());
    KeyedArchive* deviceArchive = nullptr;
    int32 result = DB_ERROR_INT_VALUE;
    if (dbClient)
    {
        RefPtr<MongodbUpdateObject> dbUpdateObject(new MongodbUpdateObject);
        KeyedArchive* currentRunArchive = FindBuildArchive(dbUpdateObject.Get(), "autotesting_system");
        deviceArchive = currentRunArchive->GetArchive(deviceName, nullptr);
        if (nullptr == deviceArchive)
        {
            autoSys->ForceQuit(Format("Couldn't find archive for %s device", deviceName.c_str()));
        }
        result = deviceArchive->GetInt32(parameter, DB_ERROR_INT_VALUE);
    }
    else
    {
        deviceArchive = AutotestingSystem::Instance()->GetIdYamlOptions().Get();
        result = deviceArchive->GetInt32(parameter, DB_ERROR_INT_VALUE);
    }
    Logger::Info("AutotestingDB::GetIntTestParameter return value: %d", result);
    return result;
}

void AutotestingDB::FailOnLocalBuild()
{
    if (dbClient == nullptr)
        autoSys->OnError("Could not work with BD on local build");
}

// BUILD Level
KeyedArchive* AutotestingDB::FindBuildArchive(MongodbUpdateObject* dbUpdateObject, const String& archiveName)
{
    if (archiveName.empty())
    {
        autoSys->ForceQuit("Archive name is empty.");
    }
    if (!dbClient->FindObjectByKey(archiveName, dbUpdateObject))
    {
        return nullptr;
    }
    dbUpdateObject->LoadData();
    return dbUpdateObject->GetData();
}

KeyedArchive* AutotestingDB::FindOrInsertBuildArchive(MongodbUpdateObject* dbUpdateObject, const String& archiveName)
{
    if (archiveName.empty())
    {
        autoSys->ForceQuit("Archive name is empty.");
    }
    if (!dbClient->FindObjectByKey(archiveName, dbUpdateObject))
    {
        dbUpdateObject->SetObjectName(archiveName);
        Logger::Debug("AutotestingSystem::InsertNewArchive  %s", archiveName.c_str());
    }
    dbUpdateObject->LoadData();
    KeyedArchive* dbUpdateData = dbUpdateObject->GetData();
    return dbUpdateData;
}

void AutotestingDB::WriteLogHeader()
{
    if (DeviceInfo::GetPlatform() == DeviceInfo::PLATFORM_PHONE_WIN_UAP)
    {
        logsFolder = "D:/autoLogs";
    }
    else if (DeviceInfo::GetPlatform() == DeviceInfo::PLATFORM_ANDROID)
    {
        logsFolder = FileSystem::Instance()->GetPublicDocumentsPath() + "/autoLogs";
    }
    else
    {
        logsFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "/autoLogs";
    }

    Logger::Info("AutotestingSystem::AutotestingDB path to log file: %s", logsFolder.GetStringValue().c_str());
    if (!FileSystem::Instance()->IsDirectory(logsFolder))
    {
        FileSystem::Instance()->CreateDirectory(logsFolder);
    }
    autoSys->testIndex++;
    logFilePath = logsFolder + Format("/%s_%s_%s_%d.log", autoSys->groupName.c_str(), autoSys->testFileName.c_str(), autoSys->runId.c_str(), autoSys->testIndex);
    if (FileSystem::Instance()->IsFile(logFilePath))
    {
        FileSystem::Instance()->DeleteFile(logFilePath);
    }

    DateTime time = DateTime::Now();
    //Get time.GetMonth() return month number - 1. Ex for 01(Jan) it return 00(Jan).
    String currentDay = Format("%d-%d-%d", time.GetYear(), time.GetMonth() + 1, time.GetDay());
    String message = Format("Platform:%s\nName:%s\n", AutotestingSystemLua::Instance()->GetPlatform().c_str(), autoSys->deviceName.c_str()) +
    Format("Model:%s\nOSVersion:%s\n", DeviceInfo::GetModel().c_str(), DeviceInfo::GetVersion().c_str()) +
    Format("BuildDate:%s\nLaunchDate:%s\n", autoSys->buildDate.c_str(), currentDay.c_str()) +
    Format("RunId:%s\nBuildId:%s\n", autoSys->runId.c_str(), autoSys->buildId.c_str()) +
    Format("Client:%s\nClientRevision:%s\n", autoSys->branch.c_str(), autoSys->branchRev.c_str()) +
    Format("Framework:%s\nFrameworkRevision:%s\n", autoSys->framework.c_str(), autoSys->frameworkRev.c_str()) +
    Format("TestGroup:%s\nFileName:%s\n", autoSys->groupName.c_str(), autoSys->testFileName.c_str());
    WriteLog(message.c_str());
    Logger::Debug("AutotestingDB::Log: [%s:%s] INFO", autoSys->GetCurrentTimeString().c_str(), message.c_str());
}

void AutotestingDB::WriteLog(const char8* text, ...)
{
    if (!text || text[0] == '\0')
        return;
    File* file = File::Create(logFilePath, File::APPEND | File::WRITE);
    if (!file)
    {
        Logger::Error("Can't open/create log file.");
        return;
    }
    va_list vl;
    va_start(vl, text);
    char tmp[4096] = { 0 };
    vsnprintf(tmp, sizeof(tmp) - 2, text, vl);
    file->Write(text, static_cast<uint32>(sizeof(char) * strlen(tmp)));
    file->Release();
    va_end(vl);
}

void AutotestingDB::Log(const String& level, const String& message)
{
    String textLog = Format("[%s:%s] %s\n", autoSys->GetCurrentTimeString().c_str(), level.c_str(), message.c_str());
    Logger::Debug("AutotestingDB::Log: [%s:%s] %s", autoSys->GetCurrentTimeString().c_str(), level.c_str(), message.c_str());
    WriteLog(textLog.c_str());
}

bool AutotestingDB::SaveKeyedArchiveToDevice(const String& archiveName, KeyedArchive* archive)
{
    String fileName = Format("/%s_%s_%s_%d_%s.yaml", autoSys->groupName.c_str(), autoSys->testFileName.c_str(), autoSys->runId.c_str(), autoSys->testIndex, archiveName.c_str());
    Logger::Info("AutotestingDB::Save keyed archive '%s' to device.", fileName.c_str());
    Log("DEBUG", Format("%s=%s", archiveName.c_str(), YamlToString(archive).c_str()));
    return archive->SaveToYamlFile(logsFolder + fileName);
}

String AutotestingDB::YamlToString(const KeyedArchive* archive)
{
    String result = "keyedArchive: \\n";
    for (const auto& obj : archive->GetArchieveData())
    {
        switch (obj.second->GetType())
        {
        case VariantType::TYPE_BOOLEAN:
        {
            if (obj.second->boolValue)
            {
                result += Format("  t%s: {string: \"true\"}\\n", obj.first.c_str());
            }
            else
            {
                result += Format("  t%s: {string: \"flase\"}\\n", obj.first.c_str());
            }
        }
        break;
        case VariantType::TYPE_INT32:
        {
            result += Format("  %s: {string: \"%d\"}\\n", obj.first.c_str(), obj.second->int32Value);
        }
        break;
        case VariantType::TYPE_FLOAT:
        {
            result += Format("  %s: {string: \"%f\"}\\n", obj.first.c_str(), obj.second->floatValue);
        }
        break;
        case VariantType::TYPE_STRING:
        {
            result += Format("  %s: {string: \"%s\"}\\n", obj.first.c_str(), obj.second->stringValue->c_str());
        }
        break;
        case VariantType::TYPE_WIDE_STRING:
        {
            result += Format("  %s: {string: \"%s\"}\\n", obj.first.c_str(), obj.second->wideStringValue->c_str());
        }
        break;

        default:
            break;
        }
    }
    return result;
}

bool AutotestingDB::SaveToDB(MongodbUpdateObject* dbUpdateObject)
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::Info("AutotestingSystem::SaveToDB");

    bool ret = dbUpdateObject->SaveToDB(dbClient);

    if (!ret)
    {
        Logger::Error("AutotestingSystem::SaveToDB failed");
    }

    uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::Info("AutotestingSystem::SaveToDB FINISH result time %d", finishTime - startTime);
    return ret;
}

void AutotestingDB::UploadScreenshot(const String& name, Image* image)
{
    if (dbClient)
    {
        dbClient->SaveBufferToGridFS(Format("%s_%dx%d", name.c_str(), image->GetWidth(), image->GetHeight()),
                                     reinterpret_cast<char*>(image->GetData()), image->dataSize);
    }
}

// DEPRECATED: Rewrite for new DB conception
String AutotestingDB::ReadState(const String& device, const String& param)
{
    FailOnLocalBuild();

    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* multiplayerArchive = FindOrInsertBuildArchive(dbUpdateObject, MULTIPLAYER_ARCHIVE);

    KeyedArchive* deviceArchive = multiplayerArchive->GetArchive(device, nullptr);
    String result = DB_ERROR_STR_VALUE;
    if (deviceArchive != nullptr)
    {
        result = deviceArchive->GetString(param, DB_ERROR_STR_VALUE);
    }

    Logger::Info("AutotestingDB::ReadState device=%s: %s='%s'", device.c_str(), param.c_str(), result.c_str());
    SafeRelease(dbUpdateObject);
    return result;
}

void AutotestingDB::WriteState(const String& device, const String& param, const String& state)
{
    Logger::Info("AutotestingDB::WriteState device=%s %s=%s", device.c_str(), param.c_str(), state.c_str());
    FailOnLocalBuild();

    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* multiplayerArchive = FindOrInsertBuildArchive(dbUpdateObject, MULTIPLAYER_ARCHIVE);
    ScopedPtr<KeyedArchive> emptyKeyedArchive(new KeyedArchive());
    KeyedArchive* deviceArchive = multiplayerArchive->GetArchive(device, emptyKeyedArchive);

    deviceArchive->SetString(param, state);
    multiplayerArchive->SetArchive(device, deviceArchive);
    SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
}

// auxiliary methods
void AutotestingDB::SetTestStarted()
{
    Logger::Info("AutotestingSystem::SetTestStarted for test: %s", autoSys->testFileName.c_str());
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
    if (!currentRunArchive)
    {
        autoSys->ForceQuit(Format("Couldn't find archive autotesting_system device"));
    }
    KeyedArchive* deviceArchive = currentRunArchive->GetArchive(autoSys->deviceName, nullptr);

    if (!deviceArchive)
    {
        autoSys->ForceQuit(Format("Couldn't find archive for %s device", autoSys->deviceName.c_str()));
    }
    deviceArchive->SetString("Started", "1");
    deviceArchive->SetString("BuildId", autoSys->buildId.c_str());
    deviceArchive->SetString("Framework", autoSys->framework.c_str());
    deviceArchive->SetString("Branch", autoSys->branch.c_str());
    deviceArchive->SetInt32("TestIndex", autoSys->testIndex);
    SaveToDB(dbUpdateObject);
    SafeRelease(dbUpdateObject);
}
}

#endif //__DAVAENGINE_AUTOTESTING__