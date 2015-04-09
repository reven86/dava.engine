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


#ifndef __DAVAENGINE_RESOURCE_H__
#define __DAVAENGINE_RESOURCE_H__

#include "Base/BaseObject.h"
#include "Platform/Mutex.h"

namespace DAVA
{

/**
	\ingroup render
	\brief 
            Base class for render system resources. All render system resources must be derived from this class.
 
    Thoughts about RenderResource
    RenderResource - can be the one that require invalidation and ones that not require. For example:

    Low-level resources
    - Texture - require invalidation. From DISK.
    - Shader - require invalidation. From DISK, or Memory.
    - VBO, IBO - require invalidation. From somewhere.
    
    High-level resources
    - Sprite - sometimes can require reload, looks like the same as invalidation.
    - All types of meshes - do not require any type of reload. Sometimes should provide data to underlying VBOs, IBOs.
    - Material - do not require invalidation. 
    
 
 */
class RenderResource : public BaseObject
{
protected:
	virtual ~RenderResource();
public:
	RenderResource();
	
	/**
		\brief 	This function is called when you need to save resource to system memory for easy restore later. 
				This function should be overloaded for resouces that do not regenerating every frame, like rendertargets. 
				So if application lost focus framework call this function to save resources to system memory for each restore when device will be lost
	*/
	virtual void SaveToSystemMemory();


	/**
		\brief 	Resource that lost send this message to release old unused data and prepare to be reloaded
				This function must be overloaded in all resources that can be lost and it should free all data used by resource and prepare itself for reload
	*/
	virtual void Lost();
	
	/**
		\brief 	Invalidate resource data. Restore it from original location if needed.
	*/
	virtual void Invalidate();


	static List<RenderResource*> resourceList;
    static Mutex resourceListMutex;
	static void LostAllResources();
	static void InvalidateAllResources();
	static void SaveAllResourcesToSystemMem();
 
    //Debug
	static void LostAllShaders();
	static void InvalidateAllShaders();
	static void LostAllTextures();
	static void InvalidateAllTextures();

	static void LostAllRDO();
	static void InvalidateAllRDO();

};

};

#endif // __DAVAENGINE_RESOURCE_H__