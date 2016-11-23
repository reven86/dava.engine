#pragma once
#ifndef DAVAENGINE_DATACONTEXT__H
#include "TArc/DataProcessing/DataContext.h"
#endif

namespace DAVA
{
namespace TArc
{
template <typename T>
void DataContext::DeleteData()
{
    DeleteData(ReflectedType::Get<T>());
}

template <typename T>
T* DataContext::GetData() const
{
    return static_cast<T*>(GetData(ReflectedType::Get<T>()));
}
} // namespace TArc
} // namespace DAVA
