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



#ifndef __GROUP_ENTITIES_FOR_MULTISELECT__COMMAND_H__
#define __GROUP_ENTITIES_FOR_MULTISELECT__COMMAND_H__

#include "Commands2/Command2.h"
#include "Qt/Scene/EntityGroup.h"
#include "Qt/Scene/SceneEditor2.h"
#include "Scene3D/Entity.h"

class GroupEntitiesForMultiselectCommand : public Command2
{
public:
	GroupEntitiesForMultiselectCommand(const EntityGroup &entities);
	virtual ~GroupEntitiesForMultiselectCommand();

	virtual void Undo();
	virtual void Redo();

	virtual DAVA::Entity* GetEntity() const;

protected:
	EntityGroup				entitiesToGroup;
	DAVA::Entity*			resultEntity;
	DAVA::Map<DAVA::Entity*, DAVA::Entity*>	originalChildParentRelations;//child, paretn
	SceneEditor2*			sceneEditor;
	
	DAVA::Map<DAVA::Entity*, DAVA::Matrix4> originalMatrixes; // local, world
	DAVA::Map<DAVA::Entity*, DAVA::Component*> originalLodComponents;
		
	void UpdateTransformMatrixes(Entity* entity, Matrix4& worldMatrix);
	void MoveEntity(Entity* entity, Vector3& destPoint);
	Entity* GetEntityWithSolidProp(Entity* en);
	void GetLodComponentsRecursive(Entity* fromEntity, DAVA::Map<DAVA::Entity*, DAVA::Component*>& hostEntitiesAndComponents);
    
    bool IsSelectionValid(const EntityGroup &entities);
};

#endif // __GROUP_ENTITIES_FOR_MULTISELECT__COMMAND_H__
