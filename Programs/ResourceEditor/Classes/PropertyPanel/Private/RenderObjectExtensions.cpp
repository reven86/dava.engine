#include "Classes/PropertyPanel/RenderObjectExtensions.h"
#include "Classes/Commands2/ConvertToBillboardCommand.h"
#include "Classes/Commands2/CloneLastBatchCommand.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Selection/SelectableGroup.h"
#include "Classes/Deprecated/SceneValidator.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Render/Highlevel/RenderObject.h>
#include <Debug/DVAssert.h>

namespace RenderObjectExtensionsDetail
{
class BillboardCommandProducer : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const DAVA::Reflection& field) const override;
    Info GetInfo() const override;
    void CreateCache(DAVA::TArc::ContextAccessor* accessor) override;
    void ClearCache() override;
    std::unique_ptr<DAVA::Command> CreateCommand(const DAVA::Reflection& field, const Params& params) const override;

private:
    DAVA::UnorderedMap<DAVA::RenderObject*, DAVA::Entity*> cache;
};

bool BillboardCommandProducer::IsApplyable(const DAVA::Reflection& field) const
{
    using namespace DAVA;
    RenderObject* renderObject = *field.GetValueObject().GetPtr<RenderObject*>();
    RenderObject::eType type = renderObject->GetType();
    return type == RenderObject::TYPE_MESH || type == RenderObject::TYPE_RENDEROBJECT;
}

std::unique_ptr<DAVA::Command> BillboardCommandProducer::CreateCommand(const DAVA::Reflection& field, const Params& params) const
{
    using namespace DAVA;
    RenderObject* renderObject = *field.GetValueObject().GetPtr<RenderObject*>();
    DAVA::RenderObject::eType type = renderObject->GetType();
    DVASSERT(type == DAVA::RenderObject::TYPE_MESH || type == DAVA::RenderObject::TYPE_RENDEROBJECT);

    auto iter = cache.find(renderObject);
    DVASSERT(iter != cache.end());

    return std::make_unique<ConvertToBillboardCommand>(renderObject, iter->second);
}

void BillboardCommandProducer::CreateCache(DAVA::TArc::ContextAccessor* accessor)
{
    using namespace DAVA;

    DAVA::TArc::DataContext* ctx = accessor->GetActiveContext();
    DVASSERT(ctx != nullptr);
    SelectionData* data = ctx->GetData<SelectionData>();
    SelectableGroup selection = data->GetMutableSelection();
    for (Selectable& obj : selection.GetMutableContent())
    {
        DAVA::Entity* entity = obj.AsEntity();
        if (entity != nullptr)
        {
            RenderComponent* component = GetRenderComponent(entity);
            if (component != nullptr)
            {
                RenderObject* renderObject = component->GetRenderObject();
                if (renderObject != nullptr)
                {
                    cache.emplace(renderObject, entity);
                }
            }
        }
    }
}

void BillboardCommandProducer::ClearCache()
{
    cache.clear();
}

DAVA::M::CommandProducer::Info BillboardCommandProducer::GetInfo() const
{
    Info info;
    info.icon = SharedIcon(":/QtIcons/sphere.png");
    info.tooltip = QStringLiteral("Make billboard");
    info.description = "Convert to billboard";
    return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Fix lods and switches                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FixLodsAndSwitches : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const DAVA::Reflection& field) const override;
    Info GetInfo() const override;
    bool OnlyForSingleSelection() const;
    std::unique_ptr<DAVA::Command> CreateCommand(const DAVA::Reflection& field, const Params& params) const override;
};

bool FixLodsAndSwitches::IsApplyable(const DAVA::Reflection& field) const
{
    using namespace DAVA;
    RenderObject* renderObject = *field.GetValueObject().GetPtr<RenderObject*>();
    return SceneValidator::IsObjectHasDifferentLODsCount(renderObject);
}

DAVA::M::CommandProducer::Info FixLodsAndSwitches::GetInfo() const
{
    Info info;
    info.icon = SharedIcon(":/QtIcons/clone_batches.png");
    info.tooltip = QStringLiteral("Clone batches for LODs correction");
    info.description = "Clone Last Batch";
    return info;
}

bool FixLodsAndSwitches::OnlyForSingleSelection() const
{
    return true;
}

std::unique_ptr<DAVA::Command> FixLodsAndSwitches::CreateCommand(const DAVA::Reflection& field, const Params& params) const
{
    using namespace DAVA;
    RenderObject* renderObject = *field.GetValueObject().GetPtr<RenderObject*>();
    return std::make_unique<CloneLastBatchCommand>(renderObject);
}
}

DAVA::M::CommandProducerHolder CreateRenderObjectCommandProducer()
{
    using namespace RenderObjectExtensionsDetail;

    DAVA::M::CommandProducerHolder holder;
    holder.AddCommandProducer(std::make_shared<BillboardCommandProducer>());
    holder.AddCommandProducer(std::make_shared<FixLodsAndSwitches>());
    return holder;
}
