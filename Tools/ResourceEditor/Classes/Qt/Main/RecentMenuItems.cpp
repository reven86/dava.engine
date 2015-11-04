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


#include "Main/RecentMenuItems.h"
#include "Settings/SettingsManager.h"

#include "FileSystem/KeyedArchive.h"

#include <QMenu>
#include <QAction>

RecentMenuItems::RecentMenuItems(const DAVA::FastName & _settingsKeyCount, const DAVA::FastName & _settingsKeyData)
    :   menu(nullptr)
    ,   settingsKeyCount(_settingsKeyCount)
    ,   settingsKeyData(_settingsKeyData)
{
}

void RecentMenuItems::SetMenu(QMenu *_menu)
{
    DVASSERT(_menu);
    menu = _menu;
}

void RecentMenuItems::Add(const DAVA::String & recent)
{
    RemoveMenuItems();

    AddInternal(recent);
    
    InitMenuItems();
}


void RecentMenuItems::RemoveMenuItems()
{
    DVASSERT(menu);
    
    while(actions.size())
    {
        menu->removeAction(actions[0]);
        actions.removeAt(0);
    }
}

void RecentMenuItems::InitMenuItems()
{
    DVASSERT(menu);
    
    auto pathList = Get();
    for(auto & path : pathList)
    {
        if (path.empty())
        {
            continue;
        }
        
        QString pathQt = QString::fromStdString(path);
        QAction *action = menu->addAction(pathQt);
        action->setData(pathQt);
        
        actions.push_back(action);
    }

    bool hasActions = (menu->actions().size() != 0);
    menu->setEnabled(hasActions);
}

void RecentMenuItems::EnableMenuItems(bool enabled)
{
    for(auto act : actions)
    {
        act->setEnabled(enabled);
    }
}

DAVA::String RecentMenuItems::GetItem(const QAction *action) const
{
    for(auto act : actions)
    {
        if(act == action)
        {
            return act->data().toString().toStdString();
        }
    }
    
    return DAVA::String();
}


void RecentMenuItems::AddInternal(const DAVA::String & recent)
{
    DAVA::Vector<DAVA::String> vectorToSave = Get();

    DAVA::FilePath filePath(recent);
    DAVA::String stringToInsert = filePath.GetAbsolutePathname();
    
    //check present set to avoid duplicates
    vectorToSave.erase(std::remove(vectorToSave.begin(), vectorToSave.end(), stringToInsert), vectorToSave.end());
    vectorToSave.insert(vectorToSave.begin(), stringToInsert);
    
    DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(settingsKeyCount).AsInt32();
    DAVA::uint32 size = DAVA::Min((DAVA::uint32)vectorToSave.size(), recentFilesMaxCount);
  
    DAVA::KeyedArchive * archive = new DAVA::KeyedArchive();
    for (DAVA::uint32 i = 0; i < size; ++i)
    {
        archive->SetString(DAVA::Format("%d",i), vectorToSave[i]);
    }
    SettingsManager::SetValue(settingsKeyData, DAVA::VariantType(archive));
    SafeRelease(archive);
}

DAVA::Vector<DAVA::String> RecentMenuItems::Get() const
{
    DAVA::Vector<DAVA::String> retVector;
    DAVA::VariantType recentFilesVariant = SettingsManager::GetValue(settingsKeyData);
    if(recentFilesVariant.GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
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

