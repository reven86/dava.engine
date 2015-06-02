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


#include "Commands2/EntityAddCommand.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"


EntityAddCommand::EntityAddCommand(DAVA::Entity* _entityToAdd, DAVA::Entity* toParent)
	: Command2(CMDID_ENTITY_ADD, "Add Entity")
    , entityToAdd(_entityToAdd)
    , parentToAdd(toParent)
{
	SafeRetain(entityToAdd);
}

EntityAddCommand::~EntityAddCommand()
{
	SafeRelease(entityToAdd);
}

void EntityAddCommand::Undo()
{
    if(NULL != parentToAdd && NULL != entityToAdd)
    {
        parentToAdd->RemoveNode(entityToAdd);
    }
}

void EntityAddCommand::Redo()
{
    if(parentToAdd)
	{
        parentToAdd->AddNode(entityToAdd);

		//Workaround for correctly adding of switch
		DAVA::SwitchComponent *sw = GetSwitchComponent(entityToAdd);
		if(sw)
		{
			sw->SetSwitchIndex(sw->GetSwitchIndex());
		}
	}
}

DAVA::Entity* EntityAddCommand::GetEntity() const
{
	return entityToAdd;
}