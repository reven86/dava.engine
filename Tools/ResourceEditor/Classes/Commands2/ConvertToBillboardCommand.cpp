#include "Commands2/ConvertToBillboardCommand.h"
#include "Render/Highlevel/BillboardRenderObject.h"

ConvertToBillboardCommand::ConvertToBillboardCommand(DAVA::RenderObject* ro, DAVA::Entity* entity_)
    : RECommand(CMDID_CONVERT_TO_BILLBOARD, "Convert to billboard")
    , entity(entity_)
    , oldRenderObject(SafeRetain(ro))
{
}

ConvertToBillboardCommand::~ConvertToBillboardCommand()
{
    SafeRelease(oldRenderObject);
    SafeRelease(billboardRenderObject);
}

void ConvertToBillboardCommand::Redo()
{
    oldRenderComponent = new RenderComponent(oldRenderObject);
    entity->RemoveComponent(GetRenderComponent(entity));

    billboardRenderObject = oldRenderObject->CloneToDerivedClass<BillboardRenderObject>();
    billboardRenderObject->AddFlag(RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER);
    entity->AddComponent(new RenderComponent(billboardRenderObject));

    billboardRenderObject->RecalcBoundingBox();
}

void ConvertToBillboardCommand::Undo()
{
    entity->RemoveComponent(GetRenderComponent(entity));

    entity->AddComponent(oldRenderComponent);
    SafeRelease(billboardRenderObject);
}
