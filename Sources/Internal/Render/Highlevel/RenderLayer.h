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
#include "Render/Highlevel/RenderBatchArray.h"

namespace DAVA
{

class Camera;
class OcclusionQuery;
class RenderLayer
{
public:
    enum eRenderLayerID
    {
        RENDER_LAYER_INVALID_ID = -1,
        RENDER_LAYER_OPAQUE_ID = 0,
        RENDER_LAYER_AFTER_OPAQUE_ID = 1,
        RENDER_LAYER_ALPHA_TEST_LAYER_ID = 2,
        RENDER_LAYER_WATER_ID = 3,
        RENDER_LAYER_TRANSLUCENT_ID = 4,
        RENDER_LAYER_AFTER_TRANSLUCENT_ID = 5,
        RENDER_LAYER_SHADOW_VOLUME_ID = 6,
        RENDER_LAYER_VEGETATION_ID = 7,
        RENDER_LAYER_DEBUG_DRAW_ID = 8,
        RENDER_LAYER_ID_COUNT
    };

    static eRenderLayerID GetLayerIDByName(const FastName & name);
    static const FastName & GetLayerNameByID(eRenderLayerID layer);

    // LAYERS SORTING FLAGS
    static const uint32 LAYER_SORTING_FLAGS_OPAQUE;
    static const uint32 LAYER_SORTING_FLAGS_AFTER_OPAQUE;
    static const uint32 LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER;
    static const uint32 LAYER_SORTING_FLAGS_WATER;
    static const uint32 LAYER_SORTING_FLAGS_TRANSLUCENT;
    static const uint32 LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT;
    static const uint32 LAYER_SORTING_FLAGS_SHADOW_VOLUME;
    static const uint32 LAYER_SORTING_FLAGS_VEGETATION;
    static const uint32 LAYER_SORTING_FLAGS_DEBUG_DRAW;

    RenderLayer(eRenderLayerID id, uint32 sortingFlags);
    virtual ~RenderLayer();
    
    inline eRenderLayerID GetRenderLayerID() const;
	inline uint32 GetSortingFlags() const;

    virtual void Draw(Camera* camera, const RenderBatchArray & batchArray, rhi::HPacketList packetList);
    
protected:
    eRenderLayerID layerID;
    uint32 sortFlags;
    
public:
    INTROSPECTION(RenderLayer, NULL
        //COLLECTION(renderBatchArray, "Render Batch Array", I_VIEW)
    );
};
    
inline RenderLayer::eRenderLayerID RenderLayer::GetRenderLayerID() const
{
    return layerID;
}

inline uint32 RenderLayer::GetSortingFlags() const
{
    return sortFlags;
}

} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDERLAYER_H__ */

