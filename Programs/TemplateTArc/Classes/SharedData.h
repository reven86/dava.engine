#pragma once

#include "DataProcessing/DataNode.h"

#include "Reflection/Registrator.h"

class SharedData : public tarc::DataNode
{
public:
    SharedData()
    {
    }

    void SetValue(int v)
    {
        sharedInt = v;
    }
    int GetValue() const
    {
        return sharedInt;
    }

private:
    int sharedInt = 0;

    DAVA_VIRTUAL_REFLECTION(SharedData, tarc::DataNode)
    {
        DAVA::ReflectionRegistrator<SharedData>::Begin()
        .Field("sharedInt", &SharedData::sharedInt)
        .End();
    }
};
