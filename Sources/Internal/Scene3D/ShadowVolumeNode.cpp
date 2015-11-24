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


#include "ShadowVolumeNode.h"
#include "Render/3D/StaticMesh.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "FileSystem/FilePath.h"


namespace DAVA
{

	
	static const FastName LIGHT_POSITION0("lightPosition0");

ShadowVolumeNode::ShadowVolumeNode()
: shadowPolygonGroup(0)
{
}

DAVA::ShadowVolumeNode::~ShadowVolumeNode()
{
}

void DAVA::ShadowVolumeNode::Draw()
{
    //scene->AddDrawTimeShadowVolume(this);
}

void DAVA::ShadowVolumeNode::DrawShadow()
{
#if 0
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
	Matrix4 meshFinalMatrix = GetWorldTransform() * prevMatrix;
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);

	Matrix4 projMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);

	RenderManager::Instance()->SetShader(shader);
	RenderManager::Instance()->SetRenderData(shadowPolygonGroup->renderDataObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();

	Vector3 position = Vector3() * GetWorldTransform();
	Light * light = scene->GetNearestDynamicLight(Light::TYPE_COUNT, position);
	if (light && uniformLightPosition0 != -1)
	{
		Vector3 lightPosition0 = light->GetPosition();
		const Matrix4 & matrix = scene->GetCurrentCamera()->GetMatrix();
		lightPosition0 = lightPosition0 * matrix;

		shader->SetUniformValueByIndex(uniformLightPosition0, lightPosition0);
	}

	if (shadowPolygonGroup->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, shadowPolygonGroup->indexCount, EIF_16, 0);
	}
	else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, shadowPolygonGroup->indexCount, EIF_16, shadowPolygonGroup->indexArray);
	}

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
#endif
}

int32 ShadowVolumeNode::FindEdgeInMappingTable(int32 nV1, int32 nV2, EdgeMapping* mapping, int32 count)
{
	for( int i = 0; i < count; ++i )
	{
		// If both vertex indexes of the old edge in mapping entry are -1, then
		// we have searched every valid entry without finding a match.  Return
		// this index as a newly created entry.
		if( ( mapping[i].oldEdge[0] == -1 && mapping[i].oldEdge[1] == -1 ) ||

			// Or if we find a match, return the index.
			( mapping[i].oldEdge[1] == nV1 && mapping[i].oldEdge[0] == nV2 ) )
		{
			return i;
		}
	}

	DVASSERT(0);
	return -1;  // We should never reach this line
}


void ShadowVolumeNode::CopyGeometryFrom(MeshInstanceNode * meshInstance)
{
	PolygonGroup * oldPolygonGroup = meshInstance->GetPolygonGroups()[0]->GetPolygonGroup();

	int32 numEdges = oldPolygonGroup->GetIndexCount();
	int32 oldIndexCount = oldPolygonGroup->GetIndexCount();
	EdgeMapping * mapping = new EdgeMapping[numEdges];
	int32 numMaps = 0;

	//generate adjacency
	int32 oldVertexCount = oldPolygonGroup->GetVertexCount();
	int32 * adjacency = new int32[oldVertexCount];
	Memset(adjacency, -1, oldVertexCount*sizeof(int32));
	for(int32 i = 0; i < oldVertexCount; ++i)
	{
		Vector3 newFoundCoord;
		oldPolygonGroup->GetCoord(i, newFoundCoord);
		adjacency[i] = i;
		for(int32 j = 0; j < i; ++j)
		{
			Vector3 oldCoord;
			oldPolygonGroup->GetCoord(j, oldCoord);
			if(EdgeAdjacency::IsPointsEqual(newFoundCoord, oldCoord))
			{
				adjacency[i] = j;
				break;
			}
		}
	}

	PolygonGroup * newPolygonGroup = new PolygonGroup();
	newPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL, oldIndexCount, oldIndexCount + numEdges*3);
	int32 nextIndex = 0;

	int32 facesCount = oldIndexCount/3;
	for(int32 f = 0; f < facesCount; ++f)
	{
		//copy old vertex data
		int32 oldIndex0, oldIndex1, oldIndex2;
		Vector3 oldPos0, oldPos1, oldPos2;
		oldPolygonGroup->GetIndex(f*3+0, oldIndex0);
		oldPolygonGroup->GetCoord(oldIndex0, oldPos0);
		newPolygonGroup->SetCoord(f*3+0, oldPos0);
		newPolygonGroup->SetIndex(nextIndex++, f*3+0);

		oldPolygonGroup->GetIndex(f*3+1, oldIndex1);
		oldPolygonGroup->GetCoord(oldIndex1, oldPos1);
		newPolygonGroup->SetCoord(f*3+1, oldPos1);
		newPolygonGroup->SetIndex(nextIndex++, f*3+1);

		oldPolygonGroup->GetIndex(f*3+2, oldIndex2);
		oldPolygonGroup->GetCoord(oldIndex2, oldPos2);
		newPolygonGroup->SetCoord(f*3+2, oldPos2);
		newPolygonGroup->SetIndex(nextIndex++, f*3+2);

		//generate new normals
		Vector3 v0 = oldPos1 - oldPos0;
		Vector3 v1 = oldPos2 - oldPos0;
		Vector3 normal = v0.CrossProduct(v1);
		normal.Normalize();

		newPolygonGroup->SetNormal(f*3+0, normal);
		newPolygonGroup->SetNormal(f*3+1, normal);
		newPolygonGroup->SetNormal(f*3+2, normal);


		//edge 1
		int32 nIndex;
		int32 vertIndex[3] = 
		{
			adjacency[oldIndex0],
			adjacency[oldIndex1],
			adjacency[oldIndex2]
		};
		nIndex = FindEdgeInMappingTable(vertIndex[0], vertIndex[1], mapping, numEdges);

		if(mapping[nIndex].oldEdge[0] == -1 && mapping[nIndex].oldEdge[1] == -1)
		{
			// No entry for this edge yet.  Initialize one.
			mapping[nIndex].oldEdge[0] = vertIndex[0];
			mapping[nIndex].oldEdge[1] = vertIndex[1];
			mapping[nIndex].newEdge[0][0] = f*3 + 0;
			mapping[nIndex].newEdge[0][1] = f*3 + 1;

			++numMaps;
		}
		else
		{
			// An entry is found for this edge.  Create
			// a quad and output it.
			mapping[nIndex].newEdge[1][0] = f*3 + 0;
			mapping[nIndex].newEdge[1][1] = f*3 + 1;

			// First triangle
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);

			// Second triangle
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);

			// pMapping[nIndex] is no longer needed. Copy the last map entry
			// over and decrement the map count.
			mapping[nIndex] = mapping[numMaps - 1];
			Memset(&mapping[numMaps - 1], 0xFF, sizeof(mapping[numMaps - 1]));
			--numMaps;
		}

		//edge 2
		nIndex = FindEdgeInMappingTable(vertIndex[1], vertIndex[2], mapping, numEdges);

		if(mapping[nIndex].oldEdge[0] == -1 && mapping[nIndex].oldEdge[1] == -1)
		{
			mapping[nIndex].oldEdge[0] = vertIndex[1];
			mapping[nIndex].oldEdge[1] = vertIndex[2];
			mapping[nIndex].newEdge[0][0] = f*3 + 1;
			mapping[nIndex].newEdge[0][1] = f*3 + 2;

			++numMaps;
		}
		else
		{
			mapping[nIndex].newEdge[1][0] = f*3 + 1;
			mapping[nIndex].newEdge[1][1] = f*3 + 2;

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);

			mapping[nIndex] = mapping[numMaps - 1];
			Memset(&mapping[numMaps - 1], 0xFF, sizeof(mapping[numMaps - 1]));
			--numMaps;
		}

		//edge 3
		nIndex = FindEdgeInMappingTable(vertIndex[2], vertIndex[0], mapping, numEdges);

		if(mapping[nIndex].oldEdge[0] == -1 && mapping[nIndex].oldEdge[1] == -1)
		{
			mapping[nIndex].oldEdge[0] = vertIndex[2];
			mapping[nIndex].oldEdge[1] = vertIndex[0];
			mapping[nIndex].newEdge[0][0] = f*3 + 2;
			mapping[nIndex].newEdge[0][1] = f*3 + 0;

			++numMaps;
		}
		else
		{
			mapping[nIndex].newEdge[1][0] = f*3 + 2;
			mapping[nIndex].newEdge[1][1] = f*3 + 0;

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);

			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][1]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[1][0]);
			newPolygonGroup->SetIndex(nextIndex++, mapping[nIndex].newEdge[0][0]);

			mapping[nIndex] = mapping[numMaps - 1];
			Memset(&mapping[numMaps - 1], 0xFF, sizeof(mapping[numMaps - 1]));
			--numMaps;
		}
	}

	int32 nextVertex = oldIndexCount;

	//patch holes
	if(numMaps > 0)
	{
		PolygonGroup * patchPolygonGroup = new PolygonGroup();
		// Make enough room in IB for the face and up to 3 quads for each patching face
		patchPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL, oldIndexCount+numMaps*3, nextIndex + numMaps*7*3);

		Memcpy(patchPolygonGroup->meshData, newPolygonGroup->meshData, newPolygonGroup->GetVertexCount()*newPolygonGroup->vertexStride);
		Memcpy(patchPolygonGroup->indexArray, newPolygonGroup->indexArray, newPolygonGroup->GetIndexCount()*sizeof(int16));

		SafeRelease(newPolygonGroup);
		newPolygonGroup = patchPolygonGroup;

		// Now, we iterate through the edge mapping table and
		// for each shared edge, we generate a quad.
		// For each non-shared edge, we patch the opening
		// with new faces.
		
		for(int32 i = 0; i < numMaps; ++i)
		{
			if(mapping[i].oldEdge[0] != -1 && mapping[i].oldEdge[1] != -1)
			{
				// If the 2nd new edge indexes is -1,
				// this edge is a non-shared one.
				// We patch the opening by creating new
				// faces.
				if(mapping[i].newEdge[1][0] == -1 || mapping[i].newEdge[1][1] == -1) // must have only one new edge
				{
					// Find another non-shared edge that
					// shares a vertex with the current edge.
					for(int32 i2 = i + 1; i2 < numMaps; ++i2)
					{
						if(mapping[i2].oldEdge[0] != -1 && mapping[i2].oldEdge[1] != -1      // must have a valid old edge
							&&(mapping[i2].newEdge[1][0] == -1 || mapping[i2].newEdge[1][1] == -1))// must have only one new edge
						{
							int32 nVertShared = 0;
							if(mapping[i2].oldEdge[0] == mapping[i].oldEdge[1])
								++nVertShared;
							if(mapping[i2].oldEdge[1] == mapping[i].oldEdge[0])
								++nVertShared;

							if(2 == nVertShared)
							{
								// These are the last two edges of this particular
								// opening. Mark this edge as shared so that a degenerate
								// quad can be created for it.

								mapping[i2].newEdge[1][0] = mapping[i].newEdge[0][0];
								mapping[i2].newEdge[1][1] = mapping[i].newEdge[0][1];
								break;
							}
							else if(1 == nVertShared)
							{
								// nBefore and nAfter tell us which edge comes before the other.
								int32 nBefore, nAfter;
								if(mapping[i2].oldEdge[0] == mapping[i].oldEdge[1])
								{
									nBefore = i;
									nAfter = i2;
								}
								else
								{
									nBefore = i2;
									nAfter = i;
								}

								// Found such an edge. Now create a face along with two
								// degenerate quads from these two edges.
								Vector3 coord0, coord1, coord2;
								newPolygonGroup->GetCoord(mapping[nAfter].newEdge[0][1], coord0);
								newPolygonGroup->GetCoord(mapping[nBefore].newEdge[0][1], coord1);
								newPolygonGroup->GetCoord(mapping[nBefore].newEdge[0][0], coord2);

								newPolygonGroup->SetCoord(nextVertex+0, coord0);
								newPolygonGroup->SetCoord(nextVertex+1, coord1);
								newPolygonGroup->SetCoord(nextVertex+2, coord2);

								// Recompute the normal
								Vector3 v0 = coord1 - coord0;
								Vector3 v1 = coord2 - coord0;
								Vector3 normal = v0.CrossProduct(v1);
								normal.Normalize();

								newPolygonGroup->SetNormal(nextVertex+0, normal);
								newPolygonGroup->SetNormal(nextVertex+1, normal);
								newPolygonGroup->SetNormal(nextVertex+2, normal);

								newPolygonGroup->SetIndex(nextIndex+0, nextVertex+0);
								newPolygonGroup->SetIndex(nextIndex+1, nextVertex+1);
								newPolygonGroup->SetIndex(nextIndex+2, nextVertex+2);

								// 1st quad
								newPolygonGroup->SetIndex(nextIndex+3, mapping[nBefore].newEdge[0][1]);
								newPolygonGroup->SetIndex(nextIndex+4, mapping[nBefore].newEdge[0][0]);
								newPolygonGroup->SetIndex(nextIndex+5, nextVertex + 1);

								newPolygonGroup->SetIndex(nextIndex+6, nextVertex + 2);
								newPolygonGroup->SetIndex(nextIndex+7, nextVertex + 1);
								newPolygonGroup->SetIndex(nextIndex+8, mapping[nBefore].newEdge[0][0]);

								// 2nd quad
								newPolygonGroup->SetIndex(nextIndex+9, mapping[nAfter].newEdge[0][1]);
								newPolygonGroup->SetIndex(nextIndex+10, mapping[nAfter].newEdge[0][0]);
								newPolygonGroup->SetIndex(nextIndex+11, nextVertex);

								newPolygonGroup->SetIndex(nextIndex+12, nextVertex + 1);
								newPolygonGroup->SetIndex(nextIndex+13,  nextVertex);
								newPolygonGroup->SetIndex(nextIndex+14, mapping[nAfter].newEdge[0][0]);

								// Modify mapping entry i2 to reflect the third edge
								// of the newly added face.
								if(mapping[i2].oldEdge[0] == mapping[i].oldEdge[1])
								{
									mapping[i2].oldEdge[0] = mapping[i].oldEdge[0];
								}
								else
								{
									mapping[i2].oldEdge[1] = mapping[i].oldEdge[1];
								}
								mapping[i2].newEdge[0][0] = nextVertex + 2;
								mapping[i2].newEdge[0][1] = nextVertex;

								// Update next vertex/index positions
								nextVertex += 3;
								nextIndex += 15;

								break;
							}
						}
					}
				}
				else
				{
					// This is a shared edge.  Create the degenerate quad.
					// First triangle
					newPolygonGroup->SetIndex(nextIndex++, mapping[i].newEdge[0][1]);
					newPolygonGroup->SetIndex(nextIndex++, mapping[i].newEdge[0][0]);
					newPolygonGroup->SetIndex(nextIndex++, mapping[i].newEdge[1][0]);

					// Second triangle
					newPolygonGroup->SetIndex(nextIndex++, mapping[i].newEdge[1][1]);
					newPolygonGroup->SetIndex(nextIndex++,  mapping[i].newEdge[1][0]);
					newPolygonGroup->SetIndex(nextIndex++, mapping[i].newEdge[0][0]);
				}
			}
		}
	}

	SafeRelease(shadowPolygonGroup);
	shadowPolygonGroup = new PolygonGroup();
	shadowPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL, nextVertex, nextIndex);
	Memcpy(shadowPolygonGroup->meshData, newPolygonGroup->meshData, nextVertex*newPolygonGroup->vertexStride);
	Memcpy(shadowPolygonGroup->indexArray, newPolygonGroup->indexArray, nextIndex*sizeof(int16));

	SafeRelease(newPolygonGroup);
    SafeDeleteArray(mapping);
    SafeDeleteArray(adjacency);
}

void ShadowVolumeNode::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
	Entity::Save(archive, serializationContext);

	archive->SetByteArrayAsType("pg", (uint64)shadowPolygonGroup);
}

void ShadowVolumeNode::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	Entity::Load(archive, serializationContext);

	uint64 ptr = archive->GetByteArrayAsType("pg", (uint64)0);
	shadowPolygonGroup = static_cast<PolygonGroup*>(serializationContext->GetDataBlock(ptr));
	SafeRetain(shadowPolygonGroup);
}

void ShadowVolumeNode::GetDataNodes(Set<DataNode*> & dataNodes)
{
	dataNodes.insert(shadowPolygonGroup);
}

Entity* ShadowVolumeNode::Clone(Entity *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		DVASSERT_MSG(IsPointerToExactClass<ShadowVolumeNode>(this), "Can clone only ShadowVolumeNode");
		dstNode = new ShadowVolumeNode();
	}

	Entity::Clone(dstNode);
	ShadowVolumeNode *nd = (ShadowVolumeNode *)dstNode;

	nd->shadowPolygonGroup = shadowPolygonGroup;
    SafeRetain(nd->shadowPolygonGroup);

	return dstNode;
}


};
