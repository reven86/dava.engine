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


#ifndef __CUSTOMGEOMETRYSERIALIZATIONDATA_H__
#define __CUSTOMGEOMETRYSERIALIZATIONDATA_H__

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"

#include "Render/Material/NMaterial.h"

#include "Render/Highlevel/Vegetation/VegetationRenderData.h"

namespace DAVA
{
class Entity;

/**
 \brief This data structure allows to virtualize geometry storage. Geometry data
    is loaded both from the saved to scene KeyedArchive and an external .sc2 file.
    The data is passed to the geometry generator implementation then.
 */
class VegetationGeometryData
{
public:
    VegetationGeometryData(Vector<NMaterial*>& materialsData,
                           Vector<Vector<Vector<Vector3>>>& positionLods,
                           Vector<Vector<Vector<Vector2>>>& texCoordLods,
                           Vector<Vector<Vector<Vector3>>>& normalLods,
                           Vector<Vector<Vector<VegetationIndex>>>& indexLods);
    VegetationGeometryData(VegetationGeometryData& src);
    ~VegetationGeometryData();

    uint32 GetLayerCount() const;
    uint32 GetLodCount(uint32 layerIndex) const;

    NMaterial* GetMaterial(uint32 layerIndex);

    Vector<Vector3>& GetPositions(uint32 layerIndex, uint32 lodIndex);
    Vector<Vector2>& GetTextureCoords(uint32 layerIndex, uint32 lodIndex);
    Vector<Vector3>& GetNormals(uint32 layerIndex, uint32 lodIndex);
    Vector<VegetationIndex>& GetIndices(uint32 layerIndex, uint32 lodIndex);

private:
    void Load(Vector<NMaterial*>& materialsData,
              Vector<Vector<Vector<Vector3>>>& positionLods,
              Vector<Vector<Vector<Vector2>>>& texCoordLods,
              Vector<Vector<Vector<Vector3>>>& normalLods,
              Vector<Vector<Vector<VegetationIndex>>>& indexLods);

private:
    Vector<NMaterial*> materials;
    Vector<Vector<Vector<Vector3>>> positions;
    Vector<Vector<Vector<Vector2>>> texCoords;
    Vector<Vector<Vector<Vector3>>> normals;
    Vector<Vector<Vector<VegetationIndex>>> indices;
};

using VegetationGeometryDataPtr = std::unique_ptr<VegetationGeometryData>;

class VegetationGeometryDataReader
{
public:
    static VegetationGeometryDataPtr ReadScene(const FilePath& scenePath);
};
};

#endif /* defined(__CUSTOMGEOMETRYSERIALIZATIONDATA_H__) */
