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

#ifndef __DAVAENGINE_VEGETATIONSPATIALDATA_H__
#define __DAVAENGINE_VEGETATIONSPATIALDATA_H__

namespace DAVA
{

/**
 \brief Data chunk associated with a vegetation quad tree. 
    This data is used for visibility culling and rendering of tree leafes with proper geometry buffers.
 */
struct VegetationSpatialData
{
    int16 x;
    int16 y;
    int16 width;
    int16 height;
    int8 rdoIndex;
    bool isVisible;
    
    Vector2 animationOffset[4];
    Vector2 animationVelocity[4];
    
    AABBox3 bbox;
    float32 cameraDistance;
    uint8 clippingPlane;
    
    inline VegetationSpatialData();
    inline VegetationSpatialData& operator=(const VegetationSpatialData& src);
    inline static bool IsEmpty(uint32 cellValue);
    inline bool IsRenderable() const;
    inline int16 GetResolutionId() const;
    inline bool IsVisibleInResolution(uint32 resolutionId, uint32 maxResolutions) const;
    inline bool IsElementaryCell() const;
};

inline VegetationSpatialData::VegetationSpatialData() : x(-1),
                                    y(-1),
                                    rdoIndex(-1),
                                    isVisible(true),
                                    cameraDistance(0.0f),
                                    clippingPlane(0)
{
}

inline VegetationSpatialData& VegetationSpatialData::operator=(const VegetationSpatialData& src)
{
    x = src.x;
    y = src.y;
    bbox = src.bbox;
    cameraDistance = src.cameraDistance;
    clippingPlane = src.clippingPlane;
    width = src.width;
    height = src.height;
    rdoIndex = src.rdoIndex;
    
    return *this;
}

inline bool VegetationSpatialData::IsEmpty(uint32 cellValue)
{
    return (0 == (cellValue & 0x0F0F0F0F));
}

inline bool VegetationSpatialData::IsVisibleInResolution(uint32 resolutionId, uint32 maxResolutions) const
{
    uint32 refResolution = ((x * y) % maxResolutions);
    return (refResolution >= resolutionId);
}

inline bool VegetationSpatialData::IsRenderable() const
{
    return (width > 0 && height > 0);
}

inline int16 VegetationSpatialData::GetResolutionId() const
{
    return (width * height);
}

bool VegetationSpatialData::IsElementaryCell() const
{
    return (1 == width);
}

};

#endif
