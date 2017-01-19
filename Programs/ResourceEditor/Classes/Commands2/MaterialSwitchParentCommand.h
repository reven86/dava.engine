#ifndef __MATERIAL_ASSIGN_COMMAND_H__
#define __MATERIAL_ASSIGN_COMMAND_H__

#include "Commands2/Base/RECommand.h"
#include "Render/Material/NMaterial.h"
#include "Base/FastName.h"

class SelectableGroup;
class MaterialSwitchParentCommand : public RECommand
{
public:
    MaterialSwitchParentCommand(DAVA::NMaterial* instance, DAVA::NMaterial* newParent);
    ~MaterialSwitchParentCommand();

    void Undo() override;
    void Redo() override;

protected:
    DAVA::NMaterial* oldParent;
    DAVA::NMaterial* newParent;
    DAVA::NMaterial* currentInstance;
};

#endif // __MATERIAL_ASSIGN_COMMAND_H__
