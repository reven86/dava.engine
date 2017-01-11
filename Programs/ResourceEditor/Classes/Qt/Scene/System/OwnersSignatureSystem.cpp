#include "OwnersSignatureSystem.h"
#include "Classes/StringConstants.h"
#include "Settings/SettingsManager.h"
#include "Scene/SceneSignals.h"

namespace OwnersSignatureSystemInternal
{
static const DAVA::Array<DAVA::int32, 2> validIDs =
{
  { CMDID_ENTITY_ADD, CMDID_ENTITY_CHANGE_PARENT }
};

bool IsCommandIdValid(DAVA::int32 _id)
{
    DAVA::uint32 count = static_cast<DAVA::uint32>(validIDs.size());
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        if (validIDs[i] == _id)
        {
            return true;
        }
    }
    return false;
}

DAVA::String GetCurrentTime()
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);

    DAVA::String timeString = DAVA::Format("%04d.%02d.%02d_%02d_%02d_%02d",
                                           utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
                                           utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);

    return timeString;
}

void UpdateOwner(DAVA::Entity* entity)
{
    DAVA::KeyedArchive* properties = GetCustomPropertiesArchieve(entity);
    if (nullptr != properties)
    {
        properties->SetString(ResourceEditor::SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, SettingsManager::GetValue(Settings::General_DesinerName).AsString());
        properties->SetString(ResourceEditor::SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, GetCurrentTime());
    }
}
}

OwnersSignatureSystem::OwnersSignatureSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::TRANSFORM_PARENT_CHANGED);
}

void OwnersSignatureSystem::AddEntity(DAVA::Entity* entity)
{
    OwnersSignatureSystemInternal::UpdateOwner(entity);
}

void OwnersSignatureSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    switch (event)
    {
    case DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED:
    case DAVA::EventSystem::TRANSFORM_PARENT_CHANGED:
    {
        OwnersSignatureSystemInternal::UpdateOwner(component->GetEntity());
        break;
    }
    default:
        break;
    }
}
