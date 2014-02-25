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



#ifndef __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__
#define __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "BulletObject.h"
#include "Qt/Scene/EntityGroup.h"

class EditorBodyControl;
class SceneEditor2;

class CommandEntityModification: public Command
{
public:
	CommandEntityModification(Command::eCommandType type, CommandList::eCommandId id);

protected:
	Set<Entity*> entities;

	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();
};

class CommandTransformObject: public CommandEntityModification
{
public:
	CommandTransformObject(DAVA::Entity* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();

	void UpdateCollision();
};

class CommandGroupEntitiesForMultiselect: public CommandEntityModification
{
public:
	CommandGroupEntitiesForMultiselect(const EntityGroup* entities);
	
	Entity* GetResultEntity(){return resultEntity;}

protected:
	EntityGroup				entitiesToGroup;
	Entity*					resultEntity;
	Map<Entity*, Entity*>	originalChildParentRelations;//child, paretn
	SceneEditor2*		sceneEditor;
	
	Map<Entity*, Matrix4> originalMatrixes; // local, world

	virtual void Execute();
	virtual void Cancel();

	void UpdateTransformMatrixes(Entity* entity, Matrix4& worldMatrix);
	void MoveEntity(Entity* entity, Vector3& destPoint);
	Entity* GetEntityWithSolidProp(Entity* en);
};

class CommandCloneObject: public CommandEntityModification
{
public:
	DAVA_DEPRECATED(CommandCloneObject(DAVA::Entity* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld));// DEPRECATED: using SceneDataManager
	virtual ~CommandCloneObject();

	Entity* GetClonedNode();

protected:
	DAVA::Entity* originalNode;
	EditorBodyControl* bodyControl;
	btCollisionWorld* collisionWorld;

	virtual void Execute();
	virtual void Cancel();

	DAVA_DEPRECATED(void UpdateCollision(DAVA::Entity* node));
};

class CommandCloneAndTransform: public MultiCommand
{
public:
	CommandCloneAndTransform(DAVA::Entity* originalNode,
							 const DAVA::Matrix4& finalTransform,
							 EditorBodyControl* bodyControl,
							 btCollisionWorld* collisionWorld);
	virtual ~CommandCloneAndTransform();

protected:
	DAVA::Entity* originalNode;
	DAVA::Entity* clonedNode;
	EditorBodyControl* bodyControl;
	btCollisionWorld* collisionWorld;
	DAVA::Matrix4 transform;

	CommandCloneObject* cloneCmd;
	CommandTransformObject* transformCmd;

	virtual void Execute();
	virtual void Cancel();
};

class CommandPlaceOnLandscape: public CommandEntityModification
{
public:
	CommandPlaceOnLandscape(DAVA::Entity* node, EditorBodyControl* bodyControl);

protected:
	DAVA::Matrix4 undoTransform;
	EditorBodyControl* bodyControl;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

class CommandRestoreOriginalTransform: public CommandEntityModification
{
public:
	CommandRestoreOriginalTransform(DAVA::Entity* node);

protected:
	DAVA::Map<DAVA::Entity*, DAVA::Matrix4> undoTransforms;

	virtual void Execute();
	virtual void Cancel();

	void StoreCurrentTransform(DAVA::Entity* node);
	void RestoreTransform(DAVA::Entity* node);
};

#endif /* defined(__RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__) */
