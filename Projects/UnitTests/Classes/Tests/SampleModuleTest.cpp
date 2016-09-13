#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "SampleModule.h"

using namespace DAVA;

DAVA_TESTCLASS (SampleModuleTest)
{
    DAVA_TEST (CheckStatus)
    {
        auto statusList = SampleModule::Instance()->StatusList();

        TEST_VERIFY(statusList.size() == 3);
        TEST_VERIFY(statusList[0] == SampleModule::ES_UNKNOWN);
        TEST_VERIFY(statusList[1] == SampleModule::ES_INIT);
        TEST_VERIFY(statusList[2] == SampleModule::ES_POST_INIT);
    }
};