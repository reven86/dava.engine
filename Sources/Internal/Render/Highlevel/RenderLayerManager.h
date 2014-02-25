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


#ifndef __DAVAENGINE_RENDER_RENDER_LAYER_MANAGER_H__
#define	__DAVAENGINE_RENDER_RENDER_LAYER_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Render/Highlevel/RenderFastNames.h"

namespace DAVA
{
class RenderPass;
class RenderLayer;
    
class RenderLayerManager : public StaticSingleton<RenderLayerManager>
{
public:
    RenderLayerManager();
    ~RenderLayerManager();
    
    void InsertLayer(RenderLayer * renderLayer);
    inline RenderLayer * GetRenderLayer(uint32 index) const { return array[index]; };
    inline RenderLayer * GetRenderLayer(const FastName & name) const { return map.at(name); };

    static RenderLayerID GetLayerIDByName(const FastName & fastname) { return layerIDmap[fastname]; };
    static uint32 GetLayerIDMaskBySet(const FastNameSet & set);
private:
    void Release();
    
    Vector<RenderLayer*> array;
    HashMap<FastName, RenderLayer*> map;
    static HashMap<FastName, RenderLayerID> layerIDmap;
};

    
} // ns

#endif	/* __DAVAENGINE_RENDER_RENDER_LAYER_MANAGER_H__ */

