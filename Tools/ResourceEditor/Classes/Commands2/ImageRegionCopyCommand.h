#ifndef __IMAGE_REGION_COPY_COMMAND_H__
#define __IMAGE_REGION_COPY_COMMAND_H__

#include "Render/Image/Image.h"
#include "Commands2/Base/Command2.h"

class ImageRegionCopyCommand : public Command2
{
public:
    ImageRegionCopyCommand(DAVA::Image* dst, const DAVA::Vector2& dstPos, DAVA::Image* src, const DAVA::Rect& srcRect, DAVA::FilePath savePath = DAVA::FilePath(), DAVA::Image* orig = NULL);
    ~ImageRegionCopyCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::Image* dst;
    DAVA::Image* orig;
    DAVA::Image* copy;
    DAVA::Vector2 pos;
    DAVA::FilePath savePath;
};

#endif // __IMAGE_REGION_COPY_COMMAND_H__
