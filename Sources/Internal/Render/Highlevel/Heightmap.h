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

    Heightmap(int32 size = 0);

    bool BuildFromImage(const Image* image);
    void SaveToImage(const FilePath & filename);
    
    virtual void Save(const FilePath &filePathname);
    virtual bool Load(const FilePath &filePathname);

    inline uint16 GetHeight(uint16 x, uint16 y) const;
    inline uint16 GetHeightClamp(uint16 x, uint16 y) const;

    inline int32 Size() const;
    inline uint16* Data();

    inline int32 GetTileSize() const;
    inline void SetTileSize(int32 newSize);

    Heightmap* Clone(Heightmap* clonedHeightmap);

    inline static const String& FileExtension();

protected:
    bool ReallocateData(int32 newSize);

    DAVA_DEPRECATED(void LoadNotPow2(File* file, int32 readMapSize, int32 readTileSize));

    uint16* data = nullptr;
    int32 size = 0;
    int32 tileSize = 0;

    static String FILE_EXTENSION;

public:
    
    INTROSPECTION_EXTEND(Heightmap, BaseObject,
        MEMBER(size, "Size", I_VIEW)
        MEMBER(tileSize, "Tile Size", I_VIEW)
    );
};

inline uint16 Heightmap::GetHeightClamp(uint16 x, uint16 y) const
{
    uint16 hm_1 = uint16(size - 1);
    return data[Min(x, hm_1) + Min(y, hm_1) * size];
}

inline uint16 Heightmap::GetHeight(uint16 x, uint16 y) const
{
    DVASSERT(x < size && y < size);
    return data[x + y * size];
}

inline uint16* Heightmap::Data()
{
    return data;
}

inline int32 Heightmap::Size() const
{
    return size;
}

inline int32 Heightmap::GetTileSize() const
{
    return tileSize;
}

inline void Heightmap::SetTileSize(int32 newSize)
{
    tileSize = newSize;
}

inline const String& Heightmap::FileExtension()
{
    return FILE_EXTENSION;
}
};

#endif //__DAVAENGINE_HEIGHT_MAP_H__
