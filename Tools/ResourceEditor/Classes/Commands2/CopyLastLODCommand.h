#ifndef __COPY_LAST_LOD_COMMAND_H__
#define __COPY_LAST_LOD_COMMAND_H__

#include "Commands2/Base/Command2.h"
#include "DAVAEngine.h"

class CopyLastLODToLod0Command : public Command2
{
public:
    //TODO: remove after lod editing implementation
    DAVA_DEPRECATED(CopyLastLODToLod0Command(DAVA::LodComponent* lod));
    ~CopyLastLODToLod0Command();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

    DAVA::LodComponent* lodComponent;
    DAVA::Vector<DAVA::RenderBatch*> newBatches;
    DAVA::Vector<DAVA::int32> switchIndices;
};

#endif // __COPY_LAST_LOD_COMMAND_H__
