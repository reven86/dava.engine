#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/DataProcessing/DataListener.h"

#include "Logger/Logger.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
namespace DataWrapperDetail
{
Reflection GetDataDefault(const DataContext* context, const ReflectedType* type)
{
    Reflection ret;
    if (context == nullptr)
    {
        return ret;
    }

    DataNode* node = context->GetData(type);
    if (node != nullptr)
    {
        ret = Reflection::Create(node);
    }

    return ret;
}
}

struct DataWrapper::Impl
{
    DataContext* activeContext = nullptr;
    DataAccessor dataAccessor;
    Vector<Any> cachedValues;

    DataListener* listener = nullptr;
    DataListener* nextListenerToSet = nullptr;
#ifdef __DAVAENGINE_DEBUG__
    String typeName;
#endif
};

DataWrapper::DataWrapper(const ReflectedType* type)
    : DataWrapper(Bind(&DataWrapperDetail::GetDataDefault, std::placeholders::_1, type))
{
#ifdef __DAVAENGINE_DEBUG__
    impl->typeName = type->GetPermanentName();
#endif
}

DataWrapper::DataWrapper(const DataAccessor& accessor)
    : impl(new Impl())
{
    impl->dataAccessor = accessor;
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

bool DataWrapper::operator==(const DataWrapper& other) const
{
    return other.impl == impl;
}

void DataWrapper::SetContext(DataContext* context)
{
    DVASSERT(impl != nullptr);
    impl->activeContext = context;
}

void DataWrapper::ClearListener(DataListener* listenerForCheck)
{
    if (impl == nullptr)
    {
        return;
    }

    DVASSERT(listenerForCheck == impl->listener);
    if (impl->listener == impl->nextListenerToSet)
    {
        impl->nextListenerToSet = nullptr;
    }

    impl->listener = nullptr;
}

bool DataWrapper::HasData() const
{
    if (impl == nullptr)
    {
        return false;
    }

    Reflection reflection;
    try
    {
        reflection = impl->dataAccessor(impl->activeContext);
    }
    catch (const std::runtime_error& e)
    {
        Logger::Error(e.what());
        return false;
    }

    return reflection.IsValid();
}

void DataWrapper::SetListener(DataListener* listener)
{
    if (impl == nullptr)
    {
        return;
    }

    impl->nextListenerToSet = listener;
}

void DataWrapper::SetFieldValue(const Any& fieldKey, const Any& value)
{
    DVASSERT(HasData() == true);
    Reflection data = GetData();
    Reflection field = data.GetField(fieldKey);
    DVASSERT(field.IsValid() == true);
    bool result = field.SetValueWithCast(value);
    DVASSERT(result);
    SyncByFieldKey(fieldKey, value);
}

Any DataWrapper::GetFieldValue(const Any& fieldKey) const
{
    DVASSERT(HasData());
    Reflection data = GetData();
    Reflection field = data.GetField(fieldKey);
    DVASSERT(field.IsValid() == true);
    return field.GetValue();
}

bool DataWrapper::IsActive() const
{
    return !impl.unique();
}

void DataWrapper::UpdateCachedValue(int32 id, const Any& value)
{
    DVASSERT(id < impl->cachedValues.size());
    impl->cachedValues[id] = value;
}

void DataWrapper::Sync(bool notifyListener)
{
    DVASSERT(impl != nullptr);

    bool listenerWasChanged = false;
    if (impl->nextListenerToSet != impl->listener)
    {
        if (impl->listener != nullptr)
        {
            impl->listener->RemoveWrapper(*this);
        }
        listenerWasChanged = true;
        impl->listener = impl->nextListenerToSet;
        impl->cachedValues.clear();
        if (impl->nextListenerToSet != nullptr)
        {
            impl->listener->AddWrapper(*this);
        }
    }

    if (impl->listener == nullptr)
    {
        return;
    }

    if (HasData())
    {
        Reflection reflection = GetData();
        Vector<Reflection::Field> fields = reflection.GetFields();

        if (impl->cachedValues.size() != fields.size())
        {
            impl->cachedValues.clear();
            for (const Reflection::Field& field : fields)
            {
                impl->cachedValues.push_back(field.ref.GetValue());
            }
            NotifyListener(notifyListener);
        }
        else
        {
            Vector<Any> fieldNames;
            std::function<void(const Any&)> nameInserter;
            if (notifyListener)
            {
                nameInserter = [&fieldNames](const Any& name) { fieldNames.push_back(name); };
            }
            else
            {
                nameInserter = [](const Any&) {};
            }

            for (size_t i = 0; i < fields.size(); ++i)
            {
                const Reflection::Field& field = fields[i];
                Any newValue = field.ref.GetValue();
                bool valuesEqual = false;
                try
                {
                    valuesEqual = impl->cachedValues[i] == newValue;
                }
                catch (const DAVA::Exception& e)
                {
                    DAVA::Logger::Debug("DataWrapper::Sync: %s", e.what());
                }
                if (!valuesEqual)
                {
                    impl->cachedValues[i] = newValue;
                    nameInserter(field.key);
                }
            }

            if (!fieldNames.empty())
            {
                NotifyListener(notifyListener, fieldNames);
            }
        }
    }
    else
    {
        if (!impl->cachedValues.empty() || listenerWasChanged == true)
        {
            impl->cachedValues.clear();
            NotifyListener(notifyListener);
        }
    }
}

void DataWrapper::SyncByFieldKey(const Any& fieldKey, const Any& v)
{
    if (impl->cachedValues.empty())
    {
        return;
    }

    DVASSERT(impl != nullptr);
    DVASSERT(HasData());
    Reflection data = GetData();

    Vector<Reflection::Field> dataFields = data.GetFields();
    String fieldName = fieldKey.Cast<String>();
    for (size_t i = 0; i < dataFields.size(); ++i)
    {
        if (dataFields[i].key.Cast<String>() == fieldName)
        {
            impl->cachedValues[i] = v;
            break;
        }
    }
}

void DataWrapper::SyncWithEditor(const Reflection& etalonData)
{
    DVASSERT(impl != nullptr);
    DVASSERT(HasData());
    Reflection data = GetData();

    Vector<Reflection::Field> dataFields = data.GetFields();
    Vector<Reflection::Field> etalonFields = etalonData.GetFields();

    if (dataFields.size() != etalonFields.size())
    {
        // if sizes not equal, it means that data is collection
        // and on next Sync iteration we will signalize that all fields were changed
        // and there is no reason to sync cached values
        return;
    }

    DVASSERT(dataFields.size() == impl->cachedValues.size());
    for (size_t i = 0; i < dataFields.size(); ++i)
    {
        Any dataFieldValue = dataFields[i].ref.GetValue();
        Any etalonFieldValue = etalonFields[i].ref.GetValue();

        if (dataFieldValue != etalonFieldValue)
        {
            impl->cachedValues[i] = dataFieldValue;
        }
    }
}

void DataWrapper::NotifyListener(bool sendNotify, const Vector<Any>& fields)
{
    if (sendNotify == false)
        return;

    DVASSERT(impl != nullptr);
    if (impl->listener)
    {
        impl->listener->OnDataChanged(*this, fields);
    }
}

Reflection DataWrapper::GetData() const
{
    DVASSERT(HasData());
    return impl->dataAccessor(impl->activeContext);
}
} // namespace TArc
} // namespace DAVA
