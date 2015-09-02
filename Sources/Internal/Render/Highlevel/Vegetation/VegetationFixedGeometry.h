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


#ifndef __DAVAENGINE_VEGETATIONFIXEDGEOMETRYGENERATOR_H__
#define __DAVAENGINE_VEGETATIONFIXEDGEOMETRYGENERATOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "Render/Highlevel/Vegetation/VegetationGeometry.h"
#include "Render/Highlevel/Vegetation/VegetationMaterialTransformer.h"

namespace DAVA
{

/**
 \brief Geometry generator producing render data for billboarded alphablended vegetation.
    Each vegetation cluster is represented by a billboard plane with alphablend texture.
 */
class VegetationFixedGeometry : public VegetationGeometry
{
public:

    VegetationFixedGeometry(uint32 _maxClusters,
                            uint32 _maxDensityLevels,
                            uint32 _maxLayerTypes,
                            Vector2 _unitSize,
                            const FilePath& textureSheetPath,
                            const Vector<uint32> & _resolutionCellSquare,
                            const Vector<float32> & _resolutionScale,
                            const Vector<Vector2> & _resolutionRanges,
                            const Vector<uint32> & _resolutionTilesPerRow,
                            Vector3 _worldSize);
        
    virtual void Build(Vector<VegetationRenderData*>& renderDataArray, const FastNameSet& materialFlags);
    virtual void OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props);
    
private:

    class FixedMaterialTransformer : public VegetationMaterialTransformer
    {
        public:
        
            virtual void TransformMaterialOnCreate(NMaterial* mat);
    };

private:

    void GenerateVertices(uint32 maxClusters,
                          size_t maxTotalClusters,
                          uint32 maxClusterRowSize,
                          uint32 tilesPerRow,
                          Vector2 unitSize,
                          Vector<uint32>& layerOffsets,
                          VegetationRenderData& renderData);
    
    void GenerateIndices(uint32 maxClusters,
                         uint32 maxClusterRowSize,
                         Vector<uint32>& layerOffsets,
                         VegetationRenderData& renderData);
    
    void PrepareIndexBufferData(uint32 indexBufferIndex,
                                uint32 maxClusters,
                                uint32 maxClusterRowSize,
                                size_t resolutionIndex,
                                uint32 resolutionOffset,
                                Vector<uint32>& layerOffsets,
                                Vector<VegetationIndex>& preparedIndices,
                                AABBox3& indexBufferBBox,
                                VegetationRenderData& renderData);
    
    void PrepareSortedIndexBufferVariations(size_t& currentIndexIndex,
                                            uint32 indexBufferIndex,
                                            size_t polygonElementCount,
                                            AABBox3& indexBufferBBox,
                                            Vector<Vector3>& directionPoints,
                                            Vector<Vector<VegetationSortedBufferItem> >& currentResolutionIndexArray,
                                            Vector<PolygonSortData>& sortingArray,
                                            Vector<VegetationIndex>& preparedIndices,
                                            VegetationRenderData& renderData);
    
    void GenerateRenderDataObjects(VegetationRenderData& renderData);
    
    static bool PolygonByDistanceCompareFunction(const PolygonSortData& a, const PolygonSortData&  b);
    static int32 RandomShuffleFunc(int32 limit);
    
private:
    
    uint32 maxClusters;
    uint32 maxDensityLevels;
    uint32 maxLayerTypes;
    TextureSheet textureSheet;
    const Vector<uint32> & resolutionCellSquare;
    const Vector<float32> & resolutionScale;
    const Vector<Vector2> & resolutionRanges;
    const Vector<uint32> & resolutionTilesPerRow;
    Vector2 unitSize;
    Vector3 worldSize;
};

};


#endif /* defined(__DAVAENGINE_VEGETATIONFIXEDGEOMETRYGENERATOR_H__) */
