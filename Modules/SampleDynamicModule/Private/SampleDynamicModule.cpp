#pragma once

#include "SampleDynamicModule.h"

namespace DAVA
{
namespace Test
{
SampleDynamicModule::SampleDynamicModule(Engine* engine)
    : IModule(engine)
{
    statusList.emplace_back(eStatus::ES_UNKNOWN);
}

void SampleDynamicModule::Init()
{
    statusList.emplace_back(eStatus::ES_INIT);
}

void SampleDynamicModule::Shutdown()
{
    statusList.emplace_back(eStatus::ES_SHUTDOWN);
}
}
}