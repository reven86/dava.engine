#include "Main/RecentMenuItems.h"
#include "Settings/SettingsManager.h"

#include "FileSystem/KeyedArchive.h"

#include <QMenu>
#include <QAction>

RecentMenuItems::RecentMenuItems(const DAVA::FastName& _settingsKeyCount, const DAVA::FastName& _settingsKeyData)
    : menu(nullptr)
    , settingsKeyCount(_settingsKeyCount)
    , settingsKeyData(_settingsKeyData)
{
}

void RecentMenuItems::SetMenu(QMenu* _menu)
{
    DVASSERT(_menu);
    menu = _menu;
}

void RecentMenuItems::Add(const DAVA::String& recent)
{
    RemoveMenuItems();

    AddInternal(recent);

    InitMenuItems();
}

void RecentMenuItems::RemoveMenuItems()
{
    DVASSERT(menu);

    while (actions.size())
    {
        menu->removeAction(actions[0]);
        actions.removeAt(0);
    }
}

void RecentMenuItems::InitMenuItems()
{
    DVASSERT(menu);

    auto pathList = Get();
    for (auto& path : pathList)
    {
        if (path.empty())
        {
            continue;
        }

        QString pathQt = QString::fromStdString(path);
        QAction* action = menu->addAction(pathQt);
        action->setData(pathQt);

        actions.push_back(action);
    }

    bool hasActions = (menu->actions().size() != 0);
    menu->setEnabled(hasActions);
}

void RecentMenuItems::EnableMenuItems(bool enabled)
{
    for (auto act : actions)
    {
        act->setEnabled(enabled);
    }
}

DAVA::String RecentMenuItems::GetItem(const QAction* action) const
{
    for (auto act : actions)
    {
        if (act == action)
        {
            return act->data().toString().toStdString();
        }
    }

    return DAVA::String();
}

void RecentMenuItems::AddInternal(const DAVA::String& recent)
{
    DAVA::Vector<DAVA::String> vectorToSave = Get();

    DAVA::FilePath filePath(recent);
    DAVA::String stringToInsert = filePath.GetAbsolutePathname();

    //check present set to avoid duplicates
    vectorToSave.erase(std::remove(vectorToSave.begin(), vectorToSave.end(), stringToInsert), vectorToSave.end());
    vectorToSave.insert(vectorToSave.begin(), stringToInsert);

    DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(settingsKeyCount).AsInt32();
    DAVA::uint32 size = DAVA::Min((DAVA::uint32)vectorToSave.size(), recentFilesMaxCount);

    DAVA::KeyedArchive* archive = new DAVA::KeyedArchive();
    for (DAVA::uint32 i = 0; i < size; ++i)
    {
        archive->SetString(DAVA::Format("%d", i), vectorToSave[i]);
    }
    SettingsManager::SetValue(settingsKeyData, DAVA::VariantType(archive));
    SafeRelease(archive);
}

DAVA::Vector<DAVA::String> RecentMenuItems::Get() const
{
    DAVA::Vector<DAVA::String> retVector;
    DAVA::VariantType recentFilesVariant = SettingsManager::GetValue(settingsKeyData);
    if (recentFilesVariant.GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
    {
        DAVA::KeyedArchive* archiveRecentFiles = recentFilesVariant.AsKeyedArchive();
        DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(settingsKeyCount).AsInt32();
        DAVA::uint32 size = DAVA::Min(archiveRecentFiles->Count(), recentFilesMaxCount);
        retVector.resize(size);
        for (DAVA::uint32 i = 0; i < size; ++i)
        {
            retVector[i] = archiveRecentFiles->GetString(DAVA::Format("%d", i));
        }
    }
    return retVector;
}
