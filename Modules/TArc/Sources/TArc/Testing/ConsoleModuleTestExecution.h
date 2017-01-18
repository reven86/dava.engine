#pragma once

namespace DAVA
{
namespace TArc
{
class ConsoleModule;
class ContextAccessor;

class ConsoleModuleTestExecution
{
public:
    static void ExecuteModule(ConsoleModule* module, ContextAccessor* accessor = nullptr);
    static void InitModule(ConsoleModule* module, ContextAccessor* accessor = nullptr);
    static bool ProcessModule(ConsoleModule* module);
    static void FinalizeModule(ConsoleModule* module);
};
}
}
