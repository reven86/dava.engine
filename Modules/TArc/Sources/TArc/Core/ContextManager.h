#pragma once

#include "TArc/DataProcessing/DataContext.h"

#include <Engine/Qt/RenderWidget.h>

namespace DAVA
{
namespace TArc
{
class ContextManager
{
public:
    virtual ~ContextManager() = default;
    virtual DataContext::ContextID CreateContext(Vector<std::unique_ptr<DataNode>>&& initialData) = 0;
    // throw std::runtime_error if context with contextID doesn't exists
    virtual void DeleteContext(DataContext::ContextID contextID) = 0;
    virtual void ActivateContext(DataContext::ContextID contextID) = 0;
    virtual RenderWidget* GetRenderWidget() const = 0;
};
} // namespace TArc
} // namespace DAVA