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


#ifndef __DAVAENGINE_RENDER_FASTNAMES_H__
#define	__DAVAENGINE_RENDER_FASTNAMES_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"

namespace DAVA
{

using RenderLayerID = uint32;
using RenderPassID = uint32;

// GLOBAL PASSES
static const FastName PASS_FORWARD("ForwardPass");
static const FastName PASS_SHADOW_VOLUME("ShadowVolumePass");
static const FastName PASS_DEFERRED("DeferredPass");
static const FastName PASS_STATIC_OCCLUSION("StaticOcclusionPass");
    
enum eRenderPassID
{
    RENDER_PASS_FORWARD_ID = 0,
    RENDER_PASS_SHADOW_VOLUME_ID,
    RENDER_PASS_SHADOW_MAP_ID,
    RENDER_PASS_DEFERRED_ID,
    RENDER_PASS_STATIC_OCCLUSION_ID,    
    RENDER_PASS_WATER_REFLECTION,
    RENDER_PASS_WATER_REFRACTION,
    RENDER_PASS_ID_COUNT,
};


// GLOBAL LAYERS
static const FastName LAYER_OPAQUE("OpaqueRenderLayer");
static const FastName LAYER_AFTER_OPAQUE("AfterOpaqueRenderLayer");
static const FastName LAYER_ALPHA_TEST_LAYER("AlphaTestLayer");
static const FastName LAYER_WATER("WaterLayer");
static const FastName LAYER_TRANSLUCENT("TransclucentRenderLayer");
static const FastName LAYER_AFTER_TRANSLUCENT("AfterTransclucentRenderLayer");
static const FastName LAYER_SHADOW_VOLUME("ShadowVolumeRenderLayer");
static const FastName LAYER_VEGETATION("VegetationRenderLayer");
static const FastName LAYER_DEBUG_DRAW("DebugRenderLayer");
    
enum eRenderLayerID
{
    RENDER_LAYER_OPAQUE_ID = 0,
    RENDER_LAYER_AFTER_OPAQUE_ID = 1,
    RENDER_LAYER_ALPHA_TEST_LAYER_ID = 2,
    RENDER_LAYER_WATER_ID = 3, 
    RENDER_LAYER_TRANSLUCENT_ID = 4,
    RENDER_LAYER_AFTER_TRANSLUCENT_ID = 5,
    RENDER_LAYER_SHADOW_VOLUME_ID = 6,
    RENDER_LAYER_VEGETATION_ID = 7,
    RENDER_LAYER_DEBUG_DRAW_ID = 8,
    RENDER_LAYER_ID_COUNT,
};
    
#define RENDER_LAYER_ID_BITMASK_MIN_POS 16
#define RENDER_LAYER_ID_BITMASK_MIN_MASK 0xF
#define RENDER_LAYER_ID_BITMASK_MAX_POS 20
#define RENDER_LAYER_ID_BITMASK_MAX_MASK 0xF
    

    
static const FastName INHERIT_FROM_MATERIAL("Inherit from material");
static const FastName LAST_LAYER("Last layer");

} // ns

#endif	/* __DAVAENGINE_RENDER_FASTNAMES_H__ */

