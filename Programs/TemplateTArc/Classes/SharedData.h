#pragma once

#include "DataProcessing/DataNode.h"

#include "Reflection/Registrator.h"

class SharedData : public DAVA::TArc::DataNode
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

    DAVA_VIRTUAL_REFLECTION(SharedData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SharedData>::Begin()
        .Field("sharedInt", &SharedData::sharedInt)
        .End();
    }
};
