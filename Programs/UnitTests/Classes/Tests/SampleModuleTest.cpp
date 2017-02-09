#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"

#include "SampleModule.h"

using namespace DAVA;

#if !defined(__DAVAENGINE_ANDROID__)

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

#endif //#if defined(__DAVAENGINE_ANDROID__)
