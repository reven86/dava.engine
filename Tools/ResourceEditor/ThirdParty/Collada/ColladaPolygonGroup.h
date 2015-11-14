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


#ifndef __COLLADALOADER_COLLADAPOLYGONGROUP_H__
#define __COLLADALOADER_COLLADAPOLYGONGROUP_H__

#include "ColladaIncludes.h"
#include "Base/BaseMath.h"
#include "ColladaMaterial.h"

namespace DAVA
{

class ColladaMesh;

struct ColladaVertex
{	
	ColladaVertex()
	{
		jointCount = 0;
		for (int i = 0; i < 20; ++i)
		{
			joint[i] = 0;
			weight[i] = 0.0f;
		}
	}
        
    static bool IsEqual(const ColladaVertex & v1, const ColladaVertex & c2, int32 vertexFormat);

	Vector3 position;	
	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;
	Vector2 texCoords[4];
	
	int32	jointCount;
	int32	joint[20];
	float32	weight[20];
};

struct ColladaSortVertex
{
	float	sortValue;
	int		vertexIndex;
	int		faceIndex;
	int		pointInFaceIndex;
};

struct ColladaVertexWeight
{
	ColladaVertexWeight()
	{
		jointCount = 0;
	}
	
	void				AddWeight(int jointI, float32 weight);
	void				Normalize();

	int32					jointCount;
	int32					jointArray[10]; 
	float32					weightArray[10];
};


class ColladaPolygonGroup
{
public:
	ColladaPolygonGroup(ColladaMesh * _mesh, FCDGeometryPolygons * _polygons, ColladaVertexWeight * wertexWeightArray);
	~ColladaPolygonGroup();

	void				Render(ColladaMaterial * material);
	
	inline fstring		GetMaterialSemantic() { return materialSemantic; }; 
	inline int			GetTriangleCount() { return triangleCount; };
	inline std::vector<ColladaVertex> & GetVertices() { return unoptimizedVerteces; }
	inline std::vector<int>	& GetIndices() { return indexArray; };
    inline uint32 GetVertexFormat() { return vertexFormat; };
	
	std::vector<ColladaVertex>	unoptimizedVerteces;
	std::vector<ColladaVertex>	skinVerteces;

	ColladaMesh	* parentMesh;
protected:
	bool						skinAnimation;

	FCDGeometryPolygons *		polygons;
	
	int							triangleCount;	
	
	void				RenderMesh();


	std::vector<int>			indexArray;

	int							renderListId;
	AABBox3						bbox;

	fstring						materialSemantic;
    uint32                      vertexFormat;
};

};

#endif // __COLLADALOADER_COLLADAPOLYGONGROUP_H__


