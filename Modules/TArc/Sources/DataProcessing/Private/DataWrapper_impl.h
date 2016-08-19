#pragma once
#ifndef DAVAENGINE_DATAWRAPPER__H
#include "DataProcessing/DataWrapper.h"
#endif

namespace tarc
{

template<typename T>
DataEditor<T>::DataEditor(DataWrapper& holder_, DAVA::Reflection reflection_)
    : reflection(reflection_)
    , holder(holder_)
{
    DAVA::ReflectedObject refObject = reflection.GetValueObject();
    dataPtr = refObject.GetPtr<T>();
}

template<typename T>
DataEditor<T>::~DataEditor()
{
    holder.Sync(false);
}

template<typename T>
DataEditor<T>::DataEditor(DataEditor<T>&& other)
    : reflection(std::move(other.reflection))
    , dataPtr(std::move(other.dataPtr))
    , holder(other.holder)
{
    other.holder = nullptr;
}

template<typename T>
DataEditor<T>& DataEditor<T>::operator=(DataEditor<T>&& other)
{
    if (&other == this)
        return *this;

    reflection = std::move(other.reflection);
    dataPtr = std::move(other.dataCopy);
    holder = other.holder;
    other.holder = nullptr;

    return *this;
}

template<typename T>
T* DataEditor<T>::operator->()
{
    return dataPtr;
}

template<typename T>
DataEditor<T> DataWrapper::CreateEditor()
{
    if (HasData())
    {
        DAVA::Reflection reflection = GetData();
        try
        {
            return DataEditor<T>(*this, reflection);
        }
        catch (std::runtime_error& e)
        {
            DAVA::Logger::Error(e.what());
            throw e;
        }
    }

    throw std::runtime_error(DAVA::Format("Somebody tried to create editor for data that doesn't exist. T = %s", DAVA::Type::Instance<T>()->GetName()));
}

}