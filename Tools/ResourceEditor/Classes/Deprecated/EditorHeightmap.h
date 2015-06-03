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


#ifndef __EDITOR_HEIGHTMAP_H__
#define __EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"

class EditorHeightmap: public DAVA::Heightmap
{
    static const DAVA::int32 MAX_EDITOR_HEIGHTMAP_SIZE = 513;
    static const DAVA::int32 VALUE_NOT_CHANGED = 0;
    static const DAVA::int32 VALUE_WAS_CHANGED = 1;
    
public:

    EditorHeightmap(DAVA::Heightmap *heightmap);
	virtual ~EditorHeightmap();
    
    void HeghtWasChanged(const DAVA::Rect &changedRect);
    
    virtual void Save(const DAVA::String &filePathname);
    virtual bool Load(const DAVA::String &filePathname);
    
    
    void DrawRelativeRGBA(DAVA::Image *src, DAVA::int32 x, DAVA::int32 y, DAVA::int32 width, DAVA::int32 height, DAVA::float32 k);
    void DrawAverageRGBA(DAVA::Image *mask, DAVA::int32 x, DAVA::int32 y, DAVA::int32 width, DAVA::int32 height, DAVA::float32 k);
    void DrawAbsoluteRGBA(DAVA::Image *mask, DAVA::int32 x, DAVA::int32 y, DAVA::int32 width, DAVA::int32 height, DAVA::float32 time, DAVA::float32 dstHeight);
    void DrawCopypasteRGBA(DAVA::Image *mask, const DAVA::Vector2 &posFrom, const DAVA::Vector2 &posTo, DAVA::int32 width, DAVA::int32 height, DAVA::float32 koef);
    
    static void DrawCopypasteRGBA(DAVA::Image *src, DAVA::Image *dst, DAVA::Image *mask, const DAVA::Vector2 &posFrom, const DAVA::Vector2 &posTo, DAVA::int32 width, DAVA::int32 height);
    
    
    static bool Clipping(DAVA::int32 & srcOffset, DAVA::int32 & dstOffset, DAVA::int32 & dstX, DAVA::int32 & dstY,
                         DAVA::int32 dstWidth, DAVA::int32 dstHeight, DAVA::int32 & width, DAVA::int32 & height,
                         DAVA::int32 & yAddSrc, DAVA::int32 & xAddDst, DAVA::int32 & yAddDst);

    
    

protected:
    
    void DownscaleOrClone();
    void Downscale(DAVA::int32 newSize);
    void Upscale();
    void InitializeScalingTable(DAVA::int32 count);
    
    void InitializeTableOfChanges();
    
    DAVA::uint16 GetHeightValue(DAVA::int32 posX, DAVA::int32 posY, DAVA::int32 muliplier);
    DAVA::uint16 GetVerticalValue(DAVA::int32 posY, DAVA::int32 muliplier);
    DAVA::uint16 GetHorizontalValue(DAVA::int32 posX, DAVA::int32 muliplier);
    
    void UpscaleValue(DAVA::int32 leftX, DAVA::int32 topY, DAVA::int32 muliplier);
    
protected:

    Heightmap *savedHeightmap;
    
    DAVA::uint8 *tableOfChanges;
    DAVA::float32 *scalingTable;
};


#endif //__EDITOR_HEIGHTMAP_H__
