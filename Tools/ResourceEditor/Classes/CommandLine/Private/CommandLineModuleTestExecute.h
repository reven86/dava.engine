#pragma once

class CommandLineModule;

class CommandLineModuleTestExecute
{
public:
    static void ExecuteModule(CommandLineModule* module);
    static void InitModule(CommandLineModule* module);
    static bool ProcessModule(CommandLineModule* module);
    static void FinalizeModule(CommandLineModule* module);
};
