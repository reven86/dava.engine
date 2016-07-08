#include "AndroidBacktraceChooser.h"

#if defined(CRASH_HANDLER_CUSTOMSIGNALS)

namespace DAVA
{
BacktraceInterface* AndroidBacktraceChooser::backtraceProvider = nullptr;
BacktraceInterface* AndroidBacktraceChooser::ChooseBacktraceAndroid()
{
    // try to create at least one bactrace provider
    if (backtraceProvider == nullptr)
    {
        backtraceProvider = BacktraceCorkscrewImpl::Load();
        if (backtraceProvider == nullptr)
        {
            return nullptr;
        }

        // building memory memp of process at this point
        // all important libs are likely to be loaded at this point
        backtraceProvider->BuildMemoryMap();
        return backtraceProvider;
    }
    else
    {
        return backtraceProvider;
    }
}
void AndroidBacktraceChooser::ReleaseBacktraceInterface()
{
    SafeDelete(backtraceProvider);
}
}

#endif // defined(CRASH_HANDLER_CUSTOMSIGNALS)
