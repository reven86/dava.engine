#include "Reflection/Reflection.h"

namespace DAVA
{
List<std::unique_ptr<ReflectedType>> ReflectedTypeDB::customReflectedTypes;
UnorderedMap<const Type*, ReflectedType*> ReflectedTypeDB::typeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::typeNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::permanentNameToReflectedTypeMap;

void ReflectedTypeDB::RegisterDBType(ReflectedType* reflectedType, const Type* type, StructureWrapper* sw)
{
    reflectedType->structureWrapper.reset(sw);

    typeToReflectedTypeMap[type] = reflectedType;
    typeNameToReflectedTypeMap[String(type->GetName())] = reflectedType;
}

const ReflectedType* ReflectedTypeDB::GetByType(const Type* type)
{
    const ReflectedType* ret = nullptr;

    auto it = typeToReflectedTypeMap.find(type);
    if (it != typeToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedTypeDB::GetByTypeName(const String& rttiName)
{
    const ReflectedType* ret = nullptr;

    auto it = typeNameToReflectedTypeMap.find(rttiName);
    if (it != typeNameToReflectedTypeMap.end())
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

    DVASSERT(typeToReflectedTypeMap.count(type) == 0 && "ReflectedType with specified RttiType already exists");
    DVASSERT(typeNameToReflectedTypeMap.count(rttiName) == 0 && "ReflectedType with specified RttiType::name already exists");

    typeToReflectedTypeMap[type] = ret;
    typeNameToReflectedTypeMap[rttiName] = ret;

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
