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


#include "Render/3D/PolygonGroup.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RenderHelper.h"
#include "Render/RenderManager.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{
    
PolygonGroup::PolygonGroup()
:	DataNode(),
    vertexCount(0),
	indexCount(0),
	textureCoordCount(0),
	cubeTextureCoordCount(0),
	vertexStride(0),
	vertexFormat(0),
    indexFormat(EIF_16),
	triangleCount(0),
	
	vertexArray(0), 
	textureCoordArray(0),
	cubeTextureCoordArray(0),
	normalArray(0), 
	tangentArray(0),
	binormalArray(0),
	jointIdxArray(0),
	weightArray(0),
	jointCountArray(0),
	
	colorArray(0), 
	indexArray(0), 
	meshData(0),
	baseVertexArray(0),
    renderDataObject(0),
    primitiveType(PRIMITIVETYPE_TRIANGLELIST)
{
}

PolygonGroup::~PolygonGroup()
{
	ReleaseData();
}
    
void PolygonGroup::UpdateDataPointersAndStreams()
{
    int32 baseShift = 0;
	if (vertexFormat & EVF_VERTEX)
	{
		vertexArray = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_VERTEX);
        
        renderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, vertexStride, vertexArray);
    }
	if (vertexFormat & EVF_NORMAL)
	{
		normalArray = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_NORMAL);
        
        renderDataObject->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, vertexStride, normalArray);
	}
	if (vertexFormat & EVF_COLOR)
	{
		colorArray = reinterpret_cast<uint32*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_COLOR);
        
        renderDataObject->SetStream(EVF_COLOR, TYPE_UNSIGNED_BYTE, 4, vertexStride, colorArray);
    }
	if (vertexFormat & EVF_TEXCOORD0)
	{
		textureCoordArray[0] = reinterpret_cast<Vector2*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_TEXCOORD0);
        
        renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, textureCoordArray[0]);
	}
	if (vertexFormat & EVF_TEXCOORD1)
	{
		textureCoordArray[1] = reinterpret_cast<Vector2*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_TEXCOORD1);
        
        renderDataObject->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 2, vertexStride, textureCoordArray[1]);
    }
	if (vertexFormat & EVF_TEXCOORD2)
	{
		textureCoordArray[2] = reinterpret_cast<Vector2*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_TEXCOORD2);
        
        renderDataObject->SetStream(EVF_TEXCOORD2, TYPE_FLOAT, 2, vertexStride, textureCoordArray[2]);
	}
	if (vertexFormat & EVF_TEXCOORD3)
	{
		textureCoordArray[3] = reinterpret_cast<Vector2*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_TEXCOORD3);
        
        renderDataObject->SetStream(EVF_TEXCOORD3, TYPE_FLOAT, 2, vertexStride, textureCoordArray[3]);
	}	
	if (vertexFormat & EVF_TANGENT)
	{
		tangentArray = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_TANGENT);
        
        renderDataObject->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, vertexStride, tangentArray);
	}
	if (vertexFormat & EVF_BINORMAL)
	{
		binormalArray = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_BINORMAL);
        
        renderDataObject->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, vertexStride, binormalArray);
	}
	if (vertexFormat & EVF_JOINTWEIGHT)
	{
		jointIdxArray = reinterpret_cast<int32*>(meshData + baseShift);
		weightArray = reinterpret_cast<float32*>(meshData + baseShift + 4 * 4);
		baseShift += GetVertexSize(EVF_JOINTWEIGHT);
		
		SafeDeleteArray(jointCountArray);
		jointCountArray = new int32[vertexCount];
	}
	if (vertexFormat & EVF_CUBETEXCOORD0)
	{
		cubeTextureCoordArray[0] = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_CUBETEXCOORD0);
        
        renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 3, vertexStride, cubeTextureCoordArray[0]);
	}
	if (vertexFormat & EVF_CUBETEXCOORD1)
	{
		cubeTextureCoordArray[1] = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_CUBETEXCOORD1);
        
        renderDataObject->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 3, vertexStride, cubeTextureCoordArray[1]);
    }
	if (vertexFormat & EVF_CUBETEXCOORD2)
	{
		cubeTextureCoordArray[2] = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_CUBETEXCOORD2);
        
        renderDataObject->SetStream(EVF_TEXCOORD2, TYPE_FLOAT, 3, vertexStride, cubeTextureCoordArray[2]);
	}
	if (vertexFormat & EVF_CUBETEXCOORD3)
	{
		cubeTextureCoordArray[3] = reinterpret_cast<Vector3*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_CUBETEXCOORD3);
        
        renderDataObject->SetStream(EVF_TEXCOORD3, TYPE_FLOAT, 3, vertexStride, cubeTextureCoordArray[3]);
	}
}

void PolygonGroup::AllocateData(int32 _meshFormat, int32 _vertexCount, int32 _indexCount)
{
	vertexCount = _vertexCount;
	indexCount = _indexCount;
	vertexStride = GetVertexSize(_meshFormat);
	vertexFormat = _meshFormat;
	textureCoordCount = GetTexCoordCount(vertexFormat);
	cubeTextureCoordCount = GetCubeTexCoordCount(vertexFormat);

	meshData = new uint8[vertexStride * vertexCount];
	indexArray = new int16[indexCount];
	textureCoordArray = new Vector2*[textureCoordCount];
	cubeTextureCoordArray = new Vector3*[cubeTextureCoordCount];
	
    renderDataObject = new RenderDataObject();
    
    UpdateDataPointersAndStreams();
}
    
void PolygonGroup::TangentVectors(  const Vector3 &v0, 
                                    const Vector3 &v1, 
                                    const Vector3 &v2, 
                                    const Vector2 &t0, 
                                    const Vector2 &t1, 
                                    const Vector2 &t2, 
                                    Vector3 &sdir, 
                                    Vector3 &tdir, 
                                    Vector3 &normal)
{
    Vector3 dv0 = v1 - v0;
    Vector3 dv1 = v2 - v0;
    
    Vector2 dt0 = t1 - t0;
    Vector2 dt1 = t2 - t0;
    
    float r = 1.0f / (dt0.x * dt1.y - dt1.x * dt0.y);
    sdir = Vector3(dt1.y * dv0.x - dt0.y * dv1.x, dt1.y * dv0.y - dt0.y * dv1.y, dt1.y * dv0.z - dt0.y * dv1.z) * r;
    tdir = Vector3(dt0.x * dv1.x - dt1.x * dv0.x, dt0.x * dv1.y - dt1.x * dv0.y, dt0.x * dv1.z - dt1.x * dv0.z) * r;
    normal = CrossProduct(dv0, dv1);
    normal.Normalize();
}
    
void PolygonGroup::BuildTangentsBinormals(uint32 flagsToAdd)
{
    if (!(vertexFormat & EVF_TANGENT))
    {
        // Dirty hack. Copy pointers of this polygon group 
        //        PolygonGroup * oldGroup = this;
        //        AllocateData(oldGroup->GetVertexFormat() | flagsToAdd, oldGroup->GetVertexCount(), oldGroup->GetIndexCount());
        //        CopyFrom(oldGroup);
        //        SafeRelease(oldGroup);
    }
    
    for (int v = 0; v < vertexCount; ++v)
    {
        //Vector3 * tan0 = (Vector3 *)((uint8 *)normalArray + v * vertexStride);  
        Vector3 * tan1 = (Vector3 *)((uint8 *)tangentArray + v * vertexStride);  
        Vector3 * tan2 = (Vector3 *)((uint8 *)binormalArray + v * vertexStride);  
        //*tan0 = Vector3(0.0f, 0.0f, 0.0f);
        tan1->x = 0;
        tan1->y = 0;
        tan1->z = 0;
        
        tan2->x = 0;
        tan2->y = 0;
        tan2->z = 0;
    }
    
    for (int t = 0; t < triangleCount; ++t)	
    {
        int32 i1 = indexArray[t * 3];
        int32 i2 = indexArray[t * 3 + 1];
        int32 i3 = indexArray[t * 3 + 2];
        
        Vector3 v1 = *(Vector3 *)((uint8 *)vertexArray + i1 * vertexStride);  
        Vector3 v2 = *(Vector3 *)((uint8 *)vertexArray + i2 * vertexStride);  
        Vector3 v3 = *(Vector3 *)((uint8 *)vertexArray + i3 * vertexStride);  
        
        
        // use first texture coordinate
        Vector2 w1 = *(Vector2 *)((uint8 *)textureCoordArray[0] + i1 * vertexStride);  
        Vector2 w2 = *(Vector2 *)((uint8 *)textureCoordArray[0] + i2 * vertexStride);  
        Vector2 w3 = *(Vector2 *)((uint8 *)textureCoordArray[0] + i3 * vertexStride);  
        
        //float32 x1 = v2.x - v1.x;
        //float32 x2 = v3.x - v1.x;
        //float32 y1 = v2.y - v1.y;
        //float32 y2 = v3.y - v1.y;
        //float32 z1 = v2.z - v1.z;
        //float32 z2 = v3.z - v1.z;
        
        //float32 s1 = w2.x - w1.x;
        //float32 s2 = w3.x - w1.x;
        //float32 t1 = w2.y - w1.y;
        //float32 t2 = w3.y - w1.y;
        
        //float r = 1.0F / (s1 * t2 - s2 * t1);
        //Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
        //	(t2 * z1 - t1 * z2) * r);
        //Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
        //	(s1 * z2 - s2 * z1) * r);
        
        
        Vector3 sdir, tdir, norm;
        
        TangentVectors(v1, v2, v3, w1, w2, w3, sdir, tdir, norm);
        
        //Vector3 * nor1 = (Vector3 *)((uint8 *)normalArray + i1 * vertexStride);  
        //Vector3 * nor2 = (Vector3 *)((uint8 *)normalArray + i2 * vertexStride);  
        //Vector3 * nor3 = (Vector3 *)((uint8 *)normalArray + i3 * vertexStride);  
        
        Vector3 * tan1 = (Vector3 *)((uint8 *)tangentArray + i1 * vertexStride);  
        Vector3 * tan2 = (Vector3 *)((uint8 *)tangentArray + i2 * vertexStride);  
        Vector3 * tan3 = (Vector3 *)((uint8 *)tangentArray + i3 * vertexStride);  
        
        Vector3 * tan21 = (Vector3 *)((uint8 *)binormalArray + i1 * vertexStride);  
        Vector3 * tan22 = (Vector3 *)((uint8 *)binormalArray + i2 * vertexStride);  
        Vector3 * tan23 = (Vector3 *)((uint8 *)binormalArray + i3 * vertexStride);  
        
        
        //*nor1 += norm;
        //*nor2 += norm;
        //*nor3 += norm;
        
        *tan1 += sdir;
        *tan2 += sdir;
        *tan3 += sdir;
        
        *tan21 += tdir;
        *tan22 += tdir;
        *tan23 += tdir;
    }
    
    for (int v = 0; v < vertexCount; ++v)
    {
        Vector3 * nres = (Vector3 *)((uint8 *)normalArray + v * vertexStride);  
        Vector3 * tres = (Vector3 *)((uint8 *)tangentArray + v * vertexStride);  
        Vector3 * bres = (Vector3 *)((uint8 *)binormalArray + v * vertexStride);  
        
        *bres = -*bres;
        nres->Normalize();
        tres->Normalize();
        bres->Normalize();
        
        //Vector3 n = *nres;
        //Vector3 t = *tres;
        //Vector3 t2 = *bres;
        
        //*tres = (t - DotProduct(n, t) * n);
        //
        ////tres->x = -tres->x;
        ////tres->y = -tres->y;
        ////tres->z = -tres->z;
        ////nres->x = -nres->x;
        ////nres->y = -nres->y;
        ////nres->z = -nres->z;
        
        
        //tres->Normalize();
        //
        //float32 handedness = (DotProduct(CrossProduct(n, t), t2) < 0.0f) ? (-1.0f) : (1.0f);
        
        //if (vertexFormat & EVF_BINORMAL)
        //{
        //	*bres = CrossProduct(*tres, *nres);
        //	bres->Normalize();
        //}
        ////*bres = -*bres;
        //*tres = -*tres;
        
        /*
         BORODA: Removed unused variable
         Vector3 nrecomp = CrossProduct(*tres, *bres);
        */
        
        // int xt = 0;
        // use this method (get from .. http://www.c4engine.com/code/tangent.html);
        //const Vector3D& n = normal[a];
        //const Vector3D& t = tan1[a];
        //// Gram-Schmidt orthogonalize
        // tangent[a] = (t - n * Dot(n, t)).Normalize();
        //
        // // Calculate handedness
        // tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
    }
}
    
void PolygonGroup::CreateBaseVertexArray()
{
	SafeDeleteArray(baseVertexArray);
	baseVertexArray = new Vector3[vertexCount];

	for (int v = 0; v < vertexCount; ++v)
	{
		GetCoord(v, baseVertexArray[v]);
	}
}
    
void PolygonGroup::ApplyMatrix(const Matrix4 & matrix)
{
    aabbox = AABBox3(); // reset bbox
    
    Matrix4 normalMatrix4;
    matrix.GetInverse(normalMatrix4);
    normalMatrix4.Transpose();
    Matrix3 normalMatrix3;
    normalMatrix3 = normalMatrix4;
    
    if(NULL != vertexArray)
    {
        for (int32 vi = 0; vi < vertexCount; ++vi)
        {
            Vector3 vertex;
            GetCoord(vi, vertex);
            vertex = vertex * matrix;
            SetCoord(vi, vertex);
        
            if(NULL != normalArray)
            {
                Vector3 normal;
                GetNormal(vi, normal);
                normal = normal * normalMatrix3;
                SetNormal(vi, normal);
            }
        }
    }
}
	    
void PolygonGroup::ReleaseData()
{
    SafeRelease(renderDataObject);
    
    SafeDeleteArray(jointCountArray);
    SafeDeleteArray(meshData);
    SafeDeleteArray(indexArray);
    SafeDeleteArray(textureCoordArray);
	SafeDeleteArray(cubeTextureCoordArray);
}
	
void PolygonGroup::BuildBuffers()
{
    JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &PolygonGroup::BuildBuffersInternal));
};
    
void PolygonGroup::BuildBuffersInternal(BaseObject * caller, void * param, void *callerData)
{
    DVASSERT(Thread::IsMainThread());

    UpdateDataPointersAndStreams();
    
    renderDataObject->BuildVertexBuffer(vertexCount);
    renderDataObject->SetIndices((eIndexFormat)indexFormat, (uint8*)indexArray, indexCount);
    renderDataObject->BuildIndexBuffer();
}


    
void PolygonGroup::Save(KeyedArchive * keyedArchive, SerializationContext * serializationContext)
{
    DataNode::Save(keyedArchive, serializationContext);
    
    keyedArchive->SetInt32("vertexFormat", vertexFormat);
    keyedArchive->SetInt32("vertexCount", vertexCount); 
    keyedArchive->SetInt32("indexCount", indexCount); 
    keyedArchive->SetInt32("textureCoordCount", textureCoordCount);
    keyedArchive->SetInt32("primitiveType", primitiveType);
                           
    keyedArchive->SetInt32("packing", PACKING_NONE);
    keyedArchive->SetByteArray("vertices", meshData, vertexCount * vertexStride);
    keyedArchive->SetInt32("indexFormat", indexFormat);
    keyedArchive->SetByteArray("indices", (uint8*)indexArray, indexCount * INDEX_FORMAT_SIZE[indexFormat]);
	keyedArchive->SetInt32("cubeTextureCoordCount", cubeTextureCoordCount);

//    for (int32 k = 0; k < GetVertexCount(); ++k)
//    {
//        Vector3 normal;
//        GetNormal(k, normal);
//        Logger::FrameworkDebug("savenorm2: %f %f %f", normal.x, normal.y, normal.z);
//    }
    

}

void PolygonGroup::Load(KeyedArchive * keyedArchive, SerializationContext * serializationContext)
{
    DataNode::Load(keyedArchive, serializationContext);
    
    vertexFormat = keyedArchive->GetInt32("vertexFormat");
    vertexStride = GetVertexSize(vertexFormat);
    vertexCount = keyedArchive->GetInt32("vertexCount");
    indexCount = keyedArchive->GetInt32("indexCount");
    textureCoordCount = keyedArchive->GetInt32("textureCoordCount");
    primitiveType = (ePrimitiveType)keyedArchive->GetInt32("primitiveType");
	cubeTextureCoordCount = keyedArchive->GetInt32("cubeTextureCoordCount");
    
    int32 formatPacking = keyedArchive->GetInt32("packing");
    if (formatPacking == PACKING_NONE)
    {
        int size = keyedArchive->GetByteArraySize("vertices");
        if (size != vertexCount * vertexStride)
        {
            Logger::Error("PolygonGroup::Load - Something is going wrong, size of vertex array is incorrect");
            return;
        }
		SafeDeleteArray(meshData);
        meshData = new uint8[vertexCount * vertexStride];
        const uint8 * archiveData = keyedArchive->GetByteArray("vertices");
        memcpy(meshData, archiveData, size);
    }
    
    indexFormat = keyedArchive->GetInt32("indexFormat");
    if (indexFormat == EIF_16)
    {
        int size = keyedArchive->GetByteArraySize("indices");
        if (size != indexCount * INDEX_FORMAT_SIZE[indexFormat])
        {
            Logger::Error("PolygonGroup::Load - Something is going wrong, size of index array is incorrect");   
            return;
        }
		SafeDeleteArray(indexArray);
        indexArray = new int16[indexCount];
        const uint8 * archiveData = keyedArchive->GetByteArray("indices");
        memcpy(indexArray, archiveData, indexCount * INDEX_FORMAT_SIZE[indexFormat]);         
    }

	SafeDeleteArray(textureCoordArray);
	textureCoordArray = new Vector2*[textureCoordCount];
	
	SafeDeleteArray(cubeTextureCoordArray);
	if(cubeTextureCoordCount)
	{
		cubeTextureCoordArray = new Vector3*[cubeTextureCoordCount];
	}

	SafeRelease(renderDataObject);
    renderDataObject = new RenderDataObject();
    UpdateDataPointersAndStreams();
    RecalcAABBox();
    
    BuildBuffers();
}
    
void PolygonGroup::RecalcAABBox()
{
    aabbox = AABBox3(); // reset bbox

    // recalc aabbox
    for (int vi = 0; vi < vertexCount; ++vi)
    {
        Vector3 point;
        GetCoord(vi, point);
        aabbox.AddPoint(point);
    }
}
    
void PolygonGroup::DebugDraw()
{
    RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    for (int k = 0; k < indexCount / 3; ++k)
    {
        Vector3 v0, v1, v2;
        GetCoord(indexArray[k * 3 + 0], v0);
        GetCoord(indexArray[k * 3 + 1], v1);
        GetCoord(indexArray[k * 3 + 2], v2);
        RenderHelper::Instance()->DrawLine(v0, v1, 1.0f, RenderState::RENDERSTATE_2D_BLEND);
        RenderHelper::Instance()->DrawLine(v1, v2, 1.0f, RenderState::RENDERSTATE_2D_BLEND);
        RenderHelper::Instance()->DrawLine(v0, v2, 1.0f, RenderState::RENDERSTATE_2D_BLEND);
    }
}

/*class VertexQuadTree
{
public:
    VertexOctTree()
    {
        
    }
    
    ~VertexOctTree()
    {
        
    }
    
    struct Vertex
    {
        Vector3 position;
        uint32  color;
        Vector3 normal;
        Vector2 texCoords[4];
    };
    
    DynamicObjectCacheData<Vertex>
};*/

void PolygonGroup::CopyData(uint8 ** meshData, uint8 ** newMeshData, uint32 vertexFormat, uint32 newVertexFormat, uint32 format) const
{
	if (vertexFormat & format)
	{
		uint32 formatSize = GetVertexSize(format);
		if (newVertexFormat & format)
		{
			memcpy(*newMeshData, *meshData, formatSize);
			*newMeshData += formatSize;
		}
		*meshData += formatSize;
	}
}

bool PolygonGroup::IsFloatDataEqual(const float32 ** meshData, const float32 ** optData, uint32 vertexFormat, uint32 format) const
{
	if (vertexFormat & format)
	{
		uint32 size = GetVertexSize(format);
		uint32 count = size / sizeof(float32);
		for (uint32 i = 0; i < count; ++i)
		{
			float32 x1 = **meshData;
			*meshData += sizeof(float32);
			float32 x2 = **optData;
			*optData += sizeof(float32);
			if (!FLOAT_EQUAL_EPS(x1, x2, 0.00001f))
				return false;
		}
	}
	return true;
}
	
int32 PolygonGroup::OptimazeVertexes(const uint8 * meshData, Vector<uint8> & optMeshData, uint32 vertexFormat) const
{
	uint32 optimizedVertexCount = optMeshData.size() / GetVertexSize(vertexFormat);

	for (uint32 i = 0; i < optimizedVertexCount; ++i)
	{
		const float32 * optData = (float32*)(&optMeshData[i * GetVertexSize(vertexFormat)]);
		const float32 * tmpMeshData = (float32*) meshData;
		
		bool skip = false;
		for (uint32 mask = EVF_LOWER_BIT; mask <= EVF_HIGHER_BIT; mask = mask << 1)
		{
			if (!IsFloatDataEqual(&tmpMeshData, &optData, vertexFormat, mask))
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		return i;
	}

	optMeshData.resize(GetVertexSize(vertexFormat) * (optimizedVertexCount + 1));
	
	uint8* dest = &optMeshData[GetVertexSize(vertexFormat) * optimizedVertexCount];
	memcpy(dest, meshData, GetVertexSize(vertexFormat));
	return optimizedVertexCount;
}
	
void PolygonGroup::OptimizeVertices(uint32 newVertexFormat, float32 eplison)
{	
	int32 newVertexStride = GetVertexSize(newVertexFormat);
	uint8 * newMeshData = new uint8[newVertexStride * vertexCount];
	memset(newMeshData, 0, sizeof(newVertexStride * vertexCount));
	
	uint8 * tmpMesh = meshData;
	uint8 * tmpNewMesh = newMeshData;
	for (int32 i = 0; i < vertexCount; ++i)
	{
		for (uint32 mask = EVF_LOWER_BIT; mask <= EVF_HIGHER_BIT; mask = mask << 1)
		{
			CopyData(&tmpMesh, &tmpNewMesh, vertexFormat, newVertexFormat, mask);
		}
	}
	
	Vector<uint8> optMeshData;
	tmpNewMesh = newMeshData;
	for (int32 i = 0; i < vertexCount; ++i)
	{
		int16 newIndex = OptimazeVertexes(tmpNewMesh, optMeshData, newVertexFormat);
		tmpNewMesh += GetVertexSize(newVertexFormat);
		
		for (int32 i1 = 0; i1 < indexCount; ++i1)
		{
			if (indexArray[i1] == i)
				indexArray[i1] = newIndex;
		}
	}

	uint32 newVertexCount = optMeshData.size() / GetVertexSize(newVertexFormat);
	
	SAFE_DELETE_ARRAY(meshData);
	SAFE_DELETE_ARRAY(newMeshData);
	vertexCount = newVertexCount;
	vertexStride = newVertexStride;
	vertexFormat = newVertexFormat;
	
	meshData = new uint8[vertexStride * vertexCount];
	memcpy(meshData, &optMeshData[0], vertexStride * vertexCount);
};


void MeshConverter::CopyVertex(PolygonGroup *srcGroup, uint32 srcPos, PolygonGroup *dstGroup, uint32 dstPos)
{
    int32 copyFormat = srcGroup->GetFormat() & dstGroup->GetFormat();    //most common format;   

    if (copyFormat&EVF_VERTEX)
    {
        Vector3 v;
        srcGroup->GetCoord(srcPos, v);
        dstGroup->SetCoord(dstPos, v);
        copyFormat&=~EVF_VERTEX;
    }

    if (copyFormat&EVF_COLOR)
    {
        uint32 v;
        srcGroup->GetColor(srcPos, v);
        dstGroup->SetColor(dstPos, v);
        copyFormat&=~EVF_COLOR;
    }

    if (copyFormat&EVF_NORMAL)
    {
        Vector3 v;
        srcGroup->GetNormal(srcPos, v);
        dstGroup->SetNormal(dstPos, v);
        copyFormat&=~EVF_NORMAL;
    }

    if (copyFormat&EVF_TANGENT)
    {
        Vector3 v;
        srcGroup->GetTangent(srcPos, v);
        dstGroup->SetTangent(dstPos, v);
        copyFormat&=~EVF_TANGENT;
    }

    if (copyFormat&EVF_BINORMAL)
    {
        Vector3 v;
        srcGroup->GetBinormal(srcPos, v);
        dstGroup->SetBinormal(dstPos, v);
        copyFormat&=~EVF_BINORMAL;
    }

    if (copyFormat&EVF_TEXCOORD0)
    {
        Vector2 v;
        srcGroup->GetTexcoord(0, srcPos, v);
        dstGroup->SetTexcoord(0, dstPos, v);
        copyFormat&=~EVF_TEXCOORD0;
    }

    if (copyFormat&EVF_TEXCOORD1)
    {
        Vector2 v;
        srcGroup->GetTexcoord(1, srcPos, v);
        dstGroup->SetTexcoord(1, dstPos, v);
        copyFormat&=~EVF_TEXCOORD1;
    }

    if (copyFormat&EVF_TEXCOORD2)
    {
        Vector2 v;
        srcGroup->GetTexcoord(2, srcPos, v);
        dstGroup->SetTexcoord(2, dstPos, v);
        copyFormat&=~EVF_TEXCOORD2;
    }

    if (copyFormat&EVF_TEXCOORD3)
    {
        Vector2 v;
        srcGroup->GetTexcoord(3, srcPos, v);
        dstGroup->SetTexcoord(3, dstPos, v);
        copyFormat&=~EVF_TEXCOORD3;
    }

    if (copyFormat&EVF_CUBETEXCOORD0)
    {
        Vector3 v;
        srcGroup->GetCubeTexcoord(0, srcPos, v);
        dstGroup->SetCubeTexcoord(0, dstPos, v);
        copyFormat&=~EVF_CUBETEXCOORD0;
    }

    if (copyFormat&EVF_CUBETEXCOORD1)
    {
        Vector3 v;
        srcGroup->GetCubeTexcoord(1, srcPos, v);
        dstGroup->SetCubeTexcoord(1, dstPos, v);
        copyFormat&=~EVF_CUBETEXCOORD1;
    }
    if (copyFormat&EVF_CUBETEXCOORD2)
    {
        Vector3 v;
        srcGroup->GetCubeTexcoord(2, srcPos, v);
        dstGroup->SetCubeTexcoord(2, dstPos, v);
        copyFormat&=~EVF_CUBETEXCOORD2;
    }
    if (copyFormat&EVF_CUBETEXCOORD3)
    {
        Vector3 v;
        srcGroup->GetCubeTexcoord(3, srcPos, v);
        dstGroup->SetCubeTexcoord(3, dstPos, v);
        copyFormat&=~EVF_CUBETEXCOORD3;
    }
    
    /*unsupported stream*/
    DVASSERT((copyFormat == 0)&&"Unsupported attribute stream in copy");
    
}

void MeshConverter::RebuildMeshTangentSpace(PolygonGroup *group, bool normalizeTangentSpace/*=true*/, bool computeBinormal/*=false*/)
{
    DVASSERT(group->GetPrimitiveType() == PRIMITIVETYPE_TRIANGLELIST); //only triangle lists for now
    DVASSERT(normalizeTangentSpace||computeBinormal); //can not use non-normalized tangent space without precomputed binormals
    DVASSERT(group->GetFormat()&EVF_TEXCOORD0);

    Vector<FaceWork> faces;
    uint32 faceCount = group->GetIndexCount()/3;
    faces.resize(faceCount);
    Vector<VertexWork> vertices_origin;
    Vector<VertexWork> vertices_full;
    vertices_origin.resize(group->GetVertexCount());
    vertices_full.resize(group->GetIndexCount());

    for (uint32 i=0, sz = group->GetVertexCount(); i<sz; ++i)
        vertices_origin[i].refIndex = i;
    //compute tangent for faces
    for (uint32 f=0; f<faceCount; ++f)
    {
        Vector3 pos[3];
        Vector2 texCoord[3];
        for (uint32 i=0; i<3; ++i)
        {
            int32 workIndex = f*3+i;
            int32 originIndex;
            group->GetIndex(workIndex, originIndex);
            faces[f].indexOrigin[i] = originIndex;
            group->GetCoord(originIndex, pos[i]);
            group->GetTexcoord(0, originIndex, texCoord[i]);
            
            vertices_origin[originIndex].refIndices.push_back(workIndex);
            vertices_full[f*3+i].refIndex = faces[f].indexOrigin[i];
        }                       
        
        float32 x10 = pos[1].x - pos[0].x;
        float32 y10 = pos[1].y - pos[0].y;
        float32 z10 = pos[1].z - pos[0].z;
        float32 u10 = texCoord[1].x-texCoord[0].x;
        float32 v10 = texCoord[1].y-texCoord[0].y;
        
        
        float32 x20 = pos[2].x - pos[0].x;
        float32 y20 = pos[2].y - pos[0].y;
        float32 z20 = pos[2].z - pos[0].z;
        float32 u20 = texCoord[2].x-texCoord[0].x;
        float32 v20 = texCoord[2].y-texCoord[0].y;

        float32 d = u10 * v20 - u20 * v10;

        if(d == 0.0f)
        {
            d = 1.0f;	// this may happen in case of degenerated triangle
        }
        d = 1.0f / d;


        Vector3 tangent = Vector3((v20 * x10 - v10 * x20) * d, (v20 * y10 - v10 * y20) * d, (v20 * z10 - v10 * z20) * d);
        Vector3 binormal = Vector3((x20 * u10 - x10 * u20) * d, (y20 * u10 - y10 * u20) * d, (z20 * u10 - z10 * u20) * d);

        if (normalizeTangentSpace) //should we normalize it here or only final result?
        {
            tangent.Normalize();
            binormal.Normalize();
        }
        faces[f].tangent = tangent;
        faces[f].binormal = binormal;
        for (int32 i=0; i<3; ++i)
        {
            vertices_full[f*3+i].tangent = tangent;            
            vertices_full[f*3+i].binormal = binormal;
        }
    }

    /*smooth tangent space preventing mirrored uv's smooth*/
    for (uint32 v = 0, sz = vertices_full.size(); v<sz; ++v)
    {
        int32 faceId = v/3;
        VertexWork& originVert = vertices_origin[vertices_full[v].refIndex];      
        vertices_full[v].tbRatio = 1;
        for (int32 iRef=0, refSz = originVert.refIndices.size(); iRef<refSz; ++iRef)
        {
            int32 refFaceId = originVert.refIndices[iRef]/3;
            if (refFaceId == faceId) continue;
            
            //check if uv's mirrored;
            
            bool isNotMirrored;
            if (computeBinormal)
            {
                //here we use handness to find mirrored UV's - still not sure if it is better then using dot product
                Vector3 n1 = CrossProduct(vertices_full[v].tangent, vertices_full[v].binormal);
                Vector3 n2 = CrossProduct(faces[refFaceId].tangent, faces[refFaceId].binormal);
                isNotMirrored = DotProduct(n1, n2)>0.0f;
            }
            else
            {
                isNotMirrored = DotProduct(vertices_full[v].tangent, faces[refFaceId].tangent)>0.0f;
            }

            
            if (isNotMirrored)
            {
                vertices_full[v].tangent+=faces[refFaceId].tangent;
                vertices_full[v].binormal+=faces[refFaceId].binormal;
                vertices_full[v].tbRatio++;
            }
            
        }

        if (normalizeTangentSpace)
        {
            vertices_full[v].tangent.Normalize();
            vertices_full[v].binormal.Normalize();
        }
        else
        {
            float32 invScale = 1.0f/(float32)vertices_full[v].tbRatio;
            vertices_full[v].tangent*=invScale;
            vertices_full[v].binormal*=invScale;
        }
    }

    Vector<int32> groups;
    //unlock vertices that have different tangent/binormal but same ref
    for (uint32 i=0, sz=vertices_origin.size(); i<sz; ++i)
    {        
        DVASSERT(vertices_origin[i].refIndices.size()); //vertex with no reference triangles found?

        vertices_origin[i].tangent = vertices_full[vertices_origin[i].refIndices[0]].tangent;
        vertices_origin[i].binormal = vertices_full[vertices_origin[i].refIndices[0]].binormal;

        if (vertices_origin[i].refIndices.size()<=1) //1 and less references do not need unlock test
            continue;
        groups.clear();
        groups.push_back(0);
        vertices_full[vertices_origin[i].refIndices[0]].resultGroup = 0;
        //if has different refs, check different groups;
        for (int32 refId=1, refSz = vertices_origin[i].refIndices.size(); refId<refSz; ++refId)
        {
            VertexWork& vertexRef = vertices_full[vertices_origin[i].refIndices[refId]];
            bool groupFound = false;
            for (int32 groupId = 0, groupSz = groups.size(); groupId<groupSz; ++groupId)
            {
                const VertexWork& groupRef = vertices_full[vertices_origin[i].refIndices[groups[groupId]]];                
                bool groupEqual = FLOAT_EQUAL(vertexRef.tangent.x, groupRef.tangent.x) && FLOAT_EQUAL(vertexRef.tangent.y, groupRef.tangent.y) && FLOAT_EQUAL(vertexRef.tangent.z, groupRef.tangent.z);
                if (computeBinormal)
                    groupEqual &= FLOAT_EQUAL(vertexRef.binormal.x, groupRef.binormal.x) && FLOAT_EQUAL(vertexRef.binormal.y, groupRef.binormal.y) && FLOAT_EQUAL(vertexRef.binormal.z, groupRef.binormal.z);

                if (groupEqual)
                {                    
                    vertexRef.resultGroup = groupId;
                    groupFound = true;
                    break;
                }
            }
            if (!groupFound) //start new group
            {
                vertexRef.resultGroup = groups.size();
                groups.push_back(refId);
            }
        }

        if (groups.size()>1) //different groups found - unlock vertices and update refs
        {            
            groups[0] = i;
            for (int32 groupId = 1, groupSz = groups.size(); groupId<groupSz; ++groupId)
            {
                vertices_origin.push_back(vertices_origin[i]);
                groups[groupId] = vertices_origin.size()-1;
                vertices_origin[groups[groupId]].refIndex = i;
            }
            for (int32 refId=1, refSz = vertices_origin[i].refIndices.size(); refId<refSz; ++refId)
            {
                VertexWork& vertexRef = vertices_full[vertices_origin[i].refIndices[refId]];
                vertexRef.refIndex = groups[vertexRef.resultGroup];
            }
        }
    }
    
    ScopedPtr<PolygonGroup> tmpGroup(new PolygonGroup());        
    tmpGroup->AllocateData(group->GetFormat(), group->GetVertexCount(), group->GetIndexCount());

    Memcpy(tmpGroup->meshData, group->meshData, group->GetVertexCount()*group->vertexStride);
    Memcpy(tmpGroup->indexArray, group->indexArray, group->GetIndexCount()*sizeof(int16));

    int32 vertexFormat = group->GetFormat() | EVF_TANGENT;
    if (computeBinormal)
        vertexFormat|=EVF_BINORMAL;
    group->ReleaseData();
    group->AllocateData(vertexFormat, vertices_origin.size(), vertices_full.size());

    //copy verices
    for (uint32 i=0, sz = vertices_origin.size(); i<sz; ++i)
    {
        CopyVertex(tmpGroup, vertices_origin[i].refIndex, group, i);
        group->SetTangent(i, vertices_origin[i].tangent);
        if (computeBinormal)
            group->SetBinormal(i, vertices_origin[i].binormal);
    }

    //copy indices
    for (int32 i = 0, sz = vertices_full.size(); i<sz; ++i)
        group->SetIndex(i, vertices_full[i].refIndex);

    group->BuildBuffers();
}
    
};






