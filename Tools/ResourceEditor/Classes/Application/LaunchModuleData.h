#pragma once

#include "TArc/DataProcessing/DataNode.h"

class LaunchModuleData : public DAVA::TArc::DataNode
{
public:
    bool IsLaunchFinished() const
    {
        return launchFinished;
    }

private:
    friend class LaunchModule;
    bool launchFinished = false;
    DAVA_VIRTUAL_REFLECTION(LaunchModuleData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<LaunchModuleData>::Begin()
        .Field("launchFinished", &LaunchModuleData::IsLaunchFinished, nullptr)
        .End();
    }
};