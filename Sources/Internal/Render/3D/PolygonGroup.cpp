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
    : DataNode()
    , vertexCount(0)
    , indexCount(0)
    , textureCoordCount(0)
    , vertexStride(0)
    , vertexFormat(0)
    , indexFormat(EIF_16)
    , triangleCount(0)
    , primitiveType(PRIMITIVETYPE_TRIANGLELIST)
    , cubeTextureCoordCount(0)

    , vertexArray(0)
    , textureCoordArray(0)
    , normalArray(0)
    , tangentArray(0)
    , binormalArray(0)
    , jointIdxArray(0)
    , jointWeightArray(0)
    , cubeTextureCoordArray(0)
    , jointCountArray(0)

    , pivotArray(0)
    , flexArray(0)
    , angleArray(0)

    , colorArray(0)
    , indexArray(0)
    , meshData(0)

    , baseVertexArray(0)
    , renderDataObject(0)
{
}

PolygonGroup::~PolygonGroup()
{
    ReleaseData();
}
    
void PolygonGroup::UpdateDataPointersAndStreams()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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
    if (vertexFormat & EVF_JOINTINDEX)
    {
        jointIdxArray = reinterpret_cast<uint32*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_JOINTINDEX);

        renderDataObject->SetStream(EVF_JOINTINDEX , TYPE_UNSIGNED_BYTE, 4, vertexStride, jointIdxArray);

        SafeDeleteArray(jointCountArray);
        jointCountArray = new int32[vertexCount];
    }
	if (vertexFormat & EVF_JOINTWEIGHT)
	{
		jointWeightArray = reinterpret_cast<uint32*>(meshData + baseShift);
		baseShift += GetVertexSize(EVF_JOINTWEIGHT);
		
        renderDataObject->SetStream(EVF_JOINTWEIGHT , TYPE_UNSIGNED_BYTE, 4, vertexStride, jointWeightArray);
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
    if (vertexFormat & EVF_PIVOT)
    {
        pivotArray = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_PIVOT);

        renderDataObject->SetStream(EVF_PIVOT, TYPE_FLOAT, 3, vertexStride, pivotArray);
    }
    if (vertexFormat & EVF_FLEXIBILITY)
    {
        flexArray = reinterpret_cast<float32*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_FLEXIBILITY);

        renderDataObject->SetStream(EVF_FLEXIBILITY, TYPE_FLOAT, 1, vertexStride, flexArray);
    }
    if (vertexFormat & EVF_ANGLE_SIN_COS)
    {
        angleArray = reinterpret_cast<Vector2*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_ANGLE_SIN_COS);

        renderDataObject->SetStream(EVF_ANGLE_SIN_COS, TYPE_FLOAT, 2, vertexStride, angleArray);
    }
}

void PolygonGroup::AllocateData(int32 _meshFormat, int32 _vertexCount, int32 _indexCount)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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

    renderDataObject = new RenderDataObject;

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeDeleteArray(baseVertexArray);
    baseVertexArray = new Vector3[vertexCount];

    for (int v = 0; v < vertexCount; ++v)
    {
        GetCoord(v, baseVertexArray[v]);
    }
}
    
void PolygonGroup::ApplyMatrix(const Matrix4 & matrix)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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

uint32 PolygonGroup::ReleaseGeometryData()
{
    if (meshData)
    {
        SafeDeleteArray(jointCountArray);
        SafeDeleteArray(meshData);
        SafeDeleteArray(indexArray);
        SafeDeleteArray(textureCoordArray);
        SafeDeleteArray(cubeTextureCoordArray);

        vertexArray = nullptr;
        textureCoordArray = nullptr;
        normalArray = nullptr;
        tangentArray = nullptr;
        binormalArray = nullptr;
        jointIdxArray = nullptr;
        jointWeightArray = nullptr;
        cubeTextureCoordArray = nullptr;
        jointCountArray = nullptr;
	
        pivotArray = nullptr;
        flexArray = nullptr;
        angleArray = nullptr;

        colorArray = nullptr;
        indexArray = nullptr;
        meshData = nullptr;

        uint32 ret = vertexStride * vertexCount; //released vertex bytes
        ret += indexCount * INDEX_FORMAT_SIZE[indexFormat]; //released index bytes

        return ret;
    }

    return 0;
}

void PolygonGroup::BuildBuffers()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    UpdateDataPointersAndStreams();
    JobManager::Instance()->CreateMainJob(MakeFunction(MakeSharedObject(this), &PolygonGroup::BuildBuffersInternal));
};
    
void PolygonGroup::BuildBuffersInternal()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(Thread::IsMainThread());
    
    renderDataObject->BuildVertexBuffer(vertexCount);
    renderDataObject->SetIndices((eIndexFormat)indexFormat, (uint8*)indexArray, indexCount);
    renderDataObject->BuildIndexBuffer();
}


    
void PolygonGroup::Save(KeyedArchive * keyedArchive, SerializationContext * serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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

void PolygonGroup::LoadPolygonData(KeyedArchive * keyedArchive, SerializationContext * serializationContext, int32 requiredFlags)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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

        const uint8 * archiveData = keyedArchive->GetByteArray("vertices");

        if (vertexFormat&~requiredFlags) //not all streams in data are required - smart copy
        {
            int32 newFormat = vertexFormat&requiredFlags;
            DVASSERT(newFormat);
            int32 newVertexStride = GetVertexSize(newFormat);
            SafeDeleteArray(meshData);
            meshData = new uint8[vertexCount * newVertexStride];
            uint8 *dst = meshData;
            const uint8 *src = &archiveData[0];
            for (int32 i = 0; i < vertexCount; ++i)
            {
                for (uint32 mask = EVF_LOWER_BIT; mask <= EVF_HIGHER_BIT; mask = mask << 1)
                {
                    CopyData(&src, &dst, vertexFormat, newFormat, mask);
                }
            }
            vertexFormat = newFormat;
            vertexStride = newVertexStride;

        }
        else
        {
            SafeDeleteArray(meshData);
            meshData = new uint8[vertexCount * vertexStride];
            Memcpy(meshData, archiveData, size); //all streams in data required - just copy
        }
        
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
    renderDataObject = new RenderDataObject;
    UpdateDataPointersAndStreams();
    RecalcAABBox();

    BuildBuffers();
}
    
void PolygonGroup::RecalcAABBox()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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

void PolygonGroup::CopyData(const uint8 ** meshData, uint8 ** newMeshData, uint32 vertexFormat, uint32 newVertexFormat, uint32 format) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 optimizedVertexCount = static_cast<uint32>(optMeshData.size() / GetVertexSize(vertexFormat));

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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    int32 newVertexStride = GetVertexSize(newVertexFormat);
    uint8 * newMeshData = new uint8[newVertexStride * vertexCount];
    memset(newMeshData, 0, sizeof(newVertexStride * vertexCount));

	const uint8 * tmpMesh = meshData;
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

	uint32 newVertexCount = static_cast<uint32>(optMeshData.size() / GetVertexSize(newVertexFormat));
	
	SAFE_DELETE_ARRAY(meshData);
	SAFE_DELETE_ARRAY(newMeshData);
	vertexCount = newVertexCount;
	vertexStride = newVertexStride;
	vertexFormat = newVertexFormat;

    meshData = new uint8[vertexStride * vertexCount];
    memcpy(meshData, &optMeshData[0], vertexStride * vertexCount);
};

};
