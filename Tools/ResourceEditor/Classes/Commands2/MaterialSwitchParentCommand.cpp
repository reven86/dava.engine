#include "MaterialSwitchParentCommand.h"

MaterialSwitchParentCommand::MaterialSwitchParentCommand(DAVA::NMaterial* instance, DAVA::NMaterial* _newParent)
    : Command2(CMDID_MATERIAL_SWITCH_PARENT, "Switch Material Parent")
{
    DVASSERT(instance);
    DVASSERT(_newParent);
    DVASSERT(instance->GetParent());

    currentInstance = DAVA::SafeRetain(instance);
    newParent = DAVA::SafeRetain(_newParent);
    oldParent = DAVA::SafeRetain(instance->GetParent());
}

MaterialSwitchParentCommand::~MaterialSwitchParentCommand()
{
    DAVA::SafeRelease(oldParent);
    DAVA::SafeRelease(newParent);
    DAVA::SafeRelease(currentInstance);
}

void MaterialSwitchParentCommand::Redo()
{
    currentInstance->SetParent(newParent);
}

void MaterialSwitchParentCommand::Undo()
{
    currentInstance->SetParent(oldParent);
}

DAVA::Entity* MaterialSwitchParentCommand::GetEntity() const
{
    return NULL;
}
