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



#include "Command.h"

using namespace DAVA;


Command::Command(eCommandType _type, CommandList::eCommandId id)
    :   BaseObject()
    ,   commandType(_type)
    ,   commandState(STATE_VALID)
	,	commandName("")
	,	commandId(id)
{
    RegisterPointerType<Command *>(String("Command *"));
}

Command::~Command()
{
	
}

DAVA::Set<DAVA::Entity*> Command::GetAffectedEntities()
{
	Set<Entity*> entities;
	return entities;
}


MultiCommand::MultiCommand(eCommandType _type, CommandList::eCommandId id)
:	Command(_type, id)
{
}

void MultiCommand::ExecuteInternal(Command* command)
{
	DVASSERT(command);
	command->Execute();
}

void MultiCommand::CancelInternal(Command* command)
{
	DVASSERT(command);
	command->Cancel();
}

Command::eCommandState MultiCommand::GetInternalCommandState(Command* command)
{
	DVASSERT(command);
	return command->State();
}

DAVA::Set<DAVA::Entity*> MultiCommand::GetAffectedEntitiesInternal(Command* command)
{
	DVASSERT(command);
	return command->GetAffectedEntities();
}