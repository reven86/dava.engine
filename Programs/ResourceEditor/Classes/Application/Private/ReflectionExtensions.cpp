#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/PropertyPanel/RenderObjectExtensions.h"
#include "Classes/PropertyPanel/NMaterialExtensions.h"
#include "Classes/PropertyPanel/FilePathExtensions.h"

#include <Scene3D/Components/RenderComponent.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/Landscape.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>

namespace ReflectoinExtensionsDetail
{
using namespace DAVA;

template <typename T, typename TMeta, typename TIndex>
void EmplaceTypeMeta(Meta<TMeta, TIndex>&& meta)
{
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<T>());
    DVASSERT(type != nullptr);

    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    if (structure->meta == nullptr)
    {
        structure->meta.reset(new ReflectedMeta());
    }

    structure->meta->Emplace(std::move(meta));
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

void RegisterReflectionExtensions(DAVA::TArc::ContextAccessor* accessor)
{
    RegisterRenderComponentExtensions();
    RegisterNMaterialExtensions();
    RegFilePathExt(accessor);
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
