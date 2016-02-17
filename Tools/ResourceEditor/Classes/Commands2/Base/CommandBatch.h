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


#ifndef __COMMAND_BATCH_H__
#define __COMMAND_BATCH_H__

#include "Base/BaseTypes.h"

#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandNotify.h"

class CommandBatch final : public Command2
{
public:
    CommandBatch(const DAVA::String& text, DAVA::uint32 commandsCount);

    void Undo() override;
    void Redo() override;

    DAVA_DEPRECATED(DAVA::Entity* GetEntity() const override);

    void AddAndExec(std::unique_ptr<Command2>&& command);
    void RemoveCommands(DAVA::int32 commandId);

    bool Empty() const;
    DAVA::uint32 Size() const;

    Command2* GetCommand(DAVA::uint32 index) const;

    bool MatchCommandID(DAVA::int32 commandID) const override;
    bool MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const override;

    bool IsMultiCommandBatch() const;

protected:
    using CommandsContainer = DAVA::Vector<std::unique_ptr<Command2>>;
    CommandsContainer commandList;

    DAVA::UnorderedSet<DAVA::int32> commandIDs;
};

inline bool CommandBatch::Empty() const
{
    return commandList.empty();
}

inline DAVA::uint32 CommandBatch::Size() const
{
    return static_cast<DAVA::uint32>(commandList.size());
}

inline bool CommandBatch::IsMultiCommandBatch() const
{
    return (commandIDs.size() > 1);
}


#endif // __COMMAND_BATCH_H__
