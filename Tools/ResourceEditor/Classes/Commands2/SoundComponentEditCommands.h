#ifndef __SOUND_COMPONENT_COMMANDS_H__
#define __SOUND_COMPONENT_COMMANDS_H__

#include "DAVAEngine.h"
#include "Commands2/Base/Command2.h"

class AddSoundEventCommand : public Command2
{
public:
    AddSoundEventCommand(DAVA::Entity* entity, DAVA::SoundEvent* sEvent);
    ~AddSoundEventCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

private:
    DAVA::Entity* entity;
    DAVA::SoundEvent* savedEvent;
};

class RemoveSoundEventCommand : public Command2
{
public:
    RemoveSoundEventCommand(DAVA::Entity* entity, DAVA::SoundEvent* sEvent);
    ~RemoveSoundEventCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

private:
    DAVA::Entity* entity;
    DAVA::SoundEvent* savedEvent;
};

class SetSoundEventFlagsCommand : public Command2
{
public:
    SetSoundEventFlagsCommand(DAVA::Entity* entity, DAVA::uint32 eventIndex, DAVA::uint32 flags);
    ~SetSoundEventFlagsCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const;

private:
    DAVA::Entity* entity;
    DAVA::SoundComponent* affectComponent;

    DAVA::uint32 index;
    DAVA::uint32 oldFlags;
    DAVA::uint32 newFlags;
};
#endif // __SOUND_COMPONENT_COMMANDS_H__
