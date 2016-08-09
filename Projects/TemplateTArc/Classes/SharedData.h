#pragma once

#include "DataProcessing/DataNode.h"

#include "Reflection/ReflectionRegistrator.h"

class SharedData : public tarc::DataNode
{
    DAVA_DECLARE_TYPE_INITIALIZER
    DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION
public:
    SharedData()
    {
    }

    IMPLEMENT_TYPE(SharedData);

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
};

DAVA_TYPE_INITIALIZER(SharedData)
{
    DAVA::ReflectionRegistrator<SharedData>::Begin()
    .Base<tarc::DataNode>()
    .Field("sharedInt", &SharedData::sharedInt)
    .End();
}
