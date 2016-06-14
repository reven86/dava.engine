#pragma once

#include <core_generic_plugin/interfaces/i_component_context.hpp>
#include "Debug/DVAssert.h"

namespace NGTLayer
{
void SetGlobalContext(wgt::IComponentContext* context);
wgt::IComponentContext* GetGlobalContext();

template <class T>
T* queryInterface()
{
    wgt::IComponentContext* context = GetGlobalContext();
    DVASSERT(context != nullptr);
    return context->queryInterface<T>();
}
} // namespace NGTLayer