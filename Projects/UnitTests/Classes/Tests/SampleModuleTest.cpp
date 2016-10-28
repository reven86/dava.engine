#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#if defined(__DAVAENGINE_COREV2__)
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#endif

#include "SampleModule.h"

using namespace DAVA;

#if !defined(__DAVAENGINE_ANDROID__)

DAVA_TESTCLASS (SampleModuleTest)
{
    DAVA_TEST (CheckStatus)
    {
#if defined(__DAVAENGINE_COREV2__)
        const ModuleManager& moduleManager = *Engine::Instance()->GetContext()->moduleManager;
#else
        const ModuleManager& moduleManager = Core::Instance()->GetModuleManager();
#endif
        Test::SampleModule* sampleModule = moduleManager.GetModule<Test::SampleModule>();

        auto statusList = sampleModule->StatusList();

        TEST_VERIFY(statusList.size() == 2);
        TEST_VERIFY(statusList[0] == Test::SampleModule::ES_UNKNOWN);
        TEST_VERIFY(statusList[1] == Test::SampleModule::ES_INIT);
    }
};

#endif //#if defined(__DAVAENGINE_ANDROID__)
