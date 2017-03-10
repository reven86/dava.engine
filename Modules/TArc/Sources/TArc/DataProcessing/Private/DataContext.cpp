#include "TArc/DataProcessing/DataContext.h"
#include "TArc/Utils/CommonFieldNames.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Utils/StringFormat.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{
DAVA_REFLECTION_IMPL(DataContext)
{
    ReflectionRegistrator<DataContext>::Begin()
    .Field(ContextIDFieldName, &DataContext::GetID, nullptr)
    .End();
}

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
    const ReflectedType* type = ReflectedTypeDB::GetByPointer(node.get());
    DVASSERT(dataMap.count(type) == 0);
    dataMap.emplace(std::make_pair(type, node.release()));
}

DataNode* DataContext::GetData(const ReflectedType* type) const
{
    auto iter = dataMap.find(type);
    if (iter == dataMap.end())
    {
        if (parentContext != nullptr)
        {
            return parentContext->GetData(type);
        }
        return nullptr;
    }

    return iter->second;
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
