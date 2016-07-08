#ifndef __RECENT_MENU_ITEMS_H__
#define __RECENT_MENU_ITEMS_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"

#include <QList>

class QMenu;
class QAction;

class RecentMenuItems
{
public:
    RecentMenuItems(RecentMenuItems& rmi) = delete;
    RecentMenuItems& operator=(RecentMenuItems& rmi) = delete;

    RecentMenuItems(const DAVA::FastName& settingsKeyCount, const DAVA::FastName& settingsKeyData);

    void SetMenu(QMenu* menu);
    void InitMenuItems();
    void EnableMenuItems(bool enabled);

    void Add(const DAVA::String& recent);
    DAVA::Vector<DAVA::String> Get() const;

    DAVA::String GetItem(const QAction* action) const;

private:
    void AddInternal(const DAVA::String& recent);
    void RemoveMenuItems();

    QList<QAction*> actions;
    QMenu* menu;

    const DAVA::FastName settingsKeyCount;
    const DAVA::FastName settingsKeyData;
};


#endif // __RECENT_MENU_ITEMS_H__
