#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/PropertyPanel/RenderObjectExtensions.h"
#include "Classes/PropertyPanel/NMaterialExtensions.h"
#include "Classes/PropertyPanel/FilePathExtensions.h"
#include "Classes/PropertyPanel/ComponentExtensions.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/CommonFieldNames.h>
#include <TArc/Utils/ReflectionHelpers.h>

#include <Entity/Component.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/ActionComponent.h>
#include <Scene3D/Components/SoundComponent.h>
#include <Scene3D/Components/WaveComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/Vegetation/VegetationRenderObject.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>

namespace ReflectoinExtensionsDetail
{
using namespace DAVA;
using namespace DAVA::TArc;

void RegisterRenderComponentExtensions()
{
    EmplaceTypeMeta<RenderObject>(CreateRenderObjectCommandProducer());
    EmplaceTypeMeta<RenderBatch>(CreateRenderBatchCommandProducer());
}

void RegisterNMaterialExtensions()
{
    EmplaceFieldMeta<RenderBatch>(FastName("material"), CreateNMaterialCommandProducer());
}

void RegisterFilePathExtensions(DAVA::TArc::ContextAccessor* accessor)
{
    // HeightMap
    EmplaceFieldMeta<Landscape>(FastName("heightmapPath"), CreateHeightMapValidator(accessor));
    EmplaceFieldMeta<Landscape>(FastName("heightmapPath"), CreateHeightMapFileMeta(accessor));
    EmplaceFieldMeta<VegetationRenderObject>(FastName("lightmap"), CreateTextureValidator(accessor));
    EmplaceFieldMeta<VegetationRenderObject>(FastName("lightmap"), CreateTextureFileMeta(accessor));
    EmplaceFieldMeta<VegetationRenderObject>(FastName("customGeometry"), CreateSceneValidator(accessor));
    EmplaceFieldMeta<VegetationRenderObject>(FastName("customGeometry"), CreateSceneFileMeta(accessor));
}

void RegisterComponentsExtensions()
{
    const Type* transformComponent = Type::Instance<TransformComponent>();
    const Type* actionComponent = Type::Instance<ActionComponent>();
    const Type* soundComponent = Type::Instance<SoundComponent>();
    const Type* waveComponent = Type::Instance<WaveComponent>();
    const Type* componentType = Type::Instance<Component>();
    const Vector<TypeInheritance::Info> derivedTypes = componentType->GetInheritance()->GetDerivedTypes();
    for (const TypeInheritance::Info& derived : derivedTypes)
    {
        if (derived.type == transformComponent)
        {
            continue;
        }

        ReflectedType* refType = const_cast<ReflectedType*>(ReflectedTypeDB::GetByType(derived.type));
        if (refType == nullptr)
        {
            DVASSERT(false, "We has component that derived from DAVA::Component, but without created ReflectedType");
        }

        const ReflectedStructure* structure = refType->GetStructure();
        DVASSERT(structure != nullptr, "Somebody has forgotten to declare reflected structure for component");

        M::CommandProducerHolder holder;
        if (structure->meta == nullptr || structure->meta->GetMeta<M::CantBeDeletedManualyComponent>() == nullptr)
        {
            holder.AddCommandProducer(CreateRemoveComponentProducer());
        }

        if (derived.type == actionComponent)
        {
            holder.AddCommandProducer(CreateActionsEditProducer());
        }
        else if (derived.type == soundComponent)
        {
            holder.AddCommandProducer(CreateSoundsEditProducer());
        }
        else if (derived.type == waveComponent)
        {
            holder.AddCommandProducer(CreateWaveTriggerProducer());
        }

        EmplaceTypeMeta(refType, std::move(holder));
    }
}

String GetSceneName(DAVA::TArc::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScenePath().GetBasename();
}

String GetScenePath(DAVA::TArc::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScenePath().GetAbsolutePathname();
}

bool IsSceneChanged(DAVA::TArc::DataContext* ctx)
{
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    return data->GetScene()->IsChanged();
}

void RegisterDataContextExtensions()
{
    using namespace DAVA::TArc;
    ReflectionRegistrator<DataContext>::Begin()
    .Field(ContextNameFieldName, &GetSceneName, nullptr)
    .Field(ContextToolTipFieldName, &GetScenePath, nullptr)
    .Field(IsContextModifiedFieldName, &IsSceneChanged, nullptr)
    .End();
}

void RegisterReflectionExtensions(DAVA::TArc::ContextAccessor* accessor)
{
    RegisterRenderComponentExtensions();
    RegisterNMaterialExtensions();
    RegisterFilePathExtensions(accessor);
    RegisterComponentsExtensions();
    RegisterDataContextExtensions();
}
} // namespace ReflectoinExtensionsDetail

void ReflectionExtensionsModule::PostInit()
{
    using namespace ReflectoinExtensionsDetail;
    RegisterReflectionExtensions(GetAccessor());
}

DAVA_VIRTUAL_REFLECTION_IMPL(ReflectionExtensionsModule)
{
    DAVA::ReflectionRegistrator<ReflectionExtensionsModule>::Begin()
    .ConstructorByPointer()
    .End();
}
