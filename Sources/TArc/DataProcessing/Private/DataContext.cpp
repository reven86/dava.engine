#include "DataProcessing/DataContext.h"

#include "Reflection/Reflection.h"

namespace tarc
{

DataContext::~DataContext()
{
    for (auto& data : dataMap)
    {
        delete data.second;
    }

    dataMap.clear();
}

void DataContext::CreateData(std::unique_ptr<DataNode>&& node)
{
    DataNode* nodePointer = node.get();
    DAVA::Reflection reflect = DAVA::Reflection::Reflect(&nodePointer);
    const DAVA::Type* type = reflect.GetValueType();
    DVASSERT(dataMap.count(type) == 0);
    dataMap.emplace(std::make_pair(type, node.release()));
}

bool DataContext::HasData(const DAVA::Type* type)
{
    return dataMap.count(type) > 0;
}

DataNode& DataContext::GetData(const DAVA::Type* type)
{
    auto iter = dataMap.find(type);
    if (iter == dataMap.end())
    {
        throw std::runtime_error(DAVA::Format("Data with type %s doesn't exist", type->GetName()));
    }

    return *iter->second;
}

void DataContext::DeleteData(const DAVA::Type* type)
{
    auto iter = dataMap.find(type);
    if (iter == dataMap.end())
    {
        return;
    }

    delete iter->second;
    dataMap.erase(iter);
}

DataContext::ContextID DataContext::GetID() const
{
    return reinterpret_cast<ContextID>(this);
}

}
