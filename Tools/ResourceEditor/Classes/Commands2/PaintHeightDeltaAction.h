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


#ifndef __PAINT_HEIGHT_DELTA_ACTION_H__
#define __PAINT_HEIGHT_DELTA_ACTION_H__

#include "Commands2/CommandAction.h"
#include "DAVAEngine.h"

class PaintHeightDeltaAction : public CommandAction
{
public:

	PaintHeightDeltaAction(const DAVA::FilePath& targetImagePath,
                           DAVA::float32 refDelta,
                           DAVA::Heightmap* srcHeightmap,
                           DAVA::uint32 targetImageWidth,
                           DAVA::uint32 targetImageHeight,
                           DAVA::float32 targetTerrainHeight,
                           const DAVA::Vector<DAVA::Color>& pixelColors);
    
    ~PaintHeightDeltaAction();
    
    virtual void Redo();
    
protected:
    
    DAVA::Image* CropHeightmapToPow2(DAVA::Heightmap* srcHeightmap);
    DAVA::Image* CreateHeightDeltaImage(DAVA::uint32 width, DAVA::uint32 height);
    void CalculateHeightmapToDeltaImageMapping(DAVA::Image* heightmapImage,
                                               DAVA::Image* deltaImage,
                                               /*out*/ DAVA::float32& widthPixelRatio,
                                               /*out*/ DAVA::float32& heightPixelRatio);
    DAVA::float32 SampleHeight(DAVA::uint32 x, DAVA::uint32 y, DAVA::Image* heightmapImage);
    
    void PrepareDeltaImage(DAVA::Image* heightmapImage,
                            DAVA::Image* deltaImage);
    
    void MarkDeltaRegion(DAVA::uint32 x,
                         DAVA::uint32 y,
                         DAVA::float32 widthPixelRatio,
                         DAVA::float32 heightPixelRatio,
                         const DAVA::Color& markColor,
                         DAVA::Image* deltaImage);
    
    void SaveDeltaImage(const DAVA::FilePath& targetPath, DAVA::Image* deltaImage);
    
private:
    
    DAVA::FilePath imagePath;
    DAVA::float32 heightDelta;
    DAVA::Heightmap* heightmap;
    DAVA::uint32 imageWidth;
    DAVA::uint32 imageHeight;
    DAVA::float32 terrainHeight;
    DAVA::Vector<DAVA::Color> colors;
};


#endif
