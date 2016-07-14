#ifndef __SOUND_COMPONENT_COMMANDS_H__
#define __SOUND_COMPONENT_COMMANDS_H__

#include "DAVAEngine.h"
#include "QtTools/Commands/CommandWithoutExecute.h"

class AddSoundEventCommand : public CommandWithoutExecute
{
public:
    AddSoundEventCommand(DAVA::Entity* entity, DAVA::SoundEvent* sEvent);
    ~AddSoundEventCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;

private:
    DAVA::Entity* entity;
    DAVA::SoundEvent* savedEvent;
};

class RemoveSoundEventCommand : public CommandWithoutExecute
{
public:
    RemoveSoundEventCommand(DAVA::Entity* entity, DAVA::SoundEvent* sEvent);
    ~RemoveSoundEventCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;

private:
    DAVA::Entity* entity;
    DAVA::SoundEvent* savedEvent;
};

class SetSoundEventFlagsCommand : public CommandWithoutExecute
{
public:
    SetSoundEventFlagsCommand(DAVA::Entity* entity, DAVA::uint32 eventIndex, DAVA::uint32 flags);
    ~SetSoundEventFlagsCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const;

private:
    DAVA::Entity* entity;
    DAVA::SoundComponent* affectComponent;

    DAVA::uint32 index;
    DAVA::uint32 oldFlags;
    DAVA::uint32 newFlags;
};
#endif // __SOUND_COMPONENT_COMMANDS_H__
