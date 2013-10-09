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


#ifndef __DAVAENGINE_SCENE3D_RENDERLAYER_H__
#define	__DAVAENGINE_SCENE3D_RENDERLAYER_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderBatch.h"

namespace DAVA
{

class RenderBatchArray;
class Camera;
    
class RenderLayer
{
public:
    RenderLayer(const FastName & name);
    virtual ~RenderLayer();
    
    enum
    {
        SORT_ENABLED = 1 << 0,
        SORT_BY_MATERIAL = 1 << 1,
        SORT_BY_DISTANCE = 1 << 2,
        
        SORT_REQUIRED = 1 << 3,

		VISIBLE = 1 << 4
    };
    
    static const uint32 SORT_THIS_FRAME = SORT_ENABLED | SORT_REQUIRED;
    
    void AddRenderBatch(RenderBatch * batch);
    void RemoveRenderBatch(RenderBatch * batch);
    uint32 GetRenderBatchCount();
    inline void ForceLayerSort();
	const FastName & GetName();

    void Update(Camera * camera);
    virtual void Draw(Camera * camera);

	void SetVisible(bool visible);
	bool GetVisible();
private:
    FastName name;
    Vector<RenderBatch*> renderBatchArray;
    //Vector<
    uint32 index;
    uint32 flags;
    //RenderBatchArray * renderBatchArray;
    
    struct RenderBatchSortItem
    {
        pointer_size sortingKey;
        RenderBatch * renderBatch;
    };
    Vector<RenderBatchSortItem> sortArray;
    static bool MaterialCompareFunction(const RenderBatchSortItem & a, const RenderBatchSortItem & b);
public:
    
    INTROSPECTION(RenderLayer,
        MEMBER(name, "Name", I_VIEW)
        COLLECTION(renderBatchArray, "Render Batch Array", I_VIEW | I_EDIT)
    );
};
    
inline void RenderLayer::ForceLayerSort()
{
    flags |= SORT_REQUIRED;
}

    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDERLAYER_H__ */

