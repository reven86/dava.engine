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



#ifndef __DAVAENGINE_RENDER_HIGHLEVEL_SHADOW_VOLUME_H__
#define __DAVAENGINE_RENDER_HIGHLEVEL_SHADOW_VOLUME_H__

#include "Render/3D/PolygonGroup.h"
#include "Render/Shader.h"
#include "Render/3D/EdgeAdjacency.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{

class PolygonGroup;
class Light;
    
class ShadowVolume : public RenderBatch
{
protected:
	virtual ~ShadowVolume();

public:
	ShadowVolume();
	static const FastName MATERIAL_NAME;

    //virtual void Draw(Camera * camera);
	virtual void Draw(const FastName & ownerRenderPass, Camera * camera);

	void MakeShadowVolumeFromPolygonGroup(PolygonGroup * polygonGroup);
    void SetPolygonGroup(PolygonGroup * polygonGroup);
    PolygonGroup * GetPolygonGroup();
    
	virtual void GetDataNodes(Set<DataNode*> & dataNodes);
	virtual RenderBatch * Clone(RenderBatch * dstNode = NULL);
	virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);
	virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);

	virtual void UpdateAABBoxFromSource();

	virtual ShadowVolume * CreateShadow();


private:
	struct EdgeMapping
	{
		int32 oldEdge[2];
		int32 newEdge[2][2];

    public:
		EdgeMapping()
		{
			Memset(oldEdge, -1, sizeof(oldEdge));
			Memset(newEdge, -1, sizeof(newEdge));
		}
	};

	int32 FindEdgeInMappingTable(int32 nV1, int32 nV2, EdgeMapping* mapping, int32 count);
    
public:
    INTROSPECTION_EXTEND(ShadowVolume, RenderBatch,
                         0);
};

}

#endif //__DAVAENGINE_RENDER_HIGHLEVEL_SHADOW_VOLUME_H__
