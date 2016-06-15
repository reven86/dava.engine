#pragma once

#include <core_command_system/command.hpp>
#include <core_reflection/generic/generic_object.hpp>

class NGTCommand : public wgt::Command
{
public:
    const char* getId() const override;
    wgt::ObjectHandle execute(const wgt::ObjectHandle& arguments) const override;

    wgt::CommandThreadAffinity threadAffinity() const override;

    bool customUndo() const override;
    bool canUndo(const wgt::ObjectHandle& arguments) const override;
    bool undo(const wgt::ObjectHandle& arguments) const override;
    bool redo(const wgt::ObjectHandle& arguments) const override;
    wgt::ObjectHandle getCommandDescription(const wgt::ObjectHandle& arguments) const override;
};
