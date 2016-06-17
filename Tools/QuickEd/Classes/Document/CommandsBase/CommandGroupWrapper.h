#pragma once

#include <memory>
#include <list>

class CommandStack;

class CommandStackGroup
{
public:
    CommandStackGroup();
    void RemoveStack(const CommandStack* stackToRemove);
    void AddStack(std::unique_ptr<CommandStack> stackToAdd);

    void SetActiveStack(CommandStack* commandStack);

private:
    CommandStack* activeStack = nullptr;
    std::list<std::unique_ptr<CommandStack>> stacks;
};
