#include "UnitTests/UnitTests.h"

#include "Concurrency/Thread.h"
#include "Time/SystemTimer.h"
#include "Utils/StringFormat.h"

#include <ctime>

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

            TEST_VERIFY_WITH_MESSAGE(deltaMs >= sleepTime, Format("deltaMs=%lld, sleepTime=%lld", deltaMs, sleepTime));
            TEST_VERIFY_WITH_MESSAGE(deltaUs >= sleepTime * 1000ll, Format("deltaUs=%lld, sleepTime=%lld", deltaUs, sleepTime * 1000ll));
            TEST_VERIFY_WITH_MESSAGE(deltaNs >= sleepTime * 1000000ll, Format("deltaNs=%lld, sleepTime=%lld", deltaNs, sleepTime * 1000000ll));
        }

        {
            int64 systime1 = SystemTimer::GetSystemTime();
            int64 systime2 = static_cast<int64>(std::time(nullptr));
            TEST_VERIFY(0 <= (systime2 - systime1) && (systime2 - systime1) <= 1); // Include case when time has updated during second measure
        }

        {
            int64 begin = SystemTimer::GetSystemUptimeMicros();
            Thread::Sleep(2000);
            int64 delta = SystemTimer::GetSystemUptimeMicros() - begin;
            // I do not know period system updates its uptime so choose value less than sleep time by 500ms
            TEST_VERIFY_WITH_MESSAGE(delta >= 1500000ll, Format("delta=%lld", delta));
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
