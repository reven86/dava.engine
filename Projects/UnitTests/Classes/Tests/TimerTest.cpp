#include "UnitTests/UnitTests.h"

#include "Concurrency/Thread.h"
#include "Time/SystemTimer.h"

DAVA_TESTCLASS (TimerTest)
{
    DAVA_TEST (TestPrecision)
    {
        using namespace DAVA;

        const int64 sleepTimeMs[] = {
            1, 50, 100, 500, 1000, 2000
        };

        for (int64 sleepTime : sleepTimeMs)
        {
            int64 beginMs = SystemTimer::GetAbsoluteMillis();
            int64 beginUs = SystemTimer::GetAbsoluteMicros();
            int64 beginNs = SystemTimer::GetAbsoluteNanos();

            Thread::Sleep(static_cast<uint32>(sleepTime));

            int64 deltaMs = SystemTimer::GetAbsoluteMillis() - beginMs;
            int64 deltaUs = SystemTimer::GetAbsoluteMicros() - beginUs;
            int64 deltaNs = SystemTimer::GetAbsoluteNanos() - beginNs;

            TEST_VERIFY(deltaMs >= sleepTime);
            TEST_VERIFY(deltaUs >= sleepTime * 1000ll);
            TEST_VERIFY(deltaNs >= sleepTime * 1000000ll);
        }
    }

    DAVA_TEST (TestFrameDelta)
    {
        // All logic in Update method for this test
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        using namespace DAVA;

        if (testName != "TestFrameDelta")
            return;

        if (testFrameDeltaStage == 0)
        {
            // On first Update call make sleep to ensure real frame delta is greater than frame delta
            Thread::Sleep(1000);
        }
        else
        {
            // On next Update call check real frame delta and frame delta
            float32 frameDelta = SystemTimer::GetFrameDelta();
            float32 realFrameDelta = SystemTimer::GetRealFrameDelta();

            TEST_VERIFY(0.001f <= frameDelta && frameDelta <= 0.1f);
            TEST_VERIFY(frameDelta == timeElapsed);
            TEST_VERIFY(realFrameDelta >= 1.f);
        }
        testFrameDeltaStage += 1;
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        if (testName == "TestFrameDelta")
        {
            return testFrameDeltaStage > 1;
        }
        return true;
    }

    int testFrameDeltaStage = 0;
};
