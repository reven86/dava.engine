#include "TArc/Models/RecentMenuItems.h"

#include "TArc/WindowSubSystem/QtAction.h"

#include "FileSystem/KeyedArchive.h"

#include "Utils/StringFormat.h"

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
        params.ui->RemoveAction(params.windowKey, placement);
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

        QObject::connect(action, &QAction::triggered, [path, this]()
                         {
                             actionTriggered.Emit(path);
                         });

        DAVA::TArc::ActionPlacementInfo placement(DAVA::TArc::CreateMenuPoint(params.menuSubPath));
        params.ui->AddAction(params.windowKey, placement, action);
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

    DAVA::uint32 recentFilesMaxCount = params.getMaximumCount();
    DAVA::uint32 size = DAVA::Min((DAVA::uint32)vectorToSave.size(), recentFilesMaxCount);

    vectorToSave.resize(size);

    params.updateRecentFiles(vectorToSave);
}

DAVA::Vector<DAVA::String> RecentMenuItems::Get() const
{
    DAVA::Vector<DAVA::String> retVector = params.getRecentFiles();
    DAVA::uint32 recentFilesMaxCount = params.getMaximumCount();
    DAVA::uint32 size = DAVA::Min(static_cast<DAVA::uint32>(retVector.size()), recentFilesMaxCount);
    retVector.resize(size);
    return retVector;
}

extern DAVA::Vector<DAVA::String> ConvertKAToVector(const DAVA::KeyedArchive* archive)
{
    DAVA::Vector<DAVA::String> recentFiles;
    const DAVA::uint32 count = archive->Count();
    recentFiles.resize(count);
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        recentFiles[i] = archive->GetString(DAVA::Format("%d", i));
    }
    return recentFiles;
}

extern DAVA::KeyedArchive* ConvertVectorToKA(const DAVA::Vector<DAVA::String>& vector)
{
    DAVA::KeyedArchive* archive = new DAVA::KeyedArchive();
    for (DAVA::uint32 i = 0, count = static_cast<DAVA::uint32>(vector.size()); i < count; ++i)
    {
        archive->SetString(DAVA::Format("%d", i), vector[i]);
    }
    return archive;
}
