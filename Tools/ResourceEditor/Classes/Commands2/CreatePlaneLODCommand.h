#ifndef __CREATE_PLANE_LOD_COOMAND_H__
#define __CREATE_PLANE_LOD_COOMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "CreatePlaneLODCommandHelper.h"
#include "DAVAEngine.h"

class CreatePlaneLODCommand : public CommandWithoutExecute
{
public:
    CreatePlaneLODCommand(const CreatePlaneLODCommandHelper::RequestPointer& request);

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const;

    DAVA::RenderBatch* GetRenderBatch() const;

protected:
    void CreateTextureFiles();
    void DeleteTextureFiles();

    static bool IsHorisontalMesh(const DAVA::AABBox3& bbox);

private:
    CreatePlaneLODCommandHelper::RequestPointer request;
};


#endif // #ifndef __CREATE_PLANE_LOD_COOMAND_H__