#pragma once
#ifndef DAVAENGINE_DATAWRAPPER__H
#include "DataProcessing/DataWrapper.h"
#endif

namespace tarc
{

template<typename T>
DataWrapper::Editor<T>::Editor(DataWrapper& holder_, DAVA::Reflection reflection_)
    : holder(DAVA::RefPtr<DataContext>::ConstructWithRetain(&holder_))
    , reflection(reflection_)
    , dataCopy(reflection.GetValue().Cast<T>())
{
}

template<typename T>
DataWrapper::Editor<T>::~Editor()
{
    reflection.SetValue(DAVA::Any(dataCopy));
    holder->Sync();
}

template<typename T>
DataWrapper::Editor<T>::Editor(Editor<T>&& other)
    : reflection(std::move(other.reflection))
    , dataCopy(std::move(other.dataCopy))
    , holder(other.holder)
{
    other.holder = nullptr;
}

template<typename T>
DataWrapper::Editor<T>& DataWrapper::Editor<T>::operator=(Editor<T>&& other)
{
    if (&other == this)
        return *this;

    reflection = std::move(other.reflection);
    dataCopy = std::move(other.dataCopy);
    holder = other.holder;
    other.holder = nullptr;

    return *this;
}

template<typename T>
T& DataWrapper::Editor<T>::operator->()
{
    return dataCopy;
}

DataWrapper::DataWrapper(DataWrapper&& other)
    : impl(std::move(other.impl))
{
}

DataWrapper& DataWrapper::operator=(DataWrapper&& other)
{
    if (&other == this)
        return *this;

    impl = std::move(other.impl);

    return *this;
}

template<typename T>
DataWrapper::Editor<T> DataWrapper::CreateEditor()
{
    if (HasData())
    {
        DAVA::Reflection reflection = dataAccessor(context);
        try
        {
            return Editor<T>(this, reflection);
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