#include "Classes/Application/REGlobal.h"
#include "Main/RecentMenuItems.h"
#include "Settings/SettingsManager.h"

#include "TArc/WindowSubSystem/QtAction.h"

#include "FileSystem/KeyedArchive.h"

#include <QMenu>
#include <QAction>

RecentMenuItems::RecentMenuItems(const Params& params_)
    : params(params_)
{
    InitMenuItems();
}

void RecentMenuItems::Add(const DAVA::String& recent)
{
    RemoveMenuItems();
    AddInternal(recent);
    InitMenuItems();
}

void RecentMenuItems::RemoveMenuItems()
{
    DAVA::Vector<DAVA::String> actions = Get();
    for (const DAVA::String& action : actions)
    {
        QList<QString> menuPath = params.menuSubPath;
        menuPath.push_back(QString::fromStdString(action));

        DAVA::TArc::ActionPlacementInfo placement(DAVA::TArc::CreateMenuPoint(menuPath));
        params.ui->RemoveAction(REGlobal::MainWindowKey, placement);
    }
}

void RecentMenuItems::InitMenuItems()
{
    DAVA::Vector<DAVA::String> pathList = Get();
    for (const DAVA::String& path : pathList)
    {
        if (path.empty())
        {
            continue;
        }

        QString pathQt = QString::fromStdString(path);
        DAVA::TArc::QtAction* action = new DAVA::TArc::QtAction(params.accessor, pathQt);
        if (params.enablePredicate)
        {
            action->SetStateUpdationFunction(DAVA::TArc::QtAction::Enabled, params.predicateFieldDescriptor, params.enablePredicate);
        }

        connections.AddConnection(action, &QAction::triggered, [path, this]()
                                  {
                                      actionTriggered.Emit(path);
                                  });

        DAVA::TArc::ActionPlacementInfo placement(DAVA::TArc::CreateMenuPoint(params.menuSubPath));
        params.ui->AddAction(REGlobal::MainWindowKey, placement, action);
    }
}

void RecentMenuItems::AddInternal(const DAVA::String& recent)
{
    DAVA::Vector<DAVA::String> vectorToSave = Get();

    DAVA::FilePath filePath(recent);
    DAVA::String stringToInsert = filePath.GetAbsolutePathname();

    //check present set to avoid duplicates
    vectorToSave.erase(std::remove(vectorToSave.begin(), vectorToSave.end(), stringToInsert), vectorToSave.end());
    vectorToSave.insert(vectorToSave.begin(), stringToInsert);

    DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(params.settingsKeyCount).AsInt32();
    DAVA::uint32 size = DAVA::Min((DAVA::uint32)vectorToSave.size(), recentFilesMaxCount);

    DAVA::KeyedArchive* archive = new DAVA::KeyedArchive();
    for (DAVA::uint32 i = 0; i < size; ++i)
    {
        archive->SetString(DAVA::Format("%d", i), vectorToSave[i]);
    }
    SettingsManager::SetValue(params.settingsKeyData, DAVA::VariantType(archive));
    SafeRelease(archive);
}

DAVA::Vector<DAVA::String> RecentMenuItems::Get() const
{
    DAVA::Vector<DAVA::String> retVector;
    DAVA::VariantType recentFilesVariant = SettingsManager::GetValue(params.settingsKeyData);
    if (recentFilesVariant.GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
    {
        DAVA::KeyedArchive* archiveRecentFiles = recentFilesVariant.AsKeyedArchive();
        DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(params.settingsKeyCount).AsInt32();
        DAVA::uint32 size = DAVA::Min(archiveRecentFiles->Count(), recentFilesMaxCount);
        retVector.resize(size);
        for (DAVA::uint32 i = 0; i < size; ++i)
        {
            retVector[i] = archiveRecentFiles->GetString(DAVA::Format("%d", i));
        }
    }
    return retVector;
}
