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
    return HasData(DAVA::ReflectedType::Get<T>());
}

template<typename T>
void DataContext::DeleteData()
{
    DeleteData(DAVA::ReflectedType::Get<T>());
}

template<typename T>
T& DataContext::GetData() const
{
    return static_cast<T&>(GetData(DAVA::ReflectedType::Get<T>()));
}
} // namespace TArc
} // namespace DAVA
