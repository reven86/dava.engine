#pragma once

#include "Base/BaseTypes.h"

class QObject;
class QAction;

namespace DAVA
{
class FastName;
}

namespace PreferencesActionsFactory
{
QAction* CreateActionForPreference(const DAVA::FastName& className, const DAVA::FastName& propertyName, QObject* parent);
};