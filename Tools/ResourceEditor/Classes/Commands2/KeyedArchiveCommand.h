#ifndef __KEYEDARCHIVE_COMMAND_H__
#define __KEYEDARCHIVE_COMMAND_H__

#include "Commands2/Base/RECommand.h"

class KeyedArchiveAddValueCommand : public RECommand
{
public:
    KeyedArchiveAddValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::VariantType& _val);
    ~KeyedArchiveAddValueCommand();

    virtual void Undo() override;
    virtual void Redo() override;

    DAVA::KeyedArchive* archive = nullptr;
    DAVA::String key;
    DAVA::VariantType val;
};

class KeyeadArchiveRemValueCommand : public RECommand
{
public:
    KeyeadArchiveRemValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key);
    ~KeyeadArchiveRemValueCommand();

    void Undo() override;
    void Redo() override;

    DAVA::KeyedArchive* archive = nullptr;
    DAVA::String key;
    DAVA::VariantType val;
};

class KeyeadArchiveSetValueCommand : public RECommand
{
public:
    KeyeadArchiveSetValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::VariantType& _val);
    ~KeyeadArchiveSetValueCommand();

    virtual void Undo() override;
    virtual void Redo() override;

    DAVA::KeyedArchive* archive = nullptr;
    DAVA::String key;
    DAVA::VariantType oldVal;
    DAVA::VariantType newVal;
};

#endif // __KEYEDARCHIVE_COMMAND_H__
