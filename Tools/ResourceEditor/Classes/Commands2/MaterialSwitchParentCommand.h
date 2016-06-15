#ifndef __MATERIAL_ASSIGN_COMMAND_H__
#define __MATERIAL_ASSIGN_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "Render/Material/NMaterial.h"
#include "Base/FastName.h"

class SelectableGroup;
class MaterialSwitchParentCommand : public Command2
{
public:
    MaterialSwitchParentCommand(DAVA::NMaterial* instance, DAVA::NMaterial* newParent);
    ~MaterialSwitchParentCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

protected:
    DAVA::NMaterial* oldParent;
    DAVA::NMaterial* newParent;
    DAVA::NMaterial* currentInstance;
};

#endif // __MATERIAL_ASSIGN_COMMAND_H__
