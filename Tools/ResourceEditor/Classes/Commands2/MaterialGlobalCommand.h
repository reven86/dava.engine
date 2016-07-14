#ifndef __MATERIAL_GLOBAL_COMMAND_H__
#define __MATERIAL_GLOBAL_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "Render/Material/NMaterial.h"

class MaterialGlobalSetCommand : public CommandWithoutExecute
{
public:
    MaterialGlobalSetCommand(DAVA::Scene* _scene, DAVA::NMaterial* global);
    ~MaterialGlobalSetCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;

protected:
    DAVA::Scene* scene;
    DAVA::NMaterial* oldGlobal;
    DAVA::NMaterial* newGlobal;
};

#endif // __MATERIAL_GLOBAL_COMMAND_H__
