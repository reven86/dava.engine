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

#include "SwitchToRenerObjectConverter.h"



void SwitchToRenerObjectConverter::ConsumeSwitchedRenderObjects(Scene * scene)
{
	SerachForSwitch(scene);
}

void SwitchToRenerObjectConverter::SerachForSwitch(Entity * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		Entity * childNode = currentNode->GetChild(c);
		SerachForSwitch(childNode);
		bool wasReplace = MergeSwitch(childNode);
		if(wasReplace)
		{
			c--;
		}
	}
}

bool SwitchToRenerObjectConverter::MergeSwitch(Entity * entity)
{
	Vector<Entity*> entitiesToRemove;

	SwitchComponent * sw = GetSwitchComponent(entity);
	if(sw)
	{
		RenderComponent * rc = GetRenderComponent(entity);
		RenderObject * ro = 0;
		if(!rc)
		{
			ro = new RenderObject();
			rc = new RenderComponent(ro);
			ro->Release();

			ro->SetAABBox(AABBox3(Vector3(0, 0, 0), Vector3(0, 0, 0)));
			entity->AddComponent(rc);
		}
		else
		{
			ro = rc->GetRenderObject();
		}

		DVASSERT(ro);

		int32 size = entity->GetChildrenCount();
		for(int32 i = 0; i < size; ++i)
		{
			Entity * child = entity->GetChild(i);
			RenderObject * sourceRenderObject = GetRenderObject(child);
			if(sourceRenderObject)
			{
				sourceRenderObject->BakeTransform(GetTransformComponent(child)->GetLocalTransform());
				uint32 sourceSize = sourceRenderObject->GetRenderBatchCount();
				for(uint32 j = 0; j < sourceSize; ++j)
				{
					int32 lodIndex, switchIndex;
					RenderBatch * sourceRenderBatch = sourceRenderObject->GetRenderBatch(j, lodIndex, switchIndex);
					ro->AddRenderBatch(sourceRenderBatch, lodIndex, i);
				}
			}

			entitiesToRemove.push_back(child);
		}

		ro->RecalcBoundingBox();
	}

	uint32 entitiesToRemoveCount = entitiesToRemove.size();
	for(uint32 i = 0; i < entitiesToRemoveCount; ++i)
	{
		entitiesToRemove[i]->GetParent()->RemoveNode(entitiesToRemove[i]);
	}

	return false;
}
