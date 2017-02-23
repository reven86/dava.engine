#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "SampleModule.h"

using namespace DAVA;

DAVA_TESTCLASS (SampleModuleTest)
{
    DAVA_TEST (CheckStatus)
    {
        const ModuleManager& moduleManager = *GetEngineContext()->moduleManager;
        SampleModule* sampleModule = moduleManager.GetModule<SampleModule>();

        auto statusList = sampleModule->StatusList();

        TEST_VERIFY(statusList.size() == 2);
        TEST_VERIFY(statusList[0] == SampleModule::ES_UNKNOWN);
        TEST_VERIFY(statusList[1] == SampleModule::ES_INIT);
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
