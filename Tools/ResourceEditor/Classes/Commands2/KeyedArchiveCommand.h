#ifndef __KEYEDARCHIVE_COMMAND_H__
#define __KEYEDARCHIVE_COMMAND_H__

#include "Commands2/Base/Command2.h"

class KeyedArchiveAddValueCommand : public Command2
{
public:
    KeyedArchiveAddValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::VariantType& _val);
    ~KeyedArchiveAddValueCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::KeyedArchive* archive;
    DAVA::String key;
    DAVA::VariantType val;
};

class KeyeadArchiveRemValueCommand : public Command2
{
public:
    KeyeadArchiveRemValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key);
    ~KeyeadArchiveRemValueCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::KeyedArchive* archive;
    DAVA::String key;
    DAVA::VariantType val;
};

class KeyeadArchiveSetValueCommand : public Command2
{
public:
    KeyeadArchiveSetValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::VariantType& _val);
    ~KeyeadArchiveSetValueCommand();

    virtual void Undo();
    virtual void Redo();

    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    }

    DAVA::KeyedArchive* archive;
    DAVA::String key;
    DAVA::VariantType oldVal;
    DAVA::VariantType newVal;
};

#endif // __KEYEDARCHIVE_COMMAND_H__
