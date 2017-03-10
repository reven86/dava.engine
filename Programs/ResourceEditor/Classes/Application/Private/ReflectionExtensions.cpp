#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/PropertyPanel/RenderObjectExtensions.h"
#include "Classes/PropertyPanel/NMaterialExtensions.h"
#include "Classes/PropertyPanel/FilePathExtensions.h"
#include "Classes/PropertyPanel/ComponentExtensions.h"

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

namespace ReflectoinExtensionsDetail
{
using namespace DAVA;

template <typename TMeta, typename TIndex>
void EmplaceTypeMeta(ReflectedType* type, Meta<TMeta, TIndex>&& meta)
{
    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    if (structure->meta == nullptr)
    {
        structure->meta.reset(new ReflectedMeta());
    }

    structure->meta->Emplace(std::move(meta));
}

template <typename T, typename TMeta, typename TIndex>
void EmplaceTypeMeta(Meta<TMeta, TIndex>&& meta)
{
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<T>());
    DVASSERT(type != nullptr);

    EmplaceTypeMeta(type, std::move(meta));
}

template <typename T, typename TMeta, typename TIndex>
void EmplaceFieldMeta(const String& fieldName, Meta<TMeta, TIndex>&& meta)
{
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<T>());
    DVASSERT(type != nullptr);

    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    for (std::unique_ptr<ReflectedStructure::Field>& field : structure->fields)
    {
        if (field->name == fieldName)
        {
            if (field->meta == nullptr)
            {
                field->meta.reset(new ReflectedMeta());
            }

            field->meta->Emplace(std::move(meta));
            break;
        }
    }
}

void RegisterRenderComponentExtensions()
{
    EmplaceTypeMeta<RenderObject>(CreateRenderObjectCommandProducer());
    EmplaceTypeMeta<RenderBatch>(CreateRenderBatchCommandProducer());
}

void RegisterNMaterialExtensions()
{
    EmplaceFieldMeta<RenderBatch>("material", CreateNMaterialCommandProducer());
}

void RegFilePathExt(DAVA::TArc::ContextAccessor* accessor)
{
    using namespace DAVA;
    // HeightMap
    EmplaceFieldMeta<Landscape>("heightmapPath", CreateHeightMapValidator(accessor));
    EmplaceFieldMeta<Landscape>("heightmapPath", CreateHeightMapFileMeta(accessor));
    EmplaceFieldMeta<VegetationRenderObject>("lightmap", CreateTextureValidator(accessor));
    EmplaceFieldMeta<VegetationRenderObject>("lightmap", CreateTextureFileMeta(accessor));
    EmplaceFieldMeta<VegetationRenderObject>("customGeometry", CreateSceneValidator(accessor));
    EmplaceFieldMeta<VegetationRenderObject>("customGeometry", CreateSceneFileMeta(accessor));
}

void RegComponentsExtensions()
{
    using namespace DAVA;
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

        M::CommandProducerHolder holder;
        holder.AddCommandProducer(CreateRemoveComponentProducer());
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

void RegisterReflectionExtensions(DAVA::TArc::ContextAccessor* accessor)
{
    RegisterRenderComponentExtensions();
    RegisterNMaterialExtensions();
    RegFilePathExt(accessor);
    RegComponentsExtensions();
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
