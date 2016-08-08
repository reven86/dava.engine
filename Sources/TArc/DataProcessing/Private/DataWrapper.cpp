#include "DataProcessing/DataWrapper.h"

namespace tarc
{

struct DataWrapper::Impl
{
    DataContext* activeContext = nullptr;
    bool listenRecursive = false;
    DataAccessor dataAccessor;
    DAVA::Vector<DAVA::Any> cachedValues;

    DAVA::Set<DataListener*> listeners;
};

DataWrapper::DataWrapper(const DAVA::Type* type, bool listenRecursive)
    : DataWrapper(DAVA::Bind(&DataWrapper::GetDataDefault, std::placeholders::_1, type), listenRecursive)
{
}

DataWrapper::DataWrapper(const DataAccessor& accessor, bool listenRecursive_)
    : impl(new Impl())
{
    impl->dataAccessor = accessor;
    impl->listenRecursive = listenRecursive_;
}

void DataWrapper::SetContext(DataContext* context)
{
    impl->activeContext = context;
}

bool DataWrapper::HasData() const
{
    if (impl->activeContext == nullptr)
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
    DVASSERT(listener != nullptr);
    listener->InitListener(*this);
    impl->listeners.insert(listener);
}

void DataWrapper::RemoveListener(DataListener* listener)
{
    DVASSERT(listener != nullptr);
    impl->listeners.erase(listener);
}

void DataWrapper::Sync()
{
    if (HasData())
    {
        DAVA::Vector<DAVA::Any> newValues;
        SyncImpl(impl->dataAccessor(*impl->activeContext), newValues);

        if (impl->cachedValues.size() != newValues.size() ||
            std::equal(impl->cachedValues.begin(), impl->cachedValues.end(), newValues.begin()) == false)
        {
            impl->cachedValues = std::move(newValues);
            NotifyListeners();
        }
    }
    else
    {
        if (!impl->cachedValues.empty())
        {
            NotifyListeners();
            impl->cachedValues.clear();
        }
    }
}

void DataWrapper::SyncImpl(const DAVA::Reflection& reflection, DAVA::Vector<DAVA::Any>& values)
{
    const DAVA::StructureWrapper* wrapper = reflection.GetStructure();
    DAVA::Ref::FieldsList fields = wrapper->GetFields(reflection.GetValueObject());
    values.reserve(values.size() + fields.size());
    for (const DAVA::Ref::Field& field : fields)
    {
        impl->cachedValues.push_back(field.valueRef.GetValue());
    }

    if (impl->listenRecursive)
    {
        for (const DAVA::Ref::Field& field : fields)
        {
            SyncImpl(field.valueRef, values);
        }
    }
}

void DataWrapper::NotifyListeners()
{
    std::for_each(impl->listeners.begin(), impl->listeners.end(), std::bind(&DataListener::OnDataChanged, std::placeholders::_1, std::cref(*this)));
}

DAVA::Reflection DataWrapper::GetDataDefault(DataContext& context, const DAVA::Type* type)
{
    if (!context.HasData(type))
    {
        return DAVA::Reflection();
    }

    DataNode* node = &context.GetData(type);
    return DAVA::Reflection::Reflect(&node);
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