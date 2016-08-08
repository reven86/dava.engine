#pragma once
#ifndef DAVAENGINE_DATAWRAPPER__H
#include "DataProcessing/DataContext.h"
#endif

namespace tarc
{

template<typename T>
bool DataContext::HasData() const
{
    return HasData(DAVA::Type::Instance<T>());
}

template<typename T>
void DataContext::DeleteData()
{
    DeleteData(DAVA::Type::Instance<T>());
}

template<typename T>
T& DataContext::GetData() const
{
    return static_cast<T&>(GetData(DAVA::Type::Instance<T>()));
}

}
