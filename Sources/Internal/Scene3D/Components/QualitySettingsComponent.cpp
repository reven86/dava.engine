#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
namespace QualitySettingsComponentDetail
{
UnorderedMap<FastName, FastName> GetModelTypes(QualitySettingsComponent* component)
{
    static UnorderedMap<FastName, FastName> modelTypes;
    if (modelTypes.empty() == true)
    {
        modelTypes.emplace(FastName(), FastName("Undefined"));

        QualitySettingsSystem* inst = QualitySettingsSystem::Instance();
        for (int32 i = 0; i < inst->GetOptionsCount(); ++i)
        {
            FastName optionName = inst->GetOptionName(i);
            modelTypes.emplace(optionName, optionName);
        }
    }

    return modelTypes;
}

UnorderedMap<FastName, FastName> GetRequiredGroups(QualitySettingsComponent* component)
{
    static UnorderedMap<FastName, FastName> requiredGroups;
    if (requiredGroups.empty() == true)
    {
        requiredGroups.emplace(FastName(), FastName("Undefined"));

        QualitySettingsSystem* inst = QualitySettingsSystem::Instance();
        for (int32 i = 0; i < inst->GetMaterialQualityGroupCount(); ++i)
        {
            FastName groupName = inst->GetMaterialQualityGroupName(i);
            requiredGroups.emplace(groupName, groupName);
        }
    }

    return requiredGroups;
}

UnorderedMap<FastName, FastName> GetRequiredQualities(QualitySettingsComponent* component)
{
    static UnorderedMap<FastName, UnorderedMap<FastName, FastName>> requiredQualities;
    if (requiredQualities.empty() == true)
    {
        QualitySettingsSystem* inst = QualitySettingsSystem::Instance();
        for (int32 i = 0; i < inst->GetMaterialQualityGroupCount(); ++i)
        {
            FastName groupName = inst->GetMaterialQualityGroupName(i);
            UnorderedMap<FastName, FastName>& quality = requiredQualities[groupName];
            quality.emplace(FastName(), FastName("Undefined"));
            for (int32 i = 0; i < inst->GetMaterialQualityCount(groupName); ++i)
            {
                FastName qualityName = inst->GetMaterialQualityName(groupName, i);
                quality.emplace(qualityName, qualityName);
            }
        }
    }

    return requiredQualities[component->GetRequiredGroup()];
}
}

DAVA_VIRTUAL_REFLECTION_IMPL(QualitySettingsComponent)
{
    ReflectionRegistrator<QualitySettingsComponent>::Begin()
    .Field("filterByType", &QualitySettingsComponent::GetFilterByType, &QualitySettingsComponent::SetFilterByType)[M::DisplayName("Filter By Type")]
    .Field("modelType", &QualitySettingsComponent::GetModelType, &QualitySettingsComponent::SetModelType)[M::DisplayName("Model Type"), M::ValueEnumeratorField("modelTypes")]
    .Field("modelTypes", &QualitySettingsComponentDetail::GetModelTypes, nullptr)[M::HiddenField()]
    .Field("requiredGroup", &QualitySettingsComponent::GetRequiredGroup, &QualitySettingsComponent::SetRequiredGroup)[M::DisplayName("Required Group"), M::ValueEnumeratorField("requiredGroups")]
    .Field("requiredGroups", &QualitySettingsComponentDetail::GetRequiredGroups, nullptr)[M::HiddenField()]
    .Field("requiredQuality", &QualitySettingsComponent::GetRequiredQuality, &QualitySettingsComponent::SetRequiredQuality)[M::DisplayName("Required Quality"), M::ValueEnumeratorField("requiredQualities")]
    .Field("requiredQualities", &QualitySettingsComponentDetail::GetRequiredQualities, nullptr)[M::HiddenField()]
    .End();
}

QualitySettingsComponent::QualitySettingsComponent()
    : Component()
    , filterByType(true)
{
}

QualitySettingsComponent::~QualitySettingsComponent()
{
}

Component* QualitySettingsComponent::Clone(Entity* toEntity)
{
    QualitySettingsComponent* component = new QualitySettingsComponent();
    component->SetEntity(toEntity);

    component->filterByType = filterByType;
    component->modelType = modelType;
    component->requiredGroup = requiredGroup;
    component->requiredQuality = requiredQuality;

    return component;
}

void QualitySettingsComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (modelType.IsValid())
        archive->SetString("modelType", modelType.c_str());
    if (requiredGroup.IsValid())
        archive->SetString("requiredGroup", requiredGroup.c_str());
    if (requiredQuality.IsValid())
        archive->SetString("requiredQuality", requiredQuality.c_str());
    archive->SetBool("filterByType", filterByType);
}

void QualitySettingsComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive->IsKeyExists("modelType"))
        modelType = FastName(archive->GetString("modelType"));
    if (archive->IsKeyExists("requiredGroup"))
        requiredGroup = FastName(archive->GetString("requiredGroup"));
    if (archive->IsKeyExists("requiredQuality"))
        requiredQuality = FastName(archive->GetString("requiredQuality"));
    filterByType = archive->GetBool("filterByType", filterByType);

    Component::Deserialize(archive, serializationContext);
}

void QualitySettingsComponent::SetFilterByType(bool filter)
{
    filterByType = filter;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}
bool QualitySettingsComponent::GetFilterByType() const
{
    return filterByType;
}

void QualitySettingsComponent::SetModelType(const FastName& type)
{
    modelType = type;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}

const FastName& QualitySettingsComponent::GetModelType() const
{
    return modelType;
}

void QualitySettingsComponent::SetRequiredGroup(const FastName& group)
{
    requiredGroup = group;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}
const FastName& QualitySettingsComponent::GetRequiredGroup() const
{
    return requiredGroup;
}

void QualitySettingsComponent::SetRequiredQuality(const FastName& quality)
{
    requiredQuality = quality;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}
const FastName& QualitySettingsComponent::GetRequiredQuality() const
{
    return requiredQuality;
}
};
