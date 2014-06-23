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

#include "MeshUtils.h"

namespace DAVA
{

namespace MeshUtils
{

void CopyVertex(PolygonGroup *srcGroup, uint32 srcPos, PolygonGroup *dstGroup, uint32 dstPos)
{
    int32 srcFormat = srcGroup->GetFormat();
    int32 dstFormat = dstGroup->GetFormat();
    int32 copyFormat = srcFormat&dstFormat;    //most common format;   

    uint8 *srcData = srcGroup->meshData+srcPos*GetVertexSize(srcFormat);
    uint8 *dstData = dstGroup->meshData+dstPos*GetVertexSize(dstFormat);
        
    for (uint32 mask = EVF_LOWER_BIT; mask <= EVF_HIGHER_BIT; mask = mask << 1)
    {
        int32 vertexAttribSize = GetVertexSize(mask);
        if (mask&copyFormat)
            Memcpy(dstData, srcData, vertexAttribSize);

        if (mask&srcFormat)
            srcData+=vertexAttribSize;
        
        if (mask&dstFormat)
            dstData+=vertexAttribSize;

         copyFormat&=~mask;
    }    
    
    /*unsupported stream*/
    DVASSERT((copyFormat == 0)&&"Unsupported attribute stream in copy");
    
}

void CopyGroupData(PolygonGroup *srcGroup, PolygonGroup *dstGroup)
{
    dstGroup->ReleaseData();
    dstGroup->AllocateData(srcGroup->GetFormat(), srcGroup->GetVertexCount(), srcGroup->GetIndexCount());

    Memcpy(dstGroup->meshData, srcGroup->meshData, srcGroup->GetVertexCount()*srcGroup->vertexStride);
    Memcpy(dstGroup->indexArray, srcGroup->indexArray, srcGroup->GetIndexCount()*sizeof(int16));

    dstGroup->BuildBuffers();
}

void RebuildMeshTangentSpace(PolygonGroup *group, bool precomputeBinormal/*=true*/)
{
    DVASSERT(group->GetPrimitiveType() == PRIMITIVETYPE_TRIANGLELIST); //only triangle lists for now    
    DVASSERT(group->GetFormat()&EVF_TEXCOORD0);
    DVASSERT(group->GetFormat()&EVF_NORMAL);

    Vector<FaceWork> faces;
    uint32 faceCount = group->GetIndexCount()/3;
    faces.resize(faceCount);
    Vector<VertexWork> verticesOrigin;
    Vector<VertexWork> verticesFull;
    verticesOrigin.resize(group->GetVertexCount());
    verticesFull.resize(group->GetIndexCount());

    for (uint32 i=0, sz = group->GetVertexCount(); i<sz; ++i)
        verticesOrigin[i].refIndex = i;
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
            
            verticesOrigin[originIndex].refIndices.push_back(workIndex);
            verticesFull[f*3+i].refIndex = faces[f].indexOrigin[i];
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

         //should we normalize it here or only final result?        
        tangent.Normalize();
        binormal.Normalize();
        
        faces[f].tangent = tangent;
        faces[f].binormal = binormal;
        for (int32 i=0; i<3; ++i)
        {
            verticesFull[f*3+i].tangent = tangent;            
            verticesFull[f*3+i].binormal = binormal;
        }
    }

    /*smooth tangent space preventing mirrored uv's smooth*/
    for (uint32 v = 0, sz = verticesFull.size(); v<sz; ++v)
    {
        int32 faceId = v/3;
        VertexWork& originVert = verticesOrigin[verticesFull[v].refIndex];      
        verticesFull[v].tbRatio = 1;
        for (int32 iRef=0, refSz = originVert.refIndices.size(); iRef<refSz; ++iRef)
        {
            int32 refFaceId = originVert.refIndices[iRef]/3;
            if (refFaceId == faceId) continue;
            
            //check if uv's mirrored;            
            
            //here we use handness to find mirrored UV's - still not sure if it is better then using dot product (upd: experiments show it is really better)
            Vector3 n1 = CrossProduct(verticesFull[v].tangent, verticesFull[v].binormal);
            Vector3 n2 = CrossProduct(faces[refFaceId].tangent, faces[refFaceId].binormal);                        
            
            if (DotProduct(n1, n2)>0.0f)
            {
                verticesFull[v].tangent+=faces[refFaceId].tangent;
                verticesFull[v].binormal+=faces[refFaceId].binormal;
                verticesFull[v].tbRatio++;
            }
            
        }

        //as we use normalized tangent space - we renormalize vertex TB instead of rescaling it - think later if it is ok
        verticesFull[v].tangent.Normalize();
        verticesFull[v].binormal.Normalize();                
        
        /*float32 invScale = 1.0f/(float32)vertices_full[v].tbRatio;
        vertices_full[v].tangent*=invScale;
        vertices_full[v].binormal*=invScale;*/
        
    }


    const float32 EPS = 0.00001f; //should be the same value as in exporter
    Vector<int32> groups;
    //unlock vertices that have different tangent/binormal but same ref
    for (uint32 i=0, sz=verticesOrigin.size(); i<sz; ++i)
    {        
        DVASSERT(verticesOrigin[i].refIndices.size()); //vertex with no reference triangles found?

        verticesOrigin[i].tangent = verticesFull[verticesOrigin[i].refIndices[0]].tangent;
        verticesOrigin[i].binormal = verticesFull[verticesOrigin[i].refIndices[0]].binormal;

        if (verticesOrigin[i].refIndices.size()<=1) //1 and less references do not need unlock test
            continue;
        groups.clear();
        groups.push_back(0);
        verticesFull[verticesOrigin[i].refIndices[0]].resultGroup = 0;
        //if has different refs, check different groups;
        for (int32 refId=1, refSz = verticesOrigin[i].refIndices.size(); refId<refSz; ++refId)
        {
            VertexWork& vertexRef = verticesFull[verticesOrigin[i].refIndices[refId]];
            bool groupFound = false;
            for (int32 groupId = 0, groupSz = groups.size(); groupId<groupSz; ++groupId)
            {
                const VertexWork& groupRef = verticesFull[verticesOrigin[i].refIndices[groups[groupId]]];                
                bool groupEqual = FLOAT_EQUAL_EPS(vertexRef.tangent.x, groupRef.tangent.x, EPS) && FLOAT_EQUAL_EPS(vertexRef.tangent.y, groupRef.tangent.y, EPS) && FLOAT_EQUAL_EPS(vertexRef.tangent.z, groupRef.tangent.z, EPS);
                if (precomputeBinormal)
                    groupEqual &= FLOAT_EQUAL_EPS(vertexRef.binormal.x, groupRef.binormal.x, EPS) && FLOAT_EQUAL_EPS(vertexRef.binormal.y, groupRef.binormal.y, EPS) && FLOAT_EQUAL_EPS(vertexRef.binormal.z, groupRef.binormal.z, EPS);

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
                verticesOrigin.push_back(verticesOrigin[i]);
                groups[groupId] = verticesOrigin.size()-1;
                verticesOrigin[groups[groupId]].refIndex = i;
            }
            for (int32 refId=1, refSz = verticesOrigin[i].refIndices.size(); refId<refSz; ++refId)
            {
                VertexWork& vertexRef = verticesFull[verticesOrigin[i].refIndices[refId]];
                vertexRef.refIndex = groups[vertexRef.resultGroup];
            }
        }
    }
    
    //copy original polygon group data and fill new tangent/binormal values
    ScopedPtr<PolygonGroup> tmpGroup(new PolygonGroup());        
    tmpGroup->AllocateData(group->GetFormat(), group->GetVertexCount(), group->GetIndexCount());

    Memcpy(tmpGroup->meshData, group->meshData, group->GetVertexCount()*group->vertexStride);
    Memcpy(tmpGroup->indexArray, group->indexArray, group->GetIndexCount()*sizeof(int16));

    int32 vertexFormat = group->GetFormat() | EVF_TANGENT;
    if (precomputeBinormal)
        vertexFormat|=EVF_BINORMAL;
    group->ReleaseData();
    group->AllocateData(vertexFormat, verticesOrigin.size(), verticesFull.size());

    //copy vertices
    for (uint32 i=0, sz = verticesOrigin.size(); i<sz; ++i)
    {
        CopyVertex(tmpGroup, verticesOrigin[i].refIndex, group, i);   
        Vector3 normal;
        group->GetNormal(i, normal);
        
        Vector3 tangent = verticesOrigin[i].tangent;
        tangent -=normal*DotProduct(tangent, normal);        
        tangent.Normalize();
        group->SetTangent(i, tangent);
        if (precomputeBinormal)
        {
            Vector3 binormal = -verticesOrigin[i].binormal;
            binormal -=normal*DotProduct(binormal, normal);            
            binormal.Normalize();
            group->SetBinormal(i, binormal);
        }
    }

    //copy indices
    for (int32 i = 0, sz = verticesFull.size(); i<sz; ++i)
        group->SetIndex(i, verticesFull[i].refIndex);

    group->BuildBuffers();
}

};
};

