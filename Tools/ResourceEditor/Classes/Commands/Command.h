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



#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "DAVAEngine.h"
#include "CommandList.h"

class MultiCommand;
class CommandsManager;
class Command: public DAVA::BaseObject
{
	friend class MultiCommand;
    friend class CommandsManager;
    
protected:
    enum eCommandState
    {
        STATE_VALID = 0,
        STATE_INVALID
    };
    
    
public:
    
    enum eCommandType
    {
        COMMAND_WITHOUT_UNDO_EFFECT = 0,    // Command has no effect onto undo-redo queue
        COMMAND_CLEAR_UNDO_QUEUE,           // Command reset undo queue
        COMMAND_UNDO_REDO                   // You cat undo & redo this command
    };
    
public:
	Command(eCommandType _type, CommandList::eCommandId id);
	virtual ~Command();

protected:	
    
    virtual void Execute() = 0;
    virtual void Cancel() {};

	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

    inline eCommandType Type() const {return commandType; };
	inline CommandList::eCommandId Id() const {return commandId; };
	
    inline void SetState(eCommandState newState) {commandState = newState; };
    inline eCommandState State() const {return commandState; };

protected:
    
    eCommandType commandType;
    eCommandState commandState;
	CommandList::eCommandId commandId;

	DAVA::String commandName;
};

class MultiCommand: public Command
{
public:
	MultiCommand(eCommandType _type, CommandList::eCommandId id);

protected:
	void ExecuteInternal(Command* command);
	void CancelInternal(Command* command);

	DAVA::Set<DAVA::Entity*> GetAffectedEntitiesInternal(Command* command);

	eCommandState GetInternalCommandState(Command* command);
};

#include "../Qt/DockSceneGraph/PointerHolder.h"
DECLARE_POINTER_TYPE(Command *);


#endif // #ifndef __COMMAND_H__