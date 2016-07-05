#pragma once

#include "Base/BaseTypes.h"
#include "Concurrency/Thread.h"
#include "Debug/DVAssert.h"
#include "Debug/InterthreadBlockingCallMonitor.h"

#if !defined(DAVA_DISABLE_DEADLOCK_MONITOR)

#define DAVA_BEGIN_BLOCKING_CALL(targetThread) \
    do { \
        using namespace DAVA; \
        Vector<uint64> callChain; \
        InterthreadBlockingCallMonitor* monitor = InterthreadBlockingCallMonitor::Instance(); \
        bool deadlock = monitor->BeginBlockingCall(Thread::GetCurrentIdAsInteger(), targetThread, callChain); \
        if (deadlock) \
        { \
            String s = monitor->CallChainToString(callChain); \
            DVASSERT_MSG(0, ("Deadlock callchain: " + s).c_str()); \
        } \
    } while (0)

#define DAVA_END_BLOCKING_CALL(targetThread) \
    do { \
        using namespace DAVA; \
        InterthreadBlockingCallMonitor::Instance()->EndBlockingCall(Thread::GetCurrentIdAsInteger(), targetThread); \
    } while (0)

#else

#define DAVA_BEGIN_BLOCKING_CALL(targetThread)
#define DAVA_END_BLOCKING_CALL(targetThread)

#endif
