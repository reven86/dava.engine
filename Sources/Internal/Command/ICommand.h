#pragma once

namespace DAVA
{
class ICommand
{
public:
    virtual ~ICommand() = default;
    virtual void Redo() = 0;
    virtual void Undo() = 0;
};
}
