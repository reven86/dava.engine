#include "Commands2/ConvertToBillboardCommand.h"

ConvertToBillboardCommand::ConvertToBillboardCommand(DAVA::RenderObject* ro, DAVA::RenderComponent* rc)
    : RECommand(CMDID_CONVERT_TO_BILLBOARD, "Convert to billboard")
    , oldRenderObject(SafeRetain(ro))
    , renderComponent(rc)
{
}

ConvertToBillboardCommand::~ConvertToBillboardCommand()
{
    SafeRelease(oldRenderObject);
}

void ConvertToBillboardCommand::Undo()
{
}

void ConvertToBillboardCommand::Redo()
{
}
