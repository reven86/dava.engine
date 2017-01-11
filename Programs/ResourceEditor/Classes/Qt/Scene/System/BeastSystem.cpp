#include "BeastSystem.h"

#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

BeastSystem::BeastSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void BeastSystem::AddEntity(DAVA::Entity* entity)
{
    SetDefaultPropertyValues(entity);
}

void BeastSystem::SetDefaultPropertyValues(DAVA::Entity* entity)
{
    DAVA::KeyedArchive* propertyList = GetOrCreateCustomProperties(entity)->GetArchive();

    if (GetLight(entity))
    {
        SetBool(propertyList, "editor.staticlight.enable", true);

        SetFloat(propertyList, "editor.intensity", 1.f);

        SetFloat(propertyList, "editor.staticlight.shadowangle", 0.f);
        SetFloat(propertyList, "editor.staticlight.shadowradius", 0.f);
        SetInt32(propertyList, "editor.staticlight.shadowsamples", 1);
        SetFloat(propertyList, "editor.staticlight.falloffcutoff", 1000.f);
        SetFloat(propertyList, "editor.staticlight.falloffexponent", 1.f);
    }
}

void BeastSystem::SetBool(DAVA::KeyedArchive* propertyList, const DAVA::String& key, bool value)
{
    if (!propertyList->IsKeyExists(key))
    {
        propertyList->SetBool(key, value);
    }
}

void BeastSystem::SetFloat(DAVA::KeyedArchive* propertyList, const DAVA::String& key, DAVA::float32 value)
{
    if (!propertyList->IsKeyExists(key))
    {
        propertyList->SetFloat(key, value);
    }
}

void BeastSystem::SetInt32(DAVA::KeyedArchive* propertyList, const DAVA::String& key, DAVA::int32 value)
{
    if (!propertyList->IsKeyExists(key))
    {
        propertyList->SetInt32(key, value);
    }
}
