#include "Reflection/Reflection.h"

namespace DAVA
{
List<std::unique_ptr<ReflectedType>> ReflectedTypeDB::customReflectedTypes;
UnorderedMap<const Type*, ReflectedType*> ReflectedTypeDB::rtTypeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::rtTypeNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::permanentNameToReflectedTypeMap;

const ReflectedType* ReflectedTypeDB::GetByRtType(const Type* type)
{
    const ReflectedType* ret = nullptr;

    auto it = rtTypeToReflectedTypeMap.find(type);
    if (it != rtTypeToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedTypeDB::GetByRtTypeName(const String& rttiName)
{
    const ReflectedType* ret = nullptr;

    auto it = rtTypeNameToReflectedTypeMap.find(rttiName);
    if (it != rtTypeNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedTypeDB::GetByPermanentName(const String& permanentName)
{
    const ReflectedType* ret = nullptr;

    auto it = permanentNameToReflectedTypeMap.find(permanentName);
    if (it != permanentNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

ReflectedType* ReflectedTypeDB::CreateCustomType(const Type* type, const String& permanentName)
{
    customReflectedTypes.emplace_back(new ReflectedType(type));
    ReflectedType* ret = customReflectedTypes.back().get();

    String rttiName(type->GetName());

    DVASSERT(rtTypeToReflectedTypeMap.count(type) == 0 && "ReflectedType with specified RttiType already exists");
    DVASSERT(rtTypeNameToReflectedTypeMap.count(rttiName) == 0 && "ReflectedType with specified RttiType::name already exists");

    rtTypeToReflectedTypeMap[type] = ret;
    rtTypeNameToReflectedTypeMap[rttiName] = ret;

    RegisterPermanentName(ret, permanentName);

    return ret;
}

void ReflectedTypeDB::RegisterPermanentName(const ReflectedType* reflectedType, const String& permanentName)
{
    ReflectedType* rt = const_cast<ReflectedType*>(reflectedType);

    DVASSERT(rt != nullptr);
    DVASSERT(rt->permanentName.empty() && "Name is already set");
    DVASSERT(permanentNameToReflectedTypeMap.count(permanentName) == 0 && "Permanent name alredy in use");

    rt->permanentName = permanentName;
    permanentNameToReflectedTypeMap[permanentName] = rt;
}
} // namespace DAVA
