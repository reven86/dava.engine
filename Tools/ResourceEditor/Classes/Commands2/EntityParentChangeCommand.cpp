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


#include "Commands2/EntityParentChangeCommand.h"

EntityParentChangeCommand::EntityParentChangeCommand(DAVA::Entity* _entity, DAVA::Entity *_newParent, DAVA::Entity *_newBefore /* = NULL */)
	: Command2(CMDID_ENTITY_CHANGE_PARENT, "Move entity")
	, entity(_entity)
	, oldParent(NULL)
	, oldBefore(NULL)
	, newParent(_newParent)
	, newBefore(_newBefore)
{
	SafeRetain(entity);

	if(NULL != entity)
	{
		oldParent = entity->GetParent();

		if(NULL != oldParent)
		{
			oldBefore = oldParent->GetNextChild(entity);
		}
	}
}

EntityParentChangeCommand::~EntityParentChangeCommand()
{
	SafeRelease(entity);
}

void EntityParentChangeCommand::Undo()
{
	if(NULL != entity)
	{
		if(NULL != oldParent)
		{
			if(NULL != oldBefore)
			{
				oldParent->InsertBeforeNode(entity, oldBefore);
			}
			else
			{
				oldParent->AddNode(entity);
			}
		}
		else
		{
			newParent->RemoveNode(entity);
		}
	}
}

void EntityParentChangeCommand::Redo()
{
	if(NULL != entity && NULL != newParent)
	{
		if(NULL != newBefore)
		{
			newParent->InsertBeforeNode(entity, newBefore);
		}
		else
		{
			newParent->AddNode(entity);
		}
	}
}

DAVA::Entity* EntityParentChangeCommand::GetEntity() const
{
	return entity;
}
