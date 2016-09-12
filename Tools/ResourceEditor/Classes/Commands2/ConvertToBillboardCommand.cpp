#include "Commands2/ConvertToBillboardCommand.h"
#include "Render/Highlevel/BillboardRenderObject.h"

ConvertToBillboardCommand::ConvertToBillboardCommand(DAVA::RenderObject* ro, DAVA::Entity* entity_)
    : RECommand(CMDID_CONVERT_TO_BILLBOARD, "Convert to billboard")
    , entity(entity_)
    , oldRenderComponent(GetRenderComponent(entity))
    , newRenderComponent(new RenderComponent())
{
    ScopedPtr<RenderObject> newRenderObject(new BillboardRenderObject());
    oldRenderComponent->GetRenderObject()->Clone(newRenderObject);
    newRenderObject->AddFlag(RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER);
    newRenderObject->RecalcBoundingBox();

    newRenderComponent->SetRenderObject(newRenderObject);

    detachedComponent = newRenderComponent;
}

ConvertToBillboardCommand::~ConvertToBillboardCommand()
{
    DVASSERT(detachedComponent->GetEntity() == nullptr);
    SafeDelete(detachedComponent);
}

void ConvertToBillboardCommand::Redo()
{
    entity->DetachComponent(oldRenderComponent);
    entity->AddComponent(newRenderComponent);

    detachedComponent = oldRenderComponent;
}

void ConvertToBillboardCommand::Undo()
{
    entity->DetachComponent(newRenderComponent);
    entity->AddComponent(oldRenderComponent);

    detachedComponent = newRenderComponent;
}
