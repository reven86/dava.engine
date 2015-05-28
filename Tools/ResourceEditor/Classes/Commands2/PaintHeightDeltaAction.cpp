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


#include "Commands2/PaintHeightDeltaAction.h"
#include "Qt/Settings/SettingsManager.h"


PaintHeightDeltaAction::PaintHeightDeltaAction(const DAVA::FilePath& targetImagePath,
                                               DAVA::float32 refDelta,
                                               DAVA::Heightmap* srcHeightmap,
                                               DAVA::uint32 targetImageWidth,
                                               DAVA::uint32 targetImageHeight,
                                               DAVA::float32 targetTerrainHeight,
                                               const DAVA::Vector<DAVA::Color>& pixelColors) : CommandAction(CMDID_PAINT_HEIGHT_DELTA)
                                               
{
    imagePath = targetImagePath;
    heightDelta = refDelta;
    heightmap = SafeRetain(srcHeightmap);
    imageWidth = targetImageWidth;
    imageHeight = targetImageHeight;
    terrainHeight = targetTerrainHeight;
    colors = pixelColors;
}

PaintHeightDeltaAction::~PaintHeightDeltaAction()
{
    SafeRelease(heightmap);
}

void PaintHeightDeltaAction::Redo()
{
    DAVA::Image* croppedHeightmap = CropHeightmapToPow2(heightmap);
    
    imageWidth = DAVA::Max(croppedHeightmap->width, imageWidth);
    imageHeight = DAVA::Max(croppedHeightmap->height, imageHeight);
    
    DAVA::Image* imageDelta = CreateHeightDeltaImage(imageWidth, imageHeight);
    
    PrepareDeltaImage(croppedHeightmap, imageDelta);
    
    SaveDeltaImage(imagePath, imageDelta);
    
    SafeRelease(croppedHeightmap);
    SafeRelease(imageDelta);
}

DAVA::Image* PaintHeightDeltaAction::CropHeightmapToPow2(DAVA::Heightmap* srcHeightmap)
{
    //VI: see VegetationObject for details

    DAVA::Image* originalImage = DAVA::Image::CreateFromData(srcHeightmap->Size(),
                                                             srcHeightmap->Size(),
                                                             DAVA::FORMAT_A16,
                                                             (DAVA::uint8*)srcHeightmap->Data());
    
    DAVA::int32 pow2Size = srcHeightmap->Size();
    if(!DAVA::IsPowerOf2(srcHeightmap->Size()))
    {
        DAVA::EnsurePowerOf2(pow2Size);
        
        if(pow2Size > srcHeightmap->Size())
        {
            pow2Size = pow2Size >> 1;
        }
    }
    
    if(pow2Size != heightmap->Size())
    {
        DAVA::Image* croppedImage = DAVA::Image::CopyImageRegion(originalImage, pow2Size, pow2Size);
        SafeRelease(originalImage);
        
        originalImage = croppedImage;
    }
    
    return originalImage;
}

DAVA::Image* PaintHeightDeltaAction::CreateHeightDeltaImage(DAVA::uint32 width, DAVA::uint32 height)
{
    DAVA::Image* targetImage = DAVA::Image::Create(width, height, DAVA::FORMAT_RGBA8888);
    memset(targetImage->data, 0, targetImage->dataSize);
    return targetImage;
}

void PaintHeightDeltaAction::CalculateHeightmapToDeltaImageMapping(DAVA::Image* heightmapImage,
                                                                   DAVA::Image* deltaImage,
                                                                   /*out*/ DAVA::float32& widthPixelRatio,
                                                                   /*out*/ DAVA::float32& heightPixelRatio)
{
    DAVA::float32 heightmapWidth = (DAVA::float32)heightmapImage->width;
    DAVA::float32 heightmapHeight = (DAVA::float32)heightmapImage->height;
    
    DAVA::float32 deltaImageWidth = (DAVA::float32)deltaImage->width;
    DAVA::float32 deltaImageHeight = (DAVA::float32)deltaImage->height;
    
    widthPixelRatio = deltaImageWidth / heightmapWidth;
    heightPixelRatio = deltaImageHeight / heightmapHeight;
}

DAVA::float32 PaintHeightDeltaAction::SampleHeight(DAVA::uint32 x,
                                                   DAVA::uint32 y,
                                                   DAVA::Image* heightmapImage)
{
    //VI: see VegetationObject and Landscape for details

    DAVA::uint32 pixelSize = sizeof(DAVA::uint16);
    DAVA::uint32 rowStride = pixelSize * heightmapImage->width;
    
    DAVA::uint16 heightmapValue = *(DAVA::uint16*)(heightmapImage->data + y * rowStride + x * pixelSize);
    
    DAVA::float32 height = ((DAVA::float32)heightmapValue / (DAVA::float32)DAVA::Heightmap::MAX_VALUE) * terrainHeight;
    
    return height;
}

void PaintHeightDeltaAction::PrepareDeltaImage(DAVA::Image* heightmapImage,
                                               DAVA::Image* deltaImage)
{
    DAVA::float32 widthPixelRatio = 0.0f;
    DAVA::float32 heightPixelRatio = 0.0f;
    
    CalculateHeightmapToDeltaImageMapping(heightmapImage,
                                          deltaImage,
                                          widthPixelRatio,
                                          heightPixelRatio);
    
    DVASSERT(widthPixelRatio > 0.0f);
    DVASSERT(heightPixelRatio > 0.0f);
    
    DAVA::int32 colorCount = colors.size();
    
    DVASSERT(colorCount >= 2);
    
    for(DAVA::int32 y = 0; y < (DAVA::int32)heightmapImage->height; ++y)
    {
        for(DAVA::int32 x = 0; x < (DAVA::int32)heightmapImage->width; ++x)
        {
            DAVA::float32 heightXY = SampleHeight(x, y, heightmapImage);
            DAVA::float32 heightLeft = (x - 1 >= 0) ? SampleHeight(x - 1, y, heightmapImage) : heightXY;
            DAVA::float32 heightRight = (x + 1 < (DAVA::int32)heightmapImage->width) ? SampleHeight(x + 1, y, heightmapImage) : heightXY;
            DAVA::float32 heightTop = (y - 1 >= 0) ? SampleHeight(x, y - 1, heightmapImage) : heightXY;
            DAVA::float32 heightDown = (y + 1 < (DAVA::int32)heightmapImage->height) ? SampleHeight(x, y + 1, heightmapImage) : heightXY;
            
            bool isOverThreshold = (DAVA::Abs(heightXY - heightLeft) > heightDelta) ||
                                    (DAVA::Abs(heightXY - heightRight) > heightDelta) ||
                                    (DAVA::Abs(heightXY - heightTop) > heightDelta) ||
                                    (DAVA::Abs(heightXY - heightDown) > heightDelta);
            
            if(isOverThreshold)
            {
                DAVA::int32 colorIndex = (x + y) % colorCount;
                
                MarkDeltaRegion((DAVA::uint32)x,
                                (DAVA::uint32)y, widthPixelRatio, heightPixelRatio, colors[colorIndex], deltaImage);
            }
        }
    }
}

void PaintHeightDeltaAction::MarkDeltaRegion(DAVA::uint32 x,
                                             DAVA::uint32 y,
                                             DAVA::float32 widthPixelRatio,
                                             DAVA::float32 heightPixelRatio,
                                             const DAVA::Color& markColor,
                                             DAVA::Image* deltaImage)
{
    DAVA::int32 regionEndX = (DAVA::int32)(x * widthPixelRatio + (DAVA::uint32)widthPixelRatio);
    DAVA::int32 regionEndY = (DAVA::int32)(y * heightPixelRatio + (DAVA::uint32)heightPixelRatio);
    DAVA::int32 rgbaStride = sizeof(DAVA::uint32);
    
    DAVA::uint8 r = (DAVA::uint8)(markColor.r * 255.0f);
    DAVA::uint8 g = (DAVA::uint8)(markColor.g * 255.0f);
    DAVA::uint8 b = (DAVA::uint8)(markColor.b * 255.0f);
    DAVA::uint8 a = (DAVA::uint8)(markColor.a * 255.0f);
    
    for(DAVA::int32 yy = (DAVA::int32)(y * heightPixelRatio); yy < regionEndY; ++yy)
    {
        for(DAVA::int32 xx = (DAVA::int32)(x * widthPixelRatio); xx < regionEndX; ++xx)
        {
            DAVA::uint8* dataPtr = deltaImage->data + yy * deltaImage->width * rgbaStride + xx * rgbaStride;
            
            dataPtr[0] = r;
            dataPtr[1] = g;
            dataPtr[2] = b;
            dataPtr[3] = a;
        }
    }
}

void PaintHeightDeltaAction::SaveDeltaImage(const DAVA::FilePath& targetPath,
                                            DAVA::Image* deltaImage)
{
    DAVA::ImageSystem::Instance()->Save(targetPath, deltaImage);
}
