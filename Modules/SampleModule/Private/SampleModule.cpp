#include "SampleModule.h"

namespace DAVA
{
SampleModule::SampleModule(Engine* engine)
    : IModule(engine)
{
    statusList.emplace_back(eStatus::ES_UNKNOWN);
}

void SampleModule::Init()
{
    statusList.emplace_back(eStatus::ES_INIT);
}

void SampleModule::Shutdown()
{
    statusList.emplace_back(eStatus::ES_SHUTDOWN);
}
}