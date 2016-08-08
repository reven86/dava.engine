#pragma once

#include "DataProcessing/DataContext.h"

namespace tarc
{

class ContextManager
{
public:
    virtual DataContext::ContextID CreateContext() = 0;
    // throw std::runtime_error if context with contextID doesn't exists
    virtual void DeleteContext(DataContext::ContextID contextID) = 0;
    virtual void ActivateContext(DataContext::ContextID contextID) = 0;
};

}