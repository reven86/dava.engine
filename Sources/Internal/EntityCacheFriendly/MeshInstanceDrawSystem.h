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



#ifndef __DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__
#define __DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__

#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Utils/StringFormat.h"
#include "Render/Material.h"

namespace DAVA
{

class MeshInstanceDrawSystem
{
public:

	static void Draw(MeshInstanceNode * meshInstance)
	{
		uint32 meshesSize = (uint32)meshInstance->polygroups.size();

		for (uint32 k = 0; k < meshesSize; ++k)
		{
			PolygonGroupWithMaterial * polygroup = meshInstance->polygroups[k];
			Material * material = polygroup->GetMaterial();
			
			if (material->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
			{
				MeshInstanceNode::LightmapData * data = meshInstance->GetLightmapDataForIndex(k);
				if(data && data->lightmap)
				{
					material->SetSetupLightmap(false);
					material->SetTexture(Material::TEXTURE_DECAL, data->lightmap);
					material->SetUvOffset(data->uvOffset);
					material->SetUvScale(data->uvScale);
				}
				else
				{
					material->SetSetupLightmap(true);
					material->SetSetupLightmapSize(meshInstance->GetCustomProperties()->GetInt32(Format("#%d.lightmap.size", k), 128));
				}
			}

			polygroup->GetMaterial()->Draw(polygroup->GetPolygonGroup(), meshInstance->materialState);
		}
	}

	static void Run(Scene * scene)
	{
		TemplatePool<uint32> * visibilityFlags = scene->entityManager->GetLinkedTemplatePools<uint32>("flags");

		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 

		while(visibilityFlags)
		{
			EntityFamily * family = visibilityFlags->GetEntityFamily();
			TemplatePool<MeshInstanceNode*> * meshInstances = (TemplatePool<MeshInstanceNode*>*)family->GetPoolByDataName("meshInstanceNode");
			TemplatePool<Matrix4> * transforms = (TemplatePool<Matrix4>*)family->GetPoolByDataName("transform");

			int32 count = visibilityFlags->GetCount();
			uint32 * flag = visibilityFlags->GetPtr(0);
			MeshInstanceNode ** meshInstancePointer = meshInstances->GetPtr(0);
			Matrix4 * transform = transforms->GetPtr(0);
			for(int32 i = 0; i < count; ++i)
			{
				if((*flag) & SceneNode::NODE_CLIPPED_THIS_FRAME)
				{
					Matrix4 meshFinalMatrix = (*transform) * prevMatrix;
					RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
					Draw(*meshInstancePointer);
				}

				transform++;
				meshInstancePointer++;
				flag++;
			}

			visibilityFlags = visibilityFlags->next;
		}

		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}
};

};

#endif //__DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__
