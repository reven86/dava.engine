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
#include "Render/RenderCallbacks.h"
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
    , primitiveType(rhi::PRIMITIVE_TRIANGLELIST)
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
    , vertexLayoutId(rhi::VertexLayout::InvalidUID)    
{
    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &PolygonGroup::RestoreBuffers));
}

PolygonGroup::~PolygonGroup()
{   
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &PolygonGroup::RestoreBuffers));
	ReleaseData();
    if (vertexBuffer.IsValid())
        rhi::DeleteVertexBuffer(vertexBuffer);    
    if (indexBuffer.IsValid())
        rhi::DeleteIndexBuffer(indexBuffer);
}
    
void PolygonGroup::UpdateDataPointersAndStreams()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    //later merge old render and new rhi formats
    rhi::VertexLayout vLayout;

    int32 baseShift = 0;
    if (vertexFormat & EVF_VERTEX)
    {
        vertexArray = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_VERTEX);
        vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_NORMAL)
    {
        normalArray = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_NORMAL);
        vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_COLOR)
    {
        colorArray = reinterpret_cast<uint32*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_COLOR);
        vLayout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
    }
    if (vertexFormat & EVF_TEXCOORD0)
    {
        textureCoordArray[0] = reinterpret_cast<Vector2*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_TEXCOORD0);
        vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    }
    if (vertexFormat & EVF_TEXCOORD1)
    {
        textureCoordArray[1] = reinterpret_cast<Vector2*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_TEXCOORD1);
        vLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 2);
    }
    if (vertexFormat & EVF_TEXCOORD2)
    {
        textureCoordArray[2] = reinterpret_cast<Vector2*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_TEXCOORD2);
        vLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 2);
    }
    if (vertexFormat & EVF_TEXCOORD3)
    {
        textureCoordArray[3] = reinterpret_cast<Vector2*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_TEXCOORD3);
        vLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 2);
    }
    if (vertexFormat & EVF_TANGENT)
    {
        tangentArray = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_TANGENT);
        vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_BINORMAL)
    {
        binormalArray = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_BINORMAL);
        vLayout.AddElement(rhi::VS_BINORMAL, 0, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_JOINTINDEX)
    {
        jointIdxArray = reinterpret_cast<float32*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_JOINTINDEX);
        vLayout.AddElement(rhi::VS_BLENDINDEX, 0, rhi::VDT_FLOAT, 1);

        SafeDeleteArray(jointCountArray);
        jointCountArray = new uint32[vertexCount];
    }
    if (vertexFormat & EVF_JOINTWEIGHT)
    {
        jointWeightArray = reinterpret_cast<float32*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_JOINTWEIGHT);
        vLayout.AddElement(rhi::VS_BLENDWEIGHT, 0, rhi::VDT_FLOAT, 1);
    }
    if (vertexFormat & EVF_CUBETEXCOORD0)
    {
        cubeTextureCoordArray[0] = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_CUBETEXCOORD0);
        vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_CUBETEXCOORD1)
    {
        cubeTextureCoordArray[1] = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_CUBETEXCOORD1);
        vLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_CUBETEXCOORD2)
    {
        cubeTextureCoordArray[2] = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_CUBETEXCOORD2);
        vLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_CUBETEXCOORD3)
    {
        cubeTextureCoordArray[3] = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_CUBETEXCOORD3);
        vLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_PIVOT)
    {
        pivotArray = reinterpret_cast<Vector3*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_PIVOT);
        vLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 3);
    }
    if (vertexFormat & EVF_FLEXIBILITY)
    {
        flexArray = reinterpret_cast<float32*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_FLEXIBILITY);
        vLayout.AddElement(rhi::VS_TEXCOORD, 5, rhi::VDT_FLOAT, 1);
    }
    if (vertexFormat & EVF_ANGLE_SIN_COS)
    {
        angleArray = reinterpret_cast<Vector2*>(meshData + baseShift);
        baseShift += GetVertexSize(EVF_ANGLE_SIN_COS);
        vLayout.AddElement(rhi::VS_TEXCOORD, 4, rhi::VDT_FLOAT, 2);
    }

    vertexLayoutId = rhi::VertexLayout::UniqueId(vLayout);
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
    
    UpdateDataPointersAndStreams();
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
    if (vertexBuffer.IsValid())
        rhi::DeleteVertexBuffer(vertexBuffer);    

    uint32 vertexDataSize = vertexStride * vertexCount;
    vertexBuffer = rhi::CreateVertexBuffer(vertexDataSize);
    rhi::UpdateVertexBuffer(vertexBuffer, meshData, 0, vertexDataSize);

    if (indexBuffer.IsValid())
        rhi::DeleteIndexBuffer(indexBuffer);
    uint32 indexDataSize = indexCount * INDEX_FORMAT_SIZE[indexFormat];
    indexBuffer = rhi::CreateIndexBuffer(indexDataSize);
    rhi::UpdateIndexBuffer(indexBuffer, indexArray, 0, indexDataSize);    
};

void PolygonGroup::RestoreBuffers()
{
    if (vertexBuffer.IsValid() && rhi::NeedRestoreVertexBuffer(vertexBuffer))
    {
        uint32 vertexDataSize = vertexStride * vertexCount;
        rhi::UpdateVertexBuffer(vertexBuffer, meshData, 0, vertexDataSize);
    }
    if (indexBuffer.IsValid() && rhi::NeedRestoreIndexBuffer(indexBuffer))
    {
        uint32 indexDataSize = indexCount * INDEX_FORMAT_SIZE[indexFormat];        
        rhi::UpdateIndexBuffer(indexBuffer, indexArray, 0, indexDataSize);
    }


}


    
void PolygonGroup::Save(KeyedArchive * keyedArchive, SerializationContext * serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DataNode::Save(keyedArchive, serializationContext);
    
    keyedArchive->SetInt32("vertexFormat", vertexFormat);
    keyedArchive->SetInt32("vertexCount", vertexCount); 
    keyedArchive->SetInt32("indexCount", indexCount); 
    keyedArchive->SetInt32("textureCoordCount", textureCoordCount);
    keyedArchive->SetInt32("rhi_primitiveType", primitiveType);
                           
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

void PolygonGroup::LoadPolygonData(KeyedArchive * keyedArchive, SerializationContext * serializationContext, int32 requiredFlags, bool cutUnusedStreams)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    vertexFormat = keyedArchive->GetInt32("vertexFormat");
    vertexStride = GetVertexSize(vertexFormat);
    vertexCount = keyedArchive->GetInt32("vertexCount");
    indexCount = keyedArchive->GetInt32("indexCount");
    textureCoordCount = keyedArchive->GetInt32("textureCoordCount");
    primitiveType = (rhi::PrimitiveType)keyedArchive->GetInt32("rhi_primitiveType", rhi::PRIMITIVE_TRIANGLELIST);
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

        int32 resFormat = cutUnusedStreams ? requiredFlags : (vertexFormat | requiredFlags);
        DVASSERT(resFormat);
        if (vertexFormat != resFormat) //not all streams in data are required or present - smart copy
        {
            if ((~vertexFormat)&resFormat)
                Logger::Debug("expanding polygon group vertex format for %d vertices!", vertexCount);                       
            int32 newVertexStride = GetVertexSize(resFormat);
            SafeDeleteArray(meshData);
            meshData = new uint8[vertexCount * newVertexStride];
            uint8 *dst = meshData;
            const uint8 *src = &archiveData[0];
            for (int32 i = 0; i < vertexCount; ++i)
            {
                for (uint32 mask = EVF_LOWER_BIT; mask <= EVF_HIGHER_BIT; mask = mask << 1)
                {
                    CopyData(&src, &dst, vertexFormat, resFormat, mask);
                }
            }
            vertexFormat = resFormat;
            vertexStride = newVertexStride;
            textureCoordCount = GetTexCoordCount(vertexFormat);
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


void PolygonGroup::CopyData(const uint8 ** meshData, uint8 ** newMeshData, uint32 vertexFormat, uint32 newVertexFormat, uint32 format) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 formatSize = GetVertexSize(format);
	if (vertexFormat & format)
	{		
		if (newVertexFormat & format)
		{
			memcpy(*newMeshData, *meshData, formatSize);		
		}
		*meshData += formatSize;
	}
    if (newVertexFormat & format)
    {
        *newMeshData += formatSize;
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
