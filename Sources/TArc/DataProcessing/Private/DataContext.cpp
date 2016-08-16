#include "DataProcessing/DataContext.h"

#include "Reflection/Reflection.h"
#include "Utils/StringFormat.h"
#include "Debug/DVAssert.h"

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
    const DAVA::ReflectedType* type = DAVA::ReflectedType::GetByPointer(node.get());
    DVASSERT(dataMap.count(type) == 0);
    dataMap.emplace(std::make_pair(type, node.release()));
}

bool DataContext::HasData(const DAVA::ReflectedType* type) const
{
    return dataMap.count(type) > 0;
}

DataNode& DataContext::GetData(const DAVA::ReflectedType* type) const
{
    auto iter = dataMap.find(type);
    if (iter == dataMap.end())
    {
        throw std::runtime_error(DAVA::Format("Data with type %s doesn't exist", type->GetPermanentName()));
    }

    return *iter->second;
}

void DataContext::DeleteData(const DAVA::ReflectedType* type)
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
