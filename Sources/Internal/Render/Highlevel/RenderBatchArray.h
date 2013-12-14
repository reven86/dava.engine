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


#ifndef __DAVAENGINE_SCENE3D_RENDERBATCHARRAY_H__
#define	__DAVAENGINE_SCENE3D_RENDERBATCHARRAY_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderBatch.h"

namespace DAVA
{

class RenderLayerBatchArray;
class RenderPassBatchArray
{
public:
    RenderPassBatchArray();
    ~RenderPassBatchArray();
    
    void Clear();
    inline void AddRenderBatch(const FastName & name, RenderBatch * renderBatch);
    RenderLayerBatchArray * Get(const FastName & name);
	void InitLayer(const FastName& layerName, uint32 sortingFlags);

private:
    HashMap<FastName, RenderLayerBatchArray*> layerBatchArrayMap;
};

class RenderLayerBatchArray
{
public:
    RenderLayerBatchArray(uint32 sortingFlags);
    virtual ~RenderLayerBatchArray();
    
    enum
    {
        SORT_ENABLED = 1 << 0,
        SORT_BY_MATERIAL = 1 << 1,
        SORT_BY_DISTANCE = 1 << 2,
        
        SORT_REQUIRED = 1 << 3,
    };
    
    static const uint32 SORT_THIS_FRAME = SORT_ENABLED | SORT_REQUIRED;
    
    void Clear();
    inline void AddRenderBatch(RenderBatch * batch);
    uint32 GetRenderBatchCount();
    RenderBatch * Get(uint32 index);

    inline void ForceLayerSort();
	const FastName & GetName();

    void Sort(Camera * camera);

	void SetVisible(bool visible);
	bool GetVisible();
private:
    Vector<RenderBatch*> renderBatchArray;
    uint32 flags;
    static bool MaterialCompareFunction(const RenderBatch * a, const RenderBatch * b);
public:
    INTROSPECTION(RenderLayerBatchArray,
        COLLECTION(renderBatchArray, "Render Batch Array", I_EDIT)
    );
};
	
	inline void RenderPassBatchArray::AddRenderBatch(const FastName & name, RenderBatch * renderBatch)
	{
		RenderLayerBatchArray * layerBatchArray = layerBatchArrayMap.at(name);
        if (!layerBatchArray)
        {
            layerBatchArray = new RenderLayerBatchArray(RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL);
            layerBatchArrayMap.insert(name, layerBatchArray);
        }
		//DVASSERT(layerBatchArray);
		
		layerBatchArray->AddRenderBatch(renderBatch);
	}
	
	inline void RenderLayerBatchArray::AddRenderBatch(RenderBatch * batch)
	{
		//DVASSERT(batch->GetRemoveIndex() == -1)
		renderBatchArray.push_back(batch);
	}


    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDERBATCHARRAY_H__ */

