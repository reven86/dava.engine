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


#include "EdgeAdjacency.h"
#include "PolygonGroup.h"
#include "Render/RenderBase.h"
#include "Math/Math2D.h"

namespace DAVA
{

bool EdgeAdjacency::Edge::IsEqual(const Edge & otherEdge)
{
	if(EdgeAdjacency::IsPointsEqual(points[0], otherEdge.points[0]) && EdgeAdjacency::IsPointsEqual(points[1], otherEdge.points[1]))
	{
		return true;
	}
	else if(EdgeAdjacency::IsPointsEqual(points[0], otherEdge.points[1]) && EdgeAdjacency::IsPointsEqual(points[1], otherEdge.points[0]))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool EdgeAdjacency::IsPointsEqual(const Vector3 & p0, const Vector3 & p1)
{
	if(FLOAT_EQUAL(p0.x, p1.x) && FLOAT_EQUAL(p0.y, p1.y) && FLOAT_EQUAL(p0.z, p1.z))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void EdgeAdjacency::InitFromPolygonGroup(PolygonGroup * _polygonGroup, int32 indexCount)
{
	polygonGroup = _polygonGroup;

#ifdef __DAVAENGINE_DEBUG__	
    DVASSERT(rhi::PRIMITIVE_TRIANGLELIST == polygonGroup->GetPrimitiveType());
#endif

	for(int32 i = 0; i < indexCount; i += 3)
	{
		CreateTriangle(i);
	}
}

void EdgeAdjacency::CreateTriangle(int32 startingI)
{
	int32 i = startingI;

	int32 i0, i1, i2;
	polygonGroup->GetIndex(i, i0);
	polygonGroup->GetIndex(i+1, i1);
	polygonGroup->GetIndex(i+2, i2);

	Vector3 p0, p1, p2;
	polygonGroup->GetCoord(i0, p0);
	polygonGroup->GetCoord(i1, p1);
	polygonGroup->GetCoord(i2, p2);

	Edge edge0;
	FillEdge(edge0, i0, i1);
	int32 edgeIndex0 = GetEdgeIndex(edge0);

	Edge edge1;
	FillEdge(edge1, i1, i2);
	int32 edgeIndex1 = GetEdgeIndex(edge1);

	Edge edge2;
	FillEdge(edge2, i2, i0);
	int32 edgeIndex2 = GetEdgeIndex(edge2);

	TriangleData triangleData;
	triangleData.i0 = i0;
	triangleData.i1 = i1;
	triangleData.i2 = i2;

	if(edges[edgeIndex0].sharedTriangles.size() < 2)
	{
		edges[edgeIndex0].sharedTriangles.push_back(triangleData);
	}
	if(edges[edgeIndex1].sharedTriangles.size() < 2)
	{
		edges[edgeIndex1].sharedTriangles.push_back(triangleData);
	}
	if(edges[edgeIndex2].sharedTriangles.size() < 2)
	{
		edges[edgeIndex2].sharedTriangles.push_back(triangleData);
	}
}

void EdgeAdjacency::FillEdge(Edge & edge, int32 index0, int32 index1)
{
	Vector3 position;

	polygonGroup->GetCoord(index0, position);
	edge.points[0] = position;

	polygonGroup->GetCoord(index1, position);
	edge.points[1] = position;
}

int32 EdgeAdjacency::GetEdgeIndex(Edge & edge)
{
	int32 edgesCount = static_cast<int32>(edges.size());
	for(int32 i = 0; i < edgesCount; ++i)
	{
		if(edges[i].IsEqual(edge))
		{
			return i;
		}
	}

	edges.push_back(edge);

	return edgesCount;
}

Vector<EdgeAdjacency::Edge> & EdgeAdjacency::GetEdges()
{
	return edges;
}

int32 EdgeAdjacency::GetEdgesWithTwoTrianglesCount()
{
	int32 ret = 0;

	int32 size = static_cast<int32>(edges.size());
	for(int32 i = 0; i < size; ++i)
	{
		if(edges[i].sharedTriangles.size() == 2)
		{
			ret++;
		}
	}

	return ret;
}

};
