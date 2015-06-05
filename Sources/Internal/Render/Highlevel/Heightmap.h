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


#ifndef __DAVAENGINE_HEIGHT_MAP_H__
#define __DAVAENGINE_HEIGHT_MAP_H__

#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
    
class Image;
class Heightmap: public BaseObject
{
protected:
	virtual ~Heightmap();
public:

    static const int32 MAX_VALUE = 65535;
    static const int32 IMAGE_CORRECTION = MAX_VALUE / 255;
    
    Heightmap();
    
    bool BuildFromImage( const Image *image);
    void SaveToImage(const FilePath & filename);
    
    virtual void Save(const FilePath &filePathname);
    virtual bool Load(const FilePath &filePathname);

    void ReleaseData();
    
    uint16 * Data();
    int32 Size() const;
    
    int32 GetTileSize() const;
    void SetTileSize(int32 newSize);
    
    static const String FileExtension();

    Heightmap *Clone(Heightmap *clonedHeightmap);

protected:
    
    Heightmap *CreateHeightmapForSize(int32 newSize);
    
    bool AllocateData(int32 newSize);
    
protected:
    
	uint16 *data;
    int32 size;
    int32 tileSize;
    
public:
    
    INTROSPECTION_EXTEND(Heightmap, BaseObject,
        MEMBER(size, "Size", I_VIEW)
        MEMBER(tileSize, "Tile Size", I_VIEW)
    );
};

};

#endif //__DAVAENGINE_HEIGHT_MAP_H__
