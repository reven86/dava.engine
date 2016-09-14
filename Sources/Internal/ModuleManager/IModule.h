#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class IModule
{
public:
    virtual ~IModule() = default;
    virtual void Init()
    {
    }
    virtual void PostInit()
    {
    }
    virtual void Shutdown()
    {
    }
};
}
