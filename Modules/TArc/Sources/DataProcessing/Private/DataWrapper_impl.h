#pragma once
#ifndef DAVAENGINE_DATAWRAPPER__H
#include "DataProcessing/DataWrapper.h"
#endif

#include "Logger/Logger.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
namespace TArc
{
template <typename T>
DataEditor<T>::DataEditor(DataWrapper& holder_, Reflection reflection_)
    : reflection(reflection_)
    , holder(holder_)
{
    ReflectedObject refObject = reflection.GetValueObject();
    dataPtr = refObject.GetPtr<T>();
    copyValue = *dataPtr;
}

template <typename T>
DataEditor<T>::~DataEditor()
{
    holder.SyncWithEditor(Reflection::Create(&copyValue).ref);
}

template <typename T>
DataEditor<T>::DataEditor(DataEditor<T>&& other)
    : reflection(std::move(other.reflection))
    , dataPtr(std::move(other.dataPtr))
    , copyValue(std::move(other.copyValue))
    , holder(other.holder)
{
    other.holder = nullptr;
}

template <typename T>
DataEditor<T>& DataEditor<T>::operator=(DataEditor<T>&& other)
{
    if (&other == this)
        return *this;

    reflection = std::move(other.reflection);
    dataPtr = std::move(other.dataCopy);
    copyValue = std::move(other.copyValue);
    holder = other.holder;
    other.holder = nullptr;

    return *this;
}

template <typename T>
T* DataEditor<T>::operator->()
{
    return dataPtr;
}

template <typename T>
DataEditor<T> DataWrapper::CreateEditor()
{
    if (HasData())
    {
        Reflection reflection = GetData();
        try
        {
            return DataEditor<T>(*this, reflection);
        }
        catch (std::runtime_error& e)
        {
            Logger::Error(e.what());
            throw e;
        }
    }

    throw std::runtime_error(Format("Somebody tried to create editor for data that doesn't exist. T = %s", Type::Instance<T>()->GetName()));
}
} // namespace TArc
} // namespace DAVA
