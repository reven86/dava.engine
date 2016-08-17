#include "DAVAEngine.h"
#include "Debug/Backtrace.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;
using namespace DAVA::Debug;

DAVA_TESTCLASS (DebugTest)
{
    DAVA_TEST(StackTraceTest)
    {
        // just check that we don't crash
        Vector<StackFrame> stackTrace = GetBacktrace();
        TEST_VERIFY(stackTrace.empty() || !stackTrace.empty());
    }
};
