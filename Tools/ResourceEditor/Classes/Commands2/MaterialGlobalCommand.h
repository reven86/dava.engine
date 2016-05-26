#ifndef __MATERIAL_GLOBAL_COMMAND_H__
#define __MATERIAL_GLOBAL_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "Render/Material/NMaterial.h"

class MaterialGlobalSetCommand : public Command2
{
public:
    MaterialGlobalSetCommand(DAVA::Scene* _scene, DAVA::NMaterial* global);
    ~MaterialGlobalSetCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

protected:
    DAVA::Scene* scene;
    DAVA::NMaterial* oldGlobal;
    DAVA::NMaterial* newGlobal;
};

#endif // __MATERIAL_GLOBAL_COMMAND_H__
