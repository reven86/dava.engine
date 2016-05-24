#pragma once

#include <core_command_system/command.hpp>
#include <core_reflection/generic/generic_object.hpp>

class NGTCommand : public Command
{
public:
    const char* getId() const override;
    ObjectHandle execute(const ObjectHandle& arguments) const override;

    CommandThreadAffinity threadAffinity() const override;

    bool canUndo(const ObjectHandle& arguments) const override;
    bool undo(const ObjectHandle& arguments) const override;
    bool redo(const ObjectHandle& arguments) const override;
    ObjectHandle getCommandDescription(const ObjectHandle& arguments) const override;
};
