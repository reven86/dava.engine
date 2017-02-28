#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Controls/PropertyPanel/TimerUpdater.h"

#include <Time/SystemTimer.h>

DAVA_TARC_TESTCLASS(TimerUpdaterTests)
{
    DAVA_TEST (FullUpdateTest)
    {
        TEST_VERIFY(updater == nullptr);
        TEST_VERIFY(currentTestFinished == false);
        updater.reset(new DAVA::TArc::TimerUpdater(500, -1));
        updater->update.Connect([this](DAVA::TArc::PropertiesView::UpdatePolicy policy)
                                {
                                    TEST_VERIFY(policy == PropertiesView::FullUpdate);
                                    DAVA::int32 delta = GetTestDelta();
                                    TEST_VERIFY(delta > 450 && delta < 1000);
                                    currentTestFinished = true;
                                });
    }

    DAVA_TEST (FullFastUpdateTest)
    {
        TEST_VERIFY(updater == nullptr);
        TEST_VERIFY(currentTestFinished == false);
        updater.reset(new DAVA::TArc::TimerUpdater(500, 100));
        updater->update.Connect([this](DAVA::TArc::PropertiesView::UpdatePolicy policy)
                                {
                                    static DAVA::int32 counter = 1;
                                    DAVA::int32 delta = GetTestDelta();
                                    if (counter == 5)
                                    {
                                        TEST_VERIFY(policy == PropertiesView::FullUpdate);
                                        TEST_VERIFY(delta > 450 && delta < 700);
                                    }
                                    else
                                    {
                                        TEST_VERIFY(policy == PropertiesView::FastUpdate);
                                        DAVA::int32 expectedDelta = 100 * counter;
                                        TEST_VERIFY(delta > (expectedDelta - 50) && delta < (expectedDelta + 100));
                                    }
                                    if (counter == 6)
                                    {
                                        currentTestFinished = true;
                                    }

                                    ++counter;
                                });
    }

    void SetUp(const DAVA::String& testName) override
    {
        startTestTime = DAVA::SystemTimer::GetMs();
        TestClass::SetUp(testName);
    }

    void TearDown(const DAVA::String& testName) override
    {
        updater.reset();
        currentTestFinished = false;
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        return currentTestFinished;
    }

    DAVA::int32 GetTestDelta() const
    {
        return DAVA::SystemTimer::GetMs() - startTestTime;
    }

    std::shared_ptr<DAVA::TArc::TimerUpdater> updater;
    DAVA::int32 startTestTime = 0;
    bool currentTestFinished = false;
};
