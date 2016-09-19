#pragma once
#ifndef DAVAENGINE_DATAWRAPPER__H
#include "DataProcessing/DataContext.h"
#endif

namespace DAVA
{
namespace TArc
{
template<typename T>
bool DataContext::HasData() const
{
    return HasData(ReflectedType::Get<T>());
}

template<typename T>
void DataContext::DeleteData()
{
    DeleteData(ReflectedType::Get<T>());
}

template<typename T>
T& DataContext::GetData() const
{
    return static_cast<T&>(GetData(ReflectedType::Get<T>()));
}
} // namespace TArc
} // namespace DAVA
