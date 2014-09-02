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



#ifndef __DAVAENGINE_SYSTEM_VISIBILITY_AABBOX__
#define __DAVAENGINE_SYSTEM_VISIBILITY_AABBOX__

#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Frustum.h"

namespace DAVA
{

class VisibilityAABBoxSystem
{
public:
	static void Run(Scene * scene)
	{
		Frustum * frustum = scene->GetClipCamera()->GetFrustum(); 

		TemplatePool<AABBox3> * boxes = scene->entityManager->GetLinkedTemplatePools<AABBox3>("meshAABox");
		while(boxes)
		{
			EntityFamily * family = boxes->GetEntityFamily();
			TemplatePool<uint32> * visibilityFlags = (TemplatePool<uint32>*)family->GetPoolByDataName("flags");

			int32 count = boxes->GetCount();
			AABBox3 * box = boxes->GetPtr(0);
			uint32 * flag = visibilityFlags->GetPtr(0);

			for(int32 i = 0; i < count; ++i)
			{
				if(frustum->IsInside(box))
				{
					*flag |= SceneNode::NODE_CLIPPED_THIS_FRAME;
				}
				else
				{
					*flag &= ~SceneNode::NODE_CLIPPED_THIS_FRAME;
				}

				box++;
				flag++;
			}

			boxes = boxes->next;
		}
	}
};

};

#endif //__DAVAENGINE_SYSTEM_VISIBILITY_AABBOX__