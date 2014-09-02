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


#ifndef __DAVAENGINE_RENDER_RENDER_PASS_MANAGER_H__
#define	__DAVAENGINE_RENDER_RENDER_PASS_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"

namespace DAVA
{
class RenderPass;
class RenderLayer;
class RenderSystem;
    
class RenderPassManager
{
public:
    RenderPassManager(RenderSystem * renderSystem);
    ~RenderPassManager();
    
    void InsertPass(RenderPass * renderPass);
    inline RenderPass * GetRenderPass(uint32 index) const;
    inline RenderPass * GetRenderPass(const FastName & name) const;
    
private:
    void Release();
    
    Vector<RenderPass*> array;
    HashMap<FastName, RenderPass*> map;
};
    
// Implementation
inline RenderPass * RenderPassManager::GetRenderPass(uint32 index) const { return array[index]; };
inline RenderPass * RenderPassManager::GetRenderPass(const FastName & name) const { return map.at(name); };

    
} // ns

#endif	/* __DAVAENGINE_RENDER_RENDER_PASS_MANAGER_H__ */

