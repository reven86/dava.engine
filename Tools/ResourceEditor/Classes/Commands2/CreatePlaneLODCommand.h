#ifndef __CREATE_PLANE_LOD_COOMAND_H__
#define __CREATE_PLANE_LOD_COOMAND_H__

#include "Commands2/Base/Command2.h"
#include "CreatePlaneLODCommandHelper.h"
#include "DAVAEngine.h"

class CreatePlaneLODCommand : public Command2
{
public:
    CreatePlaneLODCommand(const CreatePlaneLODCommandHelper::RequestPointer& request);

    virtual void Undo() override;
    virtual void Redo() override;
    virtual DAVA::Entity* GetEntity() const override;

    DAVA::RenderBatch* GetRenderBatch() const;

protected:
    void CreateTextureFiles();
    void DeleteTextureFiles();

    static bool IsHorisontalMesh(const DAVA::AABBox3& bbox);

private:
    CreatePlaneLODCommandHelper::RequestPointer request;
};


#endif // #ifndef __CREATE_PLANE_LOD_COOMAND_H__