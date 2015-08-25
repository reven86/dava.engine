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


#ifndef __DAVAENGINE_VEGETATIONGEOMETRYGENERATOR_H__
#define __DAVAENGINE_VEGETATIONGEOMETRYGENERATOR_H__

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

#include "Render/Highlevel/Vegetation/VegetationRenderData.h"
#include "Render/Highlevel/Vegetation/VegetationMaterialTransformer.h"

namespace DAVA
{

/**
 \brief Base class for vegetation geometry generation. Vegetation render object
    creates a specific vegetation geometry implementation object and uses vegetation
    render data produced by the generator to render vegetation.
 */
class VegetationGeometry
{
protected:

    struct PolygonSortData
    {
        Array<VegetationIndex, 3> indices;
        float32 cameraDistance;
        
        inline PolygonSortData();
    };
    
    VegetationMaterialTransformer* materialTransform;
    
public:

    VegetationGeometry();
    virtual ~VegetationGeometry();
        
    virtual void Build(Vector<VegetationRenderData*>& renderDataArray, const FastNameSet& materialFlags) = 0;
    virtual void OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props) = 0;
    
    virtual void SetupCameraPositions(const AABBox3& bbox, Vector<Vector3>& positions);
    
    virtual uint32 GetSortDirectionCount();
    
    virtual void ReleaseRenderData(Vector<VegetationRenderData*>& renderDataArray);
    
    inline VegetationMaterialTransformer* GetMaterialTransform();
};

inline VegetationGeometry::PolygonSortData::PolygonSortData()
{
    indices[0] = indices[1] = indices[2] = -1;
    cameraDistance = -1.0f;
}

inline VegetationMaterialTransformer* VegetationGeometry::GetMaterialTransform()
{
    return materialTransform;
}
    
};


#endif /* defined(__DAVAENGINE_VEGETATIONGEOMETRYGENERATOR_H__) */
