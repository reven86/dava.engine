#include "DataProcessing/DataWrapper.h"

#include "Logger/Logger.h"
#include "Debug/DVAssert.h"

namespace tarc
{

struct DataWrapper::Impl
{
    DataContext* activeContext = nullptr;
    DataAccessor dataAccessor;
    DAVA::Vector<DAVA::Any> cachedValues;

    DAVA::Set<DataListener*> listeners;
};

DataWrapper::DataWrapper(const DAVA::ReflectedType* type)
    : DataWrapper(DAVA::Bind(&DataWrapper::GetDataDefault, std::placeholders::_1, type))
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

    DAVA::Reflection reflection;
    try
    {
        reflection = impl->dataAccessor(*impl->activeContext);
    }
    catch (const std::runtime_error& e)
    {
        DAVA::Logger::Error(e.what());
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

void DataWrapper::Sync(bool notifyListeners)
{
    DVASSERT(impl != nullptr);
    if (HasData())
    {
        DAVA::Reflection reflection = GetData();
        DAVA::Vector<DAVA::Reflection::Field> fields = reflection.GetFields();

        if (impl->cachedValues.size() != fields.size())
        {
            impl->cachedValues.clear();
            for (const DAVA::Reflection::Field& field : fields)
            {
                impl->cachedValues.push_back(field.ref.GetValue());
            }
            NotifyListeners(notifyListeners);
        }
        else
        {
            DAVA::Set<DAVA::String> fieldNames;
            std::function<void(const DAVA::String&)> nameInserter;
            if (notifyListeners)
            {
                nameInserter = [&fieldNames](const DAVA::String& name) { fieldNames.insert(name); };
            }
            else
            {
                nameInserter = [](const DAVA::String&) {};
            }

            for (size_t i = 0; i < fields.size(); ++i)
            {
                const DAVA::Reflection::Field& field = fields[i];
                DAVA::Any newValue = field.ref.GetValue();
                if (impl->cachedValues[i] != newValue)
                {
                    impl->cachedValues[i] = newValue;
                    nameInserter(field.key.Cast<DAVA::String>());
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

void DataWrapper::NotifyListeners(bool sendNotify, const DAVA::Set<DAVA::String>& fields)
{
    if (sendNotify == false)
        return;

    DVASSERT(impl != nullptr);
    auto fn = std::bind(&DataListener::OnDataChanged, std::placeholders::_1, std::cref(*this), std::cref(fields));
    std::for_each(impl->listeners.begin(), impl->listeners.end(), fn);
}

DAVA::Reflection DataWrapper::GetData() const
{
    DVASSERT(HasData());
    return impl->dataAccessor(*impl->activeContext);
}

DAVA::Reflection DataWrapper::GetDataDefault(const DataContext& context, const DAVA::ReflectedType* type)
{
    if (!context.HasData(type))
    {
        return DAVA::Reflection();
    }

    return DAVA::Reflection::Create(&context.GetData(type)).ref;
}

DataListener::~DataListener()
{
    holder.RemoveListener(this);
}

void DataListener::InitListener(const DataWrapper& wrapper)
{
    holder = wrapper;
}

}