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



#ifndef __PAINT_TOOL_H__
#define __PAINT_TOOL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool: public BaseObject
{
    
public:

    enum eToolType
    {
        TOOL_BRUSH = 0,
        TOOL_DROPPER,
        TOOL_COPYPASTE
    };

protected:
    virtual ~LandscapeTool();
public:
    LandscapeTool(int32 _ID, eToolType _type, const FilePath & _imageName);

    static float32 SizeColorMin();
    static float32 SizeColorMax();

    static float32 StrengthColorMin();
    static float32 StrengthColorMax();
    
    static float32 StrengthHeightMax();
    static float32 SizeHeightMax();

    static float32 DefaultStrengthHeight();
    static float32 DefaultSizeHeight();

    int32 toolID;
    
    FilePath imageName;
    Image *image;
    Sprite *sprite;
    
    //height
    float32 strength;
    float32 size;
    float32 maxStrength;
    float32 maxSize;
    float32 height;
    float32 averageStrength;
    
    bool relativeDrawing;
    bool averageDrawing;
    bool absoluteDropperDrawing;
    
    bool copyHeightmap;
    bool copyTilemask;
    
    eToolType type;
};

#endif // __PAINT_TOOL_H__
