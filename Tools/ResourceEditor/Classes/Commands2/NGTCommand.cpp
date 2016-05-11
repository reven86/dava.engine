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

#include "NGTCommand.h"
#include "Commands2/Base/Command2.h"

#include "NgtTools/Common/GlobalContext.h"

#include "Debug/DVAssert.h"

const char* NGTCommand::getId() const
{
    return getClassIdentifier<NGTCommand>();
}

ObjectHandle NGTCommand::execute(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    command->Execute();
    return CommandErrorCode::COMMAND_NO_ERROR;
}

CommandThreadAffinity NGTCommand::threadAffinity() const
{
    return CommandThreadAffinity::UI_THREAD;
}

bool NGTCommand::canUndo(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    return command->CanUndo();
}

bool NGTCommand::undo(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    DVASSERT(command->CanUndo() == true);
    command->Undo();
    return true;
}

bool NGTCommand::redo(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    command->Redo();
    return true;
}

ObjectHandle NGTCommand::getCommandDescription(const ObjectHandle& arguments) const
{
    Command2* command = arguments.getBase<Command2>();
    DVASSERT(command != nullptr);
    IDefinitionManager* defManager = NGTLayer::queryInterface<IDefinitionManager>();
    DVASSERT(defManager != nullptr);
    auto handle = GenericObject::create(*defManager);
    handle->set("Name", command->GetText());
    return ObjectHandle(std::move(handle));
}
