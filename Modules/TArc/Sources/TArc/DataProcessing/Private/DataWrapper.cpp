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
Reflection GetDataDefault(const DataContext& context, const ReflectedType* type)
{
    Reflection ret;
    DataNode* node = context.GetData(type);
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

    Set<DataListener*> listeners;
};

DataWrapper::DataWrapper(const ReflectedType* type)
    : DataWrapper(Bind(&DataWrapperDetail::GetDataDefault, std::placeholders::_1, type))
{
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

bool DataWrapper::HasData() const
{
    if (impl == nullptr || impl->activeContext == nullptr)
    {
        return false;
    }

    Reflection reflection;
    try
    {
        reflection = impl->dataAccessor(*impl->activeContext);
    }
    catch (const std::runtime_error& e)
    {
        Logger::Error(e.what());
        return false;
    }

    return reflection.IsValid();
}

void DataWrapper::AddListener(DataListener* listener)
{
    if (impl == nullptr)
    {
        return;
    }

    DVASSERT(listener != nullptr);
    listener->InitListener(*this);
    impl->listeners.insert(listener);
}

void DataWrapper::RemoveListener(DataListener* listener)
{
    if (impl == nullptr)
    {
        return;
    }

    DVASSERT(listener != nullptr);
    impl->listeners.erase(listener);
}

bool DataWrapper::IsActive() const
{
    return !impl.unique();
}

void DataWrapper::Sync(bool notifyListeners)
{
    DVASSERT(impl != nullptr);
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
            NotifyListeners(notifyListeners);
        }
        else
        {
            Set<String> fieldNames;
            std::function<void(const String&)> nameInserter;
            if (notifyListeners)
            {
                nameInserter = [&fieldNames](const String& name) { fieldNames.insert(name); };
            }
            else
            {
                nameInserter = [](const String&) {};
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
                    nameInserter(field.key.Cast<String>());
                }
            }

            if (!fieldNames.empty())
            {
                NotifyListeners(notifyListeners, fieldNames);
            }
        }
    }
    else
    {
        if (!impl->cachedValues.empty())
        {
            impl->cachedValues.clear();
            NotifyListeners(notifyListeners);
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

void DataWrapper::NotifyListeners(bool sendNotify, const Set<String>& fields)
{
    if (sendNotify == false)
        return;

    DVASSERT(impl != nullptr);
    auto fn = std::bind(&DataListener::OnDataChanged, std::placeholders::_1, std::cref(*this), std::cref(fields));
    std::for_each(impl->listeners.begin(), impl->listeners.end(), fn);
}

Reflection DataWrapper::GetData() const
{
    DVASSERT(HasData());
    return impl->dataAccessor(*impl->activeContext);
}
} // namespace TArc
} // namespace DAVA
