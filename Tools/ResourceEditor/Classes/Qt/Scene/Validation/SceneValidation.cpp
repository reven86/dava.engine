#include "SceneValidation.h"

namespace SceneValidation
{
using namespace DAVA;

namespace Details
{
void CollectEntitiesByName(const Entity* entity, MultiMap<FastName, const Entity*>& container)
{
    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        CollectEntitiesByName(entity->GetChild(i), container);
    }

    container.emplace(std::make_pair(entity->GetName(), entity));
}

void CompareCustomProperties(const Entity* entity1, const Entity* entity2)
{
    DVASSERT(entity1 != nullptr);
    DVASSERT(entity2 != nullptr);

    KeyedArchive* props1 = GetCustomPropertiesArchieve(entity1);
    KeyedArchive* props2 = GetCustomPropertiesArchieve(entity2);

    if (props1 == nullptr)
    {
        Logger::Warning("Entity '%s' (id=%u) hasn't custom properties", entity1->GetName().c_str(), entity1->GetID());
        return;
    }
    if (props2 == nullptr)
    {
        Logger::Warning("Entity '%s' (id=%u) hasn't custom properties", entity2->GetName().c_str(), entity2->GetID());
        return;
    }

    bool hasMismatches = false;

    static const char* CHECKED_PROPERTIES[] = { "CollisionType", "CollisionTypeCrashed", "editor.ReferenceToOwner", "Health", "MaterialKind" };

    for (const char* checkedProperty : CHECKED_PROPERTIES)
    {
        bool entity1HasProperty = props1->IsKeyExists(checkedProperty);
        bool entity2HasProperty = props2->IsKeyExists(checkedProperty);

        if (entity1HasProperty != entity2HasProperty)
        {
            hasMismatches = true;
            Logger::Warning("Entity '%s' (id=%u) %s property '%s' while entity '%s' (id=%u) %s",
                            entity1->GetName().c_str(), entity1->GetID(),
                            (entity1HasProperty ? "has" : "hasn't"),
                            checkedProperty,
                            entity2->GetName().c_str(), entity2->GetID(),
                            (entity2HasProperty ? "has" : "hasn't"));
        }
        else if (entity1HasProperty && entity2HasProperty)
        {
            VariantType* entity1Value = props1->GetVariant(checkedProperty);
            VariantType* entity2Value = props2->GetVariant(checkedProperty);

            if (*entity1Value != *entity2Value)
            {
                hasMismatches = true;
                Logger::Warning("Property '%s' values are different for entity '%s' (id=%u) and entity '%s' (id=%u)",
                                checkedProperty,
                                entity1->GetName().c_str(), entity1->GetID(),
                                entity2->GetName().c_str(), entity2->GetID());
            }
        }
    }
}

void CompareSoundComponents(const Entity* entity1, const Entity* entity2)
{
    DVASSERT(entity1 != nullptr);
    DVASSERT(entity2 != nullptr);

    SoundComponent* soundComponent1 = GetSoundComponent(entity1);
    SoundComponent* soundComponent2 = GetSoundComponent(entity2);

    bool soundsAreEqual = true;

    if ((soundComponent1 == nullptr) != (soundComponent2 == nullptr))
    {
        soundsAreEqual = false;
    }
    else if (soundComponent1 != nullptr && soundComponent2 != nullptr)
    {
        if (soundComponent1->GetEventsCount() != soundComponent2->GetEventsCount())
        {
            soundsAreEqual = false;
        }
        else if (soundComponent1->GetEventsCount() > 0)
        {
            uint32 eventsCount = soundComponent1->GetEventsCount();
            for (uint32 iEvent = 0; iEvent < eventsCount; ++iEvent)
            {
                SoundEvent* soundEvent1 = soundComponent1->GetSoundEvent(iEvent);
                SoundEvent* soundEvent2 = soundComponent2->GetSoundEvent(iEvent);

                if (soundEvent1->GetEventName() != soundEvent2->GetEventName())
                {
                    soundsAreEqual = false;
                    break;
                }

                if (soundEvent1->GetMaxDistance() != soundEvent2->GetMaxDistance())
                {
                    soundsAreEqual = false;
                    break;
                }

                Vector<SoundEvent::SoundEventParameterInfo> eventParams1, eventParams2;
                soundEvent1->GetEventParametersInfo(eventParams1);
                soundEvent2->GetEventParametersInfo(eventParams2);

                if (eventParams1.size() != eventParams2.size())
                {
                    soundsAreEqual = false;
                    break;
                }

                auto paramLess = [](const SoundEvent::SoundEventParameterInfo& param1, const SoundEvent::SoundEventParameterInfo& param2)
                {
                    return param1.name < param2.name;
                };

                auto paramEqual = [](const SoundEvent::SoundEventParameterInfo& param1, const SoundEvent::SoundEventParameterInfo& param2)
                {
                    return (param1.name == param2.name
                            && param1.maxValue == param2.maxValue
                            && param1.minValue == param2.minValue);
                };

                std::sort(eventParams1.begin(), eventParams1.end(), paramLess);
                std::sort(eventParams2.begin(), eventParams2.end(), paramLess);
                auto firstDiff = std::mismatch(eventParams1.begin(), eventParams1.end(), eventParams2.begin(), paramEqual);

                bool vectorsAreDifferent = (firstDiff.first != eventParams1.end());
                if (vectorsAreDifferent)
                {
                    soundsAreEqual = false;
                    break;
                }

                for (SoundEvent::SoundEventParameterInfo& param : eventParams1)
                {
                    if (soundEvent1->GetParameterValue(FastName(param.name)) != soundEvent2->GetParameterValue(FastName(param.name)))
                    {
                        soundsAreEqual = false;
                        break;
                    }
                }

                if (!soundsAreEqual)
                    break;
            }
        }
    }

    if (!soundsAreEqual)
    {
        Logger::Warning("Entity '%s' (id=%u) and entity '%s' (id=%u) sound components are different",
                        entity1->GetName().c_str(), entity1->GetID(),
                        entity2->GetName().c_str(), entity2->GetID());
    }
}

Vector<Entity*> GetChildEffectEntities(const Entity* entity)
{
    Vector<Entity*> container;

    for (Entity* child : entity->children)
    {
        if (child->GetComponentCount(Component::PARTICLE_EFFECT_COMPONENT) > 0)
        {
            container.push_back(child);
        }
    }

    return container;
}

void CompareEffects(const Entity* entity1, const Entity* entity2)
{
    DVASSERT(entity1 != nullptr);
    DVASSERT(entity2 != nullptr);

    bool effectsAreEqual = true;

    Vector<Entity*> childEffects1 = GetChildEffectEntities(entity1);
    Vector<Entity*> childEffects2 = GetChildEffectEntities(entity2);

    auto paramLess = [](const Entity* entity1, const Entity* entity2)
    {
        return (entity1->GetName() < entity2->GetName());
    };

    auto paramEqual = [](const Entity* entity1, const Entity* entity2)
    {
        return (entity1->GetName() == entity2->GetName());
    };

    std::sort(childEffects1.begin(), childEffects1.end(), paramLess);
    std::sort(childEffects2.begin(), childEffects2.end(), paramLess);
    auto firstDiff = std::mismatch(childEffects1.begin(), childEffects1.end(), childEffects2.begin(), paramEqual);

    bool vectorsAreDifferent = (firstDiff.first != childEffects1.end());
    if (vectorsAreDifferent)
    {
        Logger::Warning("Entity '%s' (id=%u) and entity '%s' (id=%u) have different effects",
                        entity1->GetName().c_str(), entity1->GetID(),
                        entity2->GetName().c_str(), entity2->GetID());
    }
}

const ProjectManager::AvailableMaterialTemplate* GetTemplateByPath(const QVector<ProjectManager::AvailableMaterialTemplate>* materialTemplates, const FastName& materialTemplatePath)
{
    for (const ProjectManager::AvailableMaterialTemplate templ : *materialTemplates)
    {
        if (0 == strcmp(templ.path.toStdString().c_str(), materialTemplatePath.c_str()))
        {
            return &templ;
        }
    }

    return nullptr;
}

bool IsKnownMaterialQualityGroup(const FastName& materialGroup)
{
    size_t qcount = QualitySettingsSystem::Instance()->GetMaterialQualityGroupCount();
    for (size_t q = 0; q < qcount; ++q)
    {
        if (materialGroup == QualitySettingsSystem::Instance()->GetMaterialQualityGroupName(q))
        {
            return true;
        }
    }

    return false;
}

bool IsAssignableMaterialTemplate(const FastName& materialTemplatePath)
{
    return materialTemplatePath != DAVA::NMaterialName::SHADOW_VOLUME;
}

int32 GetCollisionTypeID(const char* collisionTypeName)
{
    const Vector<String>& collisionTypes = EditorConfig::Instance()->GetComboPropertyValues("CollisionType");

    for (int32 i = 0; i < collisionTypes.size(); ++i)
    {
        if (collisionTypes[i] == collisionTypeName)
        {
            return i;
        }
    }

    return -1;
}

String MaterialPrettyName(NMaterial* material)
{
    DVASSERT(material);

    NMaterial* parent = material->GetParent();

    String prettyName = Format("'%s'", material->GetMaterialName().c_str());
    if (parent)
    {
        prettyName.append(Format(" (parent '%s')", parent->GetMaterialName().c_str()));
    }

    return prettyName;
}

} // namespace Details

void ValidateMatrices(SceneEditor2* scene)
{
    DVASSERT(scene);

    Vector<Entity*> container;
    scene->GetChildEntitiesWithCondition(container, [](Entity* entity)
                                         {
                                             KeyedArchive* props = GetCustomPropertiesArchieve(entity);
                                             return (props != nullptr && props->IsKeyExists("editor.referenceToOwner"));
                                         });

    UnorderedSet<String> checkedScenes;

    for (const Entity* entity : container)
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(entity);
        DVASSERT(props);

        const String pathToSourceScene = props->GetString("editor.referenceToOwner");

        auto insertResult = checkedScenes.insert(pathToSourceScene);
        bool wasAlreadyInSet = (insertResult.second == false);
        if (wasAlreadyInSet)
            continue;

        ScopedPtr<Scene> sourceScene(new Scene);
        SceneFileV2::eError result = sourceScene->LoadScene(pathToSourceScene);
        if (result != SceneFileV2::eError::ERROR_NO_ERROR)
        {
            Logger::Warning("Can't load source model %s", pathToSourceScene.c_str());
            continue;
        }

        DVASSERT(sourceScene->GetChildrenCount() == 1);

        const Matrix4& localMatrix = sourceScene->GetChild(0)->GetLocalTransform();
        const Matrix4& worldMatrix = sourceScene->GetChild(0)->GetWorldTransform();

        if (localMatrix != Matrix4::IDENTITY)
        {
            Logger::Warning("Source model '%s' local transform is not an identity matrix", pathToSourceScene.c_str());
        }

        if (worldMatrix != Matrix4::IDENTITY)
        {
            Logger::Warning("Source model '%s' world transform is not an identity matrix", pathToSourceScene.c_str());
        }
    }

    Logger::Info("Validating matrices is done");
}

void ValidateSameNames(SceneEditor2* scene)
{
    DVASSERT(scene);

    MultiMap<FastName, const Entity*> entitiesByName;
    Details::CollectEntitiesByName(scene, entitiesByName);

    MultiMap<FastName, const Entity*>::const_iterator currentIter = entitiesByName.begin();

    while (currentIter != entitiesByName.end())
    {
        const FastName& currentKey = currentIter->first;

        auto rangePair = entitiesByName.equal_range(currentKey);
        if (std::distance(rangePair.first, rangePair.second) > 1)
        {
            for (auto rangeNextIter = std::next(rangePair.first);
                 rangeNextIter != rangePair.second;
                 ++rangeNextIter)
            {
                Details::CompareCustomProperties(rangePair.first->second, rangeNextIter->second);
                Details::CompareSoundComponents(rangePair.first->second, rangeNextIter->second);
                Details::CompareEffects(rangePair.first->second, rangeNextIter->second);
            }
        }

        currentIter = rangePair.second;
    }

    Logger::Info("Validating same names is done");
}

void ValidateCollisionProperties(SceneEditor2* scene)
{
    DVASSERT(scene);

    int32 collisionTypeWaterId = Details::GetCollisionTypeID("Water");
    int32 collisionTypeSpeedTreeId = Details::GetCollisionTypeID("SpeedTree");

    Vector<Entity*> container;
    scene->GetChildEntitiesWithComponent(container, Component::CUSTOM_PROPERTIES_COMPONENT);

    for (const Entity* entity : container)
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(entity);
        DVASSERT(props);

        bool isWater = (props->GetInt32("CollisionType") == collisionTypeWaterId);
        bool isSpeedTree = (props->GetInt32("CollisionType") == collisionTypeSpeedTreeId);

        if (props->IsKeyExists("CollisionType")
            && !isWater
            && !isSpeedTree
            && !props->IsKeyExists("MaterialKind")
            && !props->IsKeyExists("FallType"))
        {
            Logger::Warning("Entity '%s' (id=%u) has 'CollisionType' property but hasn't 'MaterialKind' or 'FallType'",
                            entity->GetName().c_str(), entity->GetID());
        }
    }

    Logger::Info("Validating collision types is done");
}

void ValidateTexturesRelevance(SceneEditor2* scene)
{
    DVASSERT(scene);

    SceneHelper::TextureCollector collector;
    SceneHelper::EnumerateSceneTextures(scene, collector);
    TexturesMap& texturesMap = collector.GetTextures();

    for (const std::pair<FilePath, Texture*>& entry : texturesMap)
    {
        DAVA::TextureDescriptor* descriptor = entry.second->texDescriptor;
        if (nullptr != descriptor && DAVA::FileSystem::Instance()->Exists(descriptor->pathname))
        {
            for (uint32 i = 0; i < eGPUFamily::GPU_ORIGIN; ++i)
            {
                eGPUFamily gpu = static_cast<eGPUFamily>(i);
                if (descriptor->HasCompressionFor(gpu))
                {
                    bool isRelevant = descriptor->IsCompressedTextureActual(gpu);

                    Logger::Instance()->Log((isRelevant ? Logger::LEVEL_DEBUG : Logger::LEVEL_WARNING),
                                            "Texture %s compression is %s for gpu %s",
                                            descriptor->GetSourceTexturePathname().GetFilename().c_str(),
                                            (isRelevant ? "relevant" : "not relevant"),
                                            GPUFamilyDescriptor::GetGPUName(gpu).c_str());
                }
            }
        }
    }

    Logger::Info("Validating textures relevance is done");
}

void ValidateMaterialsGroups(SceneEditor2* scene)
{
    DVASSERT(scene);

    Set<NMaterial*> materials;
    SceneHelper::BuildMaterialList(scene, materials);
    NMaterial* globalMaterial = scene->GetGlobalMaterial();
    if (nullptr != globalMaterial)
    {
        materials.erase(globalMaterial);
    }

    const QVector<ProjectManager::AvailableMaterialTemplate>* materialTemplates = nullptr;
    if (ProjectManager::Instance() != nullptr)
    {
        materialTemplates = ProjectManager::Instance()->GetAvailableMaterialTemplates();
        DVASSERT(materialTemplates);
    }

    for (NMaterial* material : materials)
    {
        const FastName& materialGroup = material->GetQualityGroup();
        bool qualityGroupIsSet = false;

        String materialName = Details::MaterialPrettyName(material);

        if (materialGroup.IsValid())
        {
            qualityGroupIsSet = Details::IsKnownMaterialQualityGroup(materialGroup);
            if (!qualityGroupIsSet)
            {
                Logger::Warning("Material %s has unknown quality group '%s'", materialName.c_str(), materialGroup.c_str());
            }
        }

        const FastName& materialTemplatePath = material->GetEffectiveFXName();
        if (materialTemplatePath.IsValid() && Details::IsAssignableMaterialTemplate(materialTemplatePath))
        {
            const ProjectManager::AvailableMaterialTemplate* materialTemplate = Details::GetTemplateByPath(materialTemplates, materialTemplatePath);
            if (materialTemplate)
            {
                if (!materialTemplate->qualities.empty() && !qualityGroupIsSet)
                {
                    Logger::Warning("Group is not selected for material %s with multi-quality template assigned to it", materialName.c_str());
                }
            }
            else
            {
                Logger::Warning("Material %s has unknown material template %s", materialName.c_str(), materialTemplatePath.c_str());
            }
        }
    }

    Logger::Info("Validating materials groups is done");
}
}
