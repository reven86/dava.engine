#pragma once
#ifndef DAVAENGINE_DATAWRAPPER__H
#include "TArc/DataProcessing/DataWrapper.h"
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
    holder.SyncWithEditor(Reflection::Create(&copyValue));
}

template <typename T>
DataEditor<T>::DataEditor(DataEditor<T>&& other)
    : reflection(std::move(other.reflection))
    , dataPtr(std::move(other.dataPtr))
    , copyValue(std::move(other.copyValue))
    , holder(std::move(other.holder))
{
}

template <typename T>
DataEditor<T>& DataEditor<T>::operator=(DataEditor<T>&& other)
{
    if (&other == this)
        return *this;

    reflection = std::move(other.reflection);
    dataPtr = std::move(other.dataPtr);
    copyValue = std::move(other.copyValue);
    holder = std::move(other.holder);

    return *this;
}

template <typename T>
T* DataEditor<T>::operator->()
{
    return dataPtr;
}

//////////////////////////////////////////////////////////////////////////////////

template <typename T>
DataReader<T>::DataReader(const DataWrapper& holder_)
    : holder(holder_)
{
}

template <typename T>
DataReader<T>::DataReader(DataReader<T>&& other)
    : holder(std::move(other.holder))
{
}

template <typename T>
DataReader<T>& DataReader<T>::operator=(DataReader<T>&& other)
{
    if (&other == this)
        return *this;

    holder = std::move(other.holder);

    return *this;
}

template <typename T>
T const* DataReader<T>::operator->() const
{
    DVASSERT(holder.HasData());
    ReflectedObject refObject = holder.GetData().GetValueObject();
    return refObject.GetPtr<T>();
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
        catch (Exception& e)
        {
            Logger::Error(e.what());
            throw e;
        }
    }

    DAVA_THROW(Exception, Format("Somebody tried to create editor for data that doesn't exist. T = %s", Type::Instance<T>()->GetName()));
}

template <typename T>
DataReader<T> DataWrapper::CreateReader() const
{
    if (HasData())
    {
        try
        {
            return DataReader<T>(*this);
        }
        catch (Exception& e)
        {
            Logger::Error(e.what());
            throw e;
        }
    }

    DAVA_THROW(Exception, Format("Somebody tried to create reader for data that doesn't exist. T = %s", Type::Instance<T>()->GetName()));
}
} // namespace TArc
} // namespace DAVA
