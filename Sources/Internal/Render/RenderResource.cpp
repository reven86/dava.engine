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


#include "Render/RenderResource.h"
#include "Render/Texture.h"
#include "Render/Shader.h"
#include "Render/RenderDataObject.h"

namespace DAVA
{

List<RenderResource*> RenderResource::resourceList;
Mutex RenderResource::resourceListMutex;

RenderResource::RenderResource()
{
    resourceListMutex.Lock();
	resourceList.push_back(this);
    resourceListMutex.Unlock();
}

RenderResource::~RenderResource()
{
    resourceListMutex.Lock();
	List<RenderResource*>::iterator it = resourceList.begin();
	for(; it != resourceList.end(); it++)
	{
		if(*it == this)
		{
			resourceList.erase(it);
			break;
		}
	}
    resourceListMutex.Unlock();
}

void RenderResource::SaveToSystemMemory()
{

}

void RenderResource::Lost()
{
}

void RenderResource::Invalidate()
{
}

void RenderResource::LostAllResources()
{
    Logger::FrameworkDebug("[RenderResource::LostAllResources]");
    
    resourceListMutex.Lock();
	List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
	for(; it != itEnd; ++it)
	{
		(*it)->Lost();
	}
    resourceListMutex.Unlock();
}

void RenderResource::InvalidateAllResources()
{
    Logger::FrameworkDebug("[RenderResource::InvalidateAllResources]");

    resourceListMutex.Lock();
	List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
	for(; it != itEnd; ++it)
	{
		(*it)->Invalidate();
	}
    resourceListMutex.Unlock();
}

void RenderResource::SaveAllResourcesToSystemMem()
{
#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)  ||  defined(__DAVAENGINE_DIRECTX9__)
    resourceListMutex.Lock();
	List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
	for(; it != itEnd; ++it)
	{
		(*it)->SaveToSystemMemory();
// 		Texture * t = dynamic_cast<Texture*>((*it));
// 		if (t)
// 			Logger::FrameworkDebug("%s", t->relativePathname.c_str());
	}
    resourceListMutex.Unlock();
#endif
}
    
void RenderResource::LostAllShaders()
{
#if RHI_COMPLETE
    resourceListMutex.Lock();
    List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
    for(; it != itEnd; ++it)
    {
        Shader *s = dynamic_cast<Shader *>(*it);
        if(s)
        {
            s->Lost();
        }
    }
    resourceListMutex.Unlock();
#endif //RHI_COMPLETE
}

void RenderResource::InvalidateAllShaders()
{
#if RHI_COMPLETE
    resourceListMutex.Lock();
    List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
    for(; it != itEnd; ++it)
    {
        Shader *s = dynamic_cast<Shader *>(*it);
        if(s)
        {
            s->Invalidate();
        }
    }
    resourceListMutex.Unlock();
#endif RHI_COMPLETE
}
    
void RenderResource::LostAllTextures()
{
    resourceListMutex.Lock();
    List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
    for(; it != itEnd; ++it)
    {
        Texture *s = dynamic_cast<Texture *>(*it);
        if(s)
        {
            s->Lost();
        }
    }
    resourceListMutex.Unlock();
}

void RenderResource::InvalidateAllTextures()
{
    resourceListMutex.Lock();
    List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
    for(; it != itEnd; ++it)
    {
        Texture *s = dynamic_cast<Texture *>(*it);
        if(s)
        {
            s->Invalidate();
        }
    }
    resourceListMutex.Unlock();
}


void RenderResource::LostAllRDO()
{
    resourceListMutex.Lock();
    List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
    for(; it != itEnd; ++it)
    {
        RenderDataObject *s = dynamic_cast<RenderDataObject *>(*it);
        if(s)
        {
            s->Lost();
        }
    }
    resourceListMutex.Unlock();
}

void RenderResource::InvalidateAllRDO()
{
    resourceListMutex.Lock();
    List<RenderResource*>::iterator it = resourceList.begin();
    List<RenderResource*>::const_iterator itEnd = resourceList.end();
    for(; it != itEnd; ++it)
    {
        RenderDataObject *s = dynamic_cast<RenderDataObject *>(*it);
        if(s)
        {
            s->Invalidate();
        }
    }
    resourceListMutex.Unlock();
}
    
};
