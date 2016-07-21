#include "DataWrapper.h"

namespace tarc
{

DataWrapper::Listener::~Listener()
{
    if (holder != 0)
    {
        holder->RemoveListener(this);
    }
}

void DataWrapper::Listener::InitListener(DataWrapper& wrapper)
{
    holder = DAVA::RefPtr<DataWrapper>::ConstructWithRetain(&wrapper);
}

DataWrapper::DataWrapper(std::unique_ptr<DataNode>&& data, DataContext& context_, bool listenRecursive_)
    : context(context_)
    , listenRecursive(listenRecursive_)
{
    context.CreateData(std::move(data));
    
    // TODO get type of data.
    //dataAccessor(DAVA::Bind(&DataWrapper::GetDataDefault, std::placeholders::_1, typeof(data)))
}

DataWrapper::DataWrapper(const DataAccessor& accessor, DataContext& context, bool listenRecursive_)
    : context(context)
    , dataAccessor(accessor)
    , listenRecursive(listenRecursive_)
{
}

bool DataWrapper::HasData() const
{
    DAVA::Reflection reflection;
    try
    {
        reflection = dataAccessor(context);
    }
    catch (const std::runtime_error& e)
    {
        DAVA::Logger::Error(e.what());
        return false;
    }
    
    return reflection.IsValid();
}

void DataWrapper::AddListener(Listener* listener)
{
    DVASSERT(listener != nullptr);
    listener->InitListener(*this);
    listeners.insert(listener);
}

void DataWrapper::RemoveListener(Listener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.erase(listener);
}

void DataWrapper::Sync()
{
    if (HasData())
    {
        DAVA::Vector<DAVA::Any> newValues;
        SyncImpl(dataAccessor(context), newValues);

        if (cachedValues.size() != newValues.size() ||
            std::equal(cachedValues.begin(), cachedValues.end(), newValues.begin()) == false)
        {
            cachedValues = std::move(newValues);
            NotifyListeners();
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
        cachedValues.push_back(field.valueRef.GetValue());
    }

    if (listenRecursive)
    {
        for (const DAVA::Ref::Field& field : fields)
        {
            SyncImpl(field.valueRef, values);
        }
    }
}

void DataWrapper::NotifyListeners()
{
    std::for_each(listeners.begin(), listeners.end(), std::bind(&Listener::OnDataChanged, std::placeholders::_1, std::cref(*this)));
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

}