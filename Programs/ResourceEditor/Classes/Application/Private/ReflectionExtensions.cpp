#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/PropertyPanel/RenderObjectExtensions.h"
#include "Classes/PropertyPanel/NMaterialExtensions.h"

#include <Scene3D/Components/RenderComponent.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>

namespace ReflectoinExtensionsDetail
{
void RegisterRenderComponentOperations()
{
    using namespace DAVA;
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<RenderComponent>());
    DVASSERT(type != nullptr);

    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    for (std::unique_ptr<ReflectedStructure::Field>& field : structure->fields)
    {
        if (field->name == "renderObject")
        {
            if (field->meta == nullptr)
            {
                field->meta.reset(new ReflectedMeta());
            }

            field->meta->Emplace(CreateRenderObjectCommandProducer());
            break;
        }
    }
}

void RegisterNMaterialExtensions()
{
    using namespace DAVA;
    ReflectedType* type = const_cast<ReflectedType*>(ReflectedTypeDB::Get<RenderBatch>());
    DVASSERT(type != nullptr);

    ReflectedStructure* structure = type->GetStructure();
    DVASSERT(structure != nullptr);

    for (std::unique_ptr<ReflectedStructure::Field>& field : structure->fields)
    {
        if (field->name == "material")
        {
            if (field->meta == nullptr)
            {
                field->meta.reset(new ReflectedMeta());
            }

            field->meta->Emplace(CreateNMaterialCommandProducer());
            break;
        }
    }
}

} // namespace ReflectoinExtensionsDetail

void RegisterReflectionExtensions()
{
    using namespace ReflectoinExtensionsDetail;
    RegisterRenderComponentOperations();
    RegisterNMaterialExtensions();
}
