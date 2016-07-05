#pragma once

#include "Commands2/Base/CommandAction.h"

namespace DAVA
{
class NMaterial;
}

class ShowMaterialAction : public CommandAction
{
public:
    ShowMaterialAction(DAVA::NMaterial* material);
    void Redo() override;

private:
    DAVA::NMaterial* material;
};
