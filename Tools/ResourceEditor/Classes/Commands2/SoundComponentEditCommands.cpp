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


#include "Commands2/SoundComponentEditCommands.h"

AddSoundEventCommand::AddSoundEventCommand(DAVA::Entity *_entity, DAVA::SoundEvent * _event)
	: Command2(CMDID_SOUND_ADD_EVENT, "Add Sound Event")
{
	DVASSERT(_entity);
    DVASSERT(_event);

	savedEvent = SafeRetain(_event);
    entity = SafeRetain(_entity);
}

AddSoundEventCommand::~AddSoundEventCommand()
{
 	SafeRelease(savedEvent);
    SafeRelease(entity);
}

void AddSoundEventCommand::Redo()
{
    DAVA::SoundComponent * component = GetSoundComponent(entity);
    DVASSERT(component);
    component->AddSoundEvent(savedEvent);
}

void AddSoundEventCommand::Undo()
{
    savedEvent->Stop();
    DAVA::SoundComponent * component = GetSoundComponent(entity);
    DVASSERT(component);
 	component->RemoveSoundEvent(savedEvent);
}

DAVA::Entity* AddSoundEventCommand::GetEntity() const
{
	return entity;
}

RemoveSoundEventCommand::RemoveSoundEventCommand(DAVA::Entity *_entity, DAVA::SoundEvent * _event)
    : Command2(CMDID_SOUND_REMOVE_EVENT, "Remove Sound Event")
{
    DVASSERT(_entity);
    DVASSERT(_event);

    savedEvent = SafeRetain(_event);
    entity = SafeRetain(_entity);
}

RemoveSoundEventCommand::~RemoveSoundEventCommand()
{
    SafeRelease(savedEvent);
    SafeRelease(entity);
}

void RemoveSoundEventCommand::Redo()
{
    savedEvent->Stop();
    DAVA::SoundComponent * component = GetSoundComponent(entity);
    DVASSERT(component);
    component->RemoveSoundEvent(savedEvent);
}

void RemoveSoundEventCommand::Undo()
{
    DAVA::SoundComponent * component = GetSoundComponent(entity);
    DVASSERT(component);
    component->AddSoundEvent(savedEvent);
}

DAVA::Entity* RemoveSoundEventCommand::GetEntity() const
{
    return entity;
}

SetSoundEventFlagsCommand::SetSoundEventFlagsCommand(DAVA::Entity *_entity, DAVA::uint32 eventIndex, DAVA::uint32 flags)
    : Command2(CMDID_SOUND_REMOVE_EVENT, "Set Sound Event Flags"),
    index(eventIndex),
    newFlags(flags)
{
    entity = SafeRetain(_entity);
    DVASSERT(entity);

    affectComponent = GetSoundComponent(entity);
    DVASSERT(affectComponent);

    oldFlags = affectComponent->GetSoundEventFlags(index);
}

SetSoundEventFlagsCommand::~SetSoundEventFlagsCommand()
{
    SafeRelease(entity);
}

void SetSoundEventFlagsCommand::Redo()
{
    affectComponent->SetSoundEventFlags(index, newFlags);
}

void SetSoundEventFlagsCommand::Undo()
{
    affectComponent->SetSoundEventFlags(index, oldFlags);
}

DAVA::Entity* SetSoundEventFlagsCommand::GetEntity() const
{
    return entity;
}