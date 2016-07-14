#ifndef __KEYEDARCHIVE_COMMAND_H__
#define __KEYEDARCHIVE_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
    class KeyedArchive;
}

class KeyedArchiveAddValueCommand : public CommandWithoutExecute
{
public:
    KeyedArchiveAddValueCommand(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::VariantType& _val);
    ~KeyedArchiveAddValueCommand();

    void Undo() override;
    void Redo() override;

    DAVA::KeyedArchive* archive = nullptr;
    DAVA::String key;
    DAVA::VariantType val;
};

class KeyeadArchiveRemValueCommand : public CommandWithoutExecute
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

class KeyeadArchiveSetValueCommand : public CommandWithoutExecute
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
