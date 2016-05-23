#ifndef __DAVAFRAMEWORK_ICOMMAND_H__
#define __DAVAFRAMEWORK_ICOMMAND_H__

namespace DAVA
{
class ICommand
{
public:
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual void Redo() = 0;
    virtual void Undo() = 0;
};
}

#endif // __DAVAFRAMEWORK_ICOMMAND_H__