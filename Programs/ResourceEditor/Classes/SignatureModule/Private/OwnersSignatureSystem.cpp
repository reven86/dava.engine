#include "Classes/SignatureModule/Private/OwnersSignatureSystem.h"
#include "Classes/StringConstants.h"

#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Systems/EventSystem.h>
#include <Scene3D/Scene.h>
#include <Utils/StringFormat.h>
#include <Time/DateTime.h>

#include <time.h>

OwnersSignatureSystem::OwnersSignatureSystem(DAVA::Scene* scene, const DAVA::String& userName)
    : SceneSystem(scene)
    , currentUserName(userName)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::TRANSFORM_PARENT_CHANGED);
}

void OwnersSignatureSystem::AddEntity(DAVA::Entity* entity)
{
    UpdateOwner(entity);
}

void OwnersSignatureSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    switch (event)
    {
    case DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED:
    case DAVA::EventSystem::TRANSFORM_PARENT_CHANGED:
    {
        UpdateOwner(component->GetEntity());
        break;
    }
    default:
        break;
    }
}

void OwnersSignatureSystem::UpdateOwner(DAVA::Entity* entity)
{
    if (systemIsEnabled == false)
    {
        return;
    }

    DAVA::KeyedArchive* properties = DAVA::GetCustomPropertiesArchieve(entity);
    if (nullptr != properties)
    {
        properties->SetString(ResourceEditor::SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, currentUserName);

        DAVA::DateTime now = DAVA::DateTime::Now();
        DAVA::String timeString = DAVA::Format("%04d.%02d.%02d_%02d_%02d_%02d",
                                               now.GetYear(), now.GetMonth() + 1, now.GetDay(),
                                               now.GetHour(), now.GetMinute(), now.GetSecond());

        properties->SetString(ResourceEditor::SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, timeString);
    }
}
