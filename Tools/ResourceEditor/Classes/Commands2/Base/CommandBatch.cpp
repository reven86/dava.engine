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


#include "Commands2/Base/CommandBatch.h"

#include "Debug/DVAssert.h"


CommandBatch::CommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount)
    : Command2(CMDID_BATCH, text)
{
    DVASSERT(commandsCount > 0);
    commandList.reserve(commandsCount);
}

CommandBatch::~CommandBatch()
{
    for (auto &command : commandList)
    {
        DVASSERT(command != nullptr);
        delete command;
    }

    commandList.clear();
    commandIDs.clear();
}

void CommandBatch::Redo()
{
    for (CommandsContainer::iterator i = commandList.begin(), end = commandList.end(); i != end; i++)
    {
        (*i)->Redo();
    }
    DVASSERT(false && "Need Notify about batch REDO");
}

void CommandBatch::Undo()
{
    for (CommandsContainer::reverse_iterator i = commandList.rbegin(), end = commandList.rend(); i != end; i++)
    {
        (*i)->Undo();
    }
    DVASSERT(false && "Need Notify about batch UNDO");
}


DAVA::Entity* CommandBatch::GetEntity() const
{
    return nullptr;
}

void CommandBatch::AddAndExec(Command2* command)
{
    DVASSERT(command != nullptr);

    commandList.push_back(command);
    commandIDs.insert(command->GetId());

    command->Redo();
    DVASSERT(false && "Need notify about batch REDO");
}

bool CommandBatch::Empty() const
{
    return commandList.empty();
}

DAVA::uint32 CommandBatch::Size() const
{
    return static_cast<DAVA::uint32>(commandList.size());
}

Command2* CommandBatch::GetCommand(DAVA::uint32 index) const
{
    if (index < static_cast<DAVA::uint32>(commandList.size()))
        return commandList[index];

    return nullptr;
}

void CommandBatch::RemoveCommands(DAVA::int32 commandId)
{
    auto i = commandList.begin();
    while (i != commandList.end())
    {
        Command2* command = *i;
        if (command->GetId() == commandId)
        {
            delete command;
            i = commandList.erase(i);
        }
        else
        {
            i++;
        }
    }

    commandIDs.erase(commandId);
}

bool CommandBatch::ContainsCommand(DAVA::int32 commandId) const
{
    return commandIDs.count(commandId) > 0;
}

bool CommandBatch::IsMultiCommandBatch() const
{
    return (commandIDs.size() > 1);
}
