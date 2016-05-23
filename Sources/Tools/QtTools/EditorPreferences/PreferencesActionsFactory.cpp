#include "Base/FastName.h"
#include "Base/Introspection.h"
#include "QtTools/EditorPreferences/PreferencesActionsFactory.h"
#include "Preferences/PreferencesStorage.h"
#include "QtTools/EditorPreferences/Actions/Actions.h"

QAction* PreferencesActionsFactory::CreateActionForPreference(const DAVA::FastName& className, const DAVA::FastName& propertyName, QObject* parent)
{
    const DAVA::InspInfo* inspInfo = PreferencesStorage::Instance()->GetInspInfo(className);
    const DAVA::InspMember* inspMember = inspInfo->Member(propertyName);
    DVASSERT(inspInfo != nullptr && inspMember != nullptr);
    const DAVA::MetaInfo* metaInfo = inspMember->Type();
    AbstractAction* action = nullptr;
    if (metaInfo == DAVA::MetaInfo::Instance<bool>())
    {
        action = new BoolAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::int32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::int64>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::uint64>())
    {
        action = new IntAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::float32>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::float64>())
    {
        action = new DoubleAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::String>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::WideString>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::FastName>()
             || metaInfo == DAVA::MetaInfo::Instance<DAVA::FilePath>())
    {
        action = new StringAction(inspMember, parent);
    }
    else if (metaInfo == DAVA::MetaInfo::Instance<DAVA::Color>())
    {
        action = new ColorAction(inspMember, parent);
    }
    else
    {
        DVASSERT(nullptr != action && "please create delegate for type");
        return nullptr;
    }
    action->Init();
    return action;
}
