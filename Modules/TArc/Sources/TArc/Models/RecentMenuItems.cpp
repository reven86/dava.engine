#include "TArc/Models/RecentMenuItems.h"

#include "TArc/WindowSubSystem/QtAction.h"

#include "FileSystem/KeyedArchive.h"

#include "Utils/StringFormat.h"

#include <QMenu>
#include <QAction>

namespace RecentMenuItemsDetails
{
const DAVA::String recentItemsKey = "recent items";
}

RecentMenuItems::RecentMenuItems(Params&& params_)
    : params(std::move(params_))
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

        connections.AddConnection(action, &QAction::triggered, [path, this]()
                                  {
                                      actionTriggered.Emit(path);
                                  });

        DAVA::TArc::ActionPlacementInfo placement(DAVA::TArc::CreateMenuPoint(params.menuSubPath));
        params.ui->AddAction(params.windowKey, placement, action);
    }
}

void RecentMenuItems::AddInternal(const DAVA::String& recent)
{
    using namespace DAVA;
    Vector<String> vectorToSave = Get();

    FilePath filePath(recent);
    String stringToInsert = filePath.GetAbsolutePathname();

    //check present set to avoid duplicates
    vectorToSave.erase(std::remove(vectorToSave.begin(), vectorToSave.end(), stringToInsert), vectorToSave.end());
    vectorToSave.insert(vectorToSave.begin(), stringToInsert);

    uint32 recentFilesMaxCount = params.getMaximumCount();
    uint32 size = Min((uint32)vectorToSave.size(), recentFilesMaxCount);

    vectorToSave.resize(size);
    TArc::PropertiesItem item = params.accessor->CreatePropertiesNode(params.propertiesItemKey);
    item.Set(RecentMenuItemsDetails::recentItemsKey, vectorToSave);
}

DAVA::Vector<DAVA::String> RecentMenuItems::Get() const
{
    using namespace DAVA;
    TArc::PropertiesItem item = params.accessor->CreatePropertiesNode(params.propertiesItemKey);
    Vector<String> retVector = item.Get<Vector<String>>(RecentMenuItemsDetails::recentItemsKey);
    uint32 recentFilesMaxCount = params.getMaximumCount();
    uint32 size = Min(static_cast<uint32>(retVector.size()), recentFilesMaxCount);
    retVector.resize(size);
    return retVector;
}

RecentMenuItems::Params::Params(const DAVA::TArc::WindowKey& windowKey_, DAVA::TArc::ContextAccessor* accessor_, const DAVA::String& propertiesItemKey_)
    : windowKey(windowKey_)
    , accessor(accessor_)
    , propertiesItemKey(propertiesItemKey_)
{
}
