#pragma once

#include "Functional\Signal.h"
#include "Reflection\Reflection.h"

namespace tarc
{

class DataNode : public DAVA::VirtualReflection
{
    DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION

public:
    virtual void Load(const Serializator& s) = 0;
    virtual void Save(Serializator& s) = 0;

    DAVA::Signal<void(const DataNode&)> DataNodeChanged;
};

}