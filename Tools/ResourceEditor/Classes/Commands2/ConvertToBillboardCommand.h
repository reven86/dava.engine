#pragma once

#include "Commands2/Base/RECommand.h"

namespace DAVA
{
class RenderObject;
class RenderComponent;
}

class ConvertToBillboardCommand : public RECommand
{
public:
    ConvertToBillboardCommand(DAVA::RenderObject*, DAVA::RenderComponent*);
    ~ConvertToBillboardCommand();

    void Undo() override;
    void Redo() override;

private:
    DAVA::RenderObject* oldRenderObject = nullptr;
    DAVA::RenderComponent* renderComponent = nullptr;
};
