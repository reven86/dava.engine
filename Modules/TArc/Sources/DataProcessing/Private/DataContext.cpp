#include "DataProcessing/DataContext.h"

#include "Reflection/Reflection.h"
#include "Utils/StringFormat.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
DataContext::DataContext(DataContext* parentContext_)
    : parentContext(parentContext_)
{
}

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
    const ReflectedType* type = ReflectedType::GetByPointer(node.get());
    DVASSERT(dataMap.count(type) == 0);
    dataMap.emplace(std::make_pair(type, node.release()));
}

bool DataContext::HasData(const ReflectedType* type) const
{
    bool result = dataMap.count(type) > 0;
    if (result == false && parentContext != nullptr)
    {
        result = parentContext->HasData(type);
    }

    return result;
}

DataNode& DataContext::GetData(const ReflectedType* type) const
{
    auto iter = dataMap.find(type);
    if (iter == dataMap.end())
    {
        if (parentContext != nullptr)
        {
            return parentContext->GetData(type);
        }
        throw std::runtime_error(Format("Data with type %s doesn't exist", type->GetPermanentName().c_str()));
    }

    return *iter->second;
}

void DataContext::DeleteData(const ReflectedType* type)
{
    auto iter = dataMap.find(type);
    if (iter == dataMap.end())
    {
        if (parentContext != nullptr)
        {
            parentContext->DeleteData(type);
        }

        return;
    }

    delete iter->second;
    dataMap.erase(iter);
}

DataContext::ContextID DataContext::GetID() const
{
    return reinterpret_cast<ContextID>(this);
}
} // namespace TArc
} // namespace DAVA
