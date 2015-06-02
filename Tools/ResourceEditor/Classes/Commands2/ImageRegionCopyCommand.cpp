/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Commands2/ImageRegionCopyCommand.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"

ImageRegionCopyCommand::ImageRegionCopyCommand(DAVA::Image* _dst, const DAVA::Vector2& dstPos, DAVA::Image* src, const DAVA::Rect &srcRect, DAVA::FilePath _savePath, DAVA::Image* _orig)
	: Command2(CMDID_IMAGE_REGION_COPY, "Remove entity")
	, dst(_dst)
	, orig(NULL)
	, copy(NULL)
	, pos(dstPos)
	, savePath(_savePath)
{
	SafeRetain(dst);

    if(NULL != src && NULL != dst)
    {
        if(NULL != _orig)
        {
            DVASSERT(_orig->width == srcRect.dx);
            DVASSERT(_orig->height == srcRect.dy);

            orig = _orig;
        }
        else
        {
            orig = DAVA::Image::CopyImageRegion((const DAVA::Image *) dst, DAVA::Rect(dstPos.x, dstPos.y, srcRect.dx, srcRect.dy));
        }

        copy = DAVA::Image::CopyImageRegion((const DAVA::Image *) src, srcRect);
    }
}

ImageRegionCopyCommand::~ImageRegionCopyCommand()
{
	SafeRelease(dst);
    SafeRelease(copy);
    SafeRelease(orig);
}

void ImageRegionCopyCommand::Undo()
{
    if(NULL != dst && NULL != orig)
    {
        dst->InsertImage(orig, pos, DAVA::Rect(0, 0, (DAVA::float32)orig->width, (DAVA::float32)orig->height));
        if(!savePath.IsEmpty())
        {
            DAVA::ImageSystem::Instance()->Save(savePath, dst);
        }
    }
}

void ImageRegionCopyCommand::Redo()
{
    if(NULL != dst && NULL != copy)
    {
        dst->InsertImage(copy, pos, DAVA::Rect(0, 0, (DAVA::float32)copy->width, (DAVA::float32)copy->height));
        if(!savePath.IsEmpty())
        {
            DAVA::ImageSystem::Instance()->Save(savePath, dst);
        }
    }
}
