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



#include "SetSwitchIndexHelper.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "Scene3D/Entity.h"

namespace DAVA
{

void SetSwitchIndexHelper::ProcessSwitchIndexUpdate(uint32 value, eSET_SWITCH_INDEX state, Set<Entity*>& entities, Map<SwitchComponent *, int32>& originalIndexes)
{
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		
		List<Entity*> switchComponents;
		if( SetSwitchIndexHelper::FOR_SELECTED == state)
		{
			Entity *selectedNode = SceneDataManager::Instance()->SceneGetSelectedNode(sceneData);
			if(NULL != selectedNode)
			{
				selectedNode->FindComponentsByTypeRecursive(Component::SWITCH_COMPONENT, switchComponents);
			}
		}
		if( SetSwitchIndexHelper::FOR_SCENE == state)
		{
			sceneData->GetScene()->FindComponentsByTypeRecursive(Component::SWITCH_COMPONENT, switchComponents);
		}
		
		for(List<Entity*>::const_iterator it = switchComponents.begin(); it != switchComponents.end(); ++it)
		{
			Entity* en = *it;
			SwitchComponent * switchComponent = cast_if_equal<SwitchComponent*>(en->GetComponent(Component::SWITCH_COMPONENT));
			if(NULL == switchComponent)
			{
				continue;
			}

			int32 originalIndex = switchComponent->GetSwitchIndex();
			if(value == originalIndex)
			{
				continue;
			}
			
			originalIndexes[switchComponent] = switchComponent->GetSwitchIndex();
			switchComponent->SetSwitchIndex(value);
			entities.insert(en);
		}
	}
}

void SetSwitchIndexHelper::RestoreOriginalIndexes(Map<SwitchComponent *, int32>& originalIndexes, Set<Entity*>& affectedEntities)
{
	for(Map<SwitchComponent *, int32>::iterator it = originalIndexes.begin(); it != originalIndexes.end(); ++it)
	{
		SwitchComponent* component = (*it).first;
		int32 origIndex = (*it).second;
		component->SetSwitchIndex(origIndex);
	}

	originalIndexes.clear();
	affectedEntities.clear();
}

};
