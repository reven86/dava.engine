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


#include "Render/RenderEffect.h"
#include "Render/Texture.h"

namespace DAVA 
{
Map<String, RenderEffect*> RenderEffect::effectsMap;
    
RenderEffect::RenderEffect()
{
    TAG_SWITCH(MemoryManager::TAG_RENDER)
    /* 
    TODO: fix this code
     
    const String & name = GetName();
    Map<String, RenderEffect *>::iterator effectIt = effectsMap.find(name);
    if (effectIt == effectsMap.end())
    {
        //Logger::FrameworkDebug("Register effect: %s", name.c_str());
        effectsMap[name] = this; 
    }else
    {
        //Logger::FrameworkDebug("RenderEffect \"%s\" already registered", name.c_str());
    }*/
}

RenderEffect::~RenderEffect()
{
    TAG_SWITCH(MemoryManager::TAG_RENDER)
    
    //effectsMap.erase(GetName());
}
    
const char * RenderEffect::GetName()
{
    TAG_SWITCH(MemoryManager::TAG_RENDER)
    return "EmptyEffect";
}

void RenderEffect::DrawArrays(ePrimitiveType mode, int32 first, int32 count)
{
    TAG_SWITCH(MemoryManager::TAG_RENDER)
    
    DVASSERT("RenderEffect::DrawArrays not implemented but called" && 0);
}
    
void RenderEffect::DrawElements(ePrimitiveType type, int32 count, eIndexFormat indexFormat, void * indices)
{
    TAG_SWITCH(MemoryManager::TAG_RENDER)
    
    DVASSERT("RenderEffect::DrawElements not implemented but called" && 0);
}

};
