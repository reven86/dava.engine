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
class RenderBatchArray
{
public:
    enum
    {
        SORT_ENABLED = 1 << 0,
        SORT_BY_MATERIAL = 1 << 1,
        SORT_BY_DISTANCE_BACK_TO_FRONT = 1 << 2,
        SORT_BY_DISTANCE_FRONT_TO_BACK = 1 << 3,

        SORT_REQUIRED = 1 << 4,
    };

    static const uint32 SORT_THIS_FRAME = SORT_ENABLED | SORT_REQUIRED;

    RenderBatchArray();

    inline void Clear();
    inline void AddRenderBatch(RenderBatch * batch);
    inline uint32 GetRenderBatchCount() const;
    inline RenderBatch* Get(uint32 index) const;

    void Sort(Camera * camera);
    inline void SetSortingFlags(uint32 flags);

private:
    Vector<RenderBatch*> renderBatchArray;
    uint32 sortFlags;
    static bool MaterialCompareFunction(const RenderBatch * a, const RenderBatch * b);
public:
    INTROSPECTION(RenderBatchArray,
                  COLLECTION(renderBatchArray, "Render Batch Array", I_EDIT));
};

inline void RenderBatchArray::Clear()
{
    renderBatchArray.clear();
}

inline void RenderBatchArray::AddRenderBatch(RenderBatch* batch)
{
    renderBatchArray.push_back(batch);
}

inline void RenderBatchArray::SetSortingFlags(uint32 _flags)
{
    sortFlags = _flags;
}

inline uint32 RenderBatchArray::GetRenderBatchCount() const
{
    return (uint32)renderBatchArray.size();
}

inline RenderBatch* RenderBatchArray::Get(uint32 index) const
{
    return renderBatchArray[index];
}

} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDERBATCHARRAY_H__ */

