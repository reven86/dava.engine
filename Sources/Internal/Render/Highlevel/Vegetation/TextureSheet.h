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


#ifndef __DAVAENGINE_TEXTURESHEET_H__
#define __DAVAENGINE_TEXTURESHEET_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

#define MAX_CELL_TEXTURE_COORDS 4

namespace DAVA
{

/**
 \brief Describes a single cell inside of the entire texture sheet.
 */
struct TextureSheetCell
{
    Vector2 coords[MAX_CELL_TEXTURE_COORDS];
    uint32 geometryId;
    Vector2 geometryScale;
        
    inline TextureSheetCell();
        
    inline TextureSheetCell& operator=(const TextureSheetCell& src);
};

/**
 \brief Texture sheet is a simple texture atlas. Each cell represents single texture.
    Texture sheet is used for rendering billboarded vegetation.
 */
class TextureSheet
{
public:
        
    Vector<TextureSheetCell> cells;
        
    inline TextureSheet& operator=(const TextureSheet& src);
        
    void Load(const FilePath &yamlPath);
};

inline TextureSheetCell::TextureSheetCell() :
    geometryId(0),
    geometryScale(1.0f, 1.0f)
{
}
    
inline TextureSheetCell& TextureSheetCell::operator=(const TextureSheetCell& src)
{
    coords[0] = src.coords[0];
    coords[1] = src.coords[1];
    coords[2] = src.coords[2];
    coords[3] = src.coords[3];
        
    geometryId = src.geometryId;
    geometryScale = src.geometryScale;
        
    return *this;
}

};

#endif /* defined(__DAVAENGINE_TEXTURESHEETCELL_H__) */
