#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
List<std::unique_ptr<ReflectedType>> ReflectedTypeDB::customReflectedTypes;
UnorderedMap<const RttiType*, ReflectedType*> ReflectedTypeDB::rttiTypeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::rttiNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::permanentNameToReflectedTypeMap;

#ifdef __REFLECTION_FEATURE__

void ReflectedTypeDBB : SetPermanentName(const String& name) const
{
    ReflectedType* rt = const_cast<ReflectedType*>(this);

    assert(permanentName.empty() && "Name is already set");
    assert(permanentNameToReflectedTypeMap.count(name) == 0 && "Permanent name alredy in use");

    rt->permanentName = name;
    rt->permanentNameToReflectedTypeMap[permanentName] = rt;
}
#endif

const ReflectedType* ReflectedTypeDB::GetByRttiType(const RttiType* rttiType)
{
    const ReflectedType* ret = nullptr;

    auto it = rttiTypeToReflectedTypeMap.find(rttiType);
    if (it != rttiTypeToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedTypeDB::GetByRttiName(const String& rttiName)
{
    const ReflectedType* ret = nullptr;

    auto it = rttiNameToReflectedTypeMap.find(rttiName);
    if (it != rttiNameToReflectedTypeMap.end())
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

ReflectedType* ReflectedTypeDB::Create(const RttiType* rttiType, const String& permanentName)
{
    customReflectedTypes.emplace_back(new ReflectedType(rttiType));
    ReflectedType* ret = customReflectedTypes.back().get();

    String rttiName(rttiType->GetName());

    DVASSERT(rttiTypeToReflectedTypeMap.count(rttiType) == 0 && "ReflectedType with specified RttiType already exists");
    DVASSERT(rttiNameToReflectedTypeMap.count(rttiName) == 0 && "ReflectedType with specified RttiType::name already exists");

    rttiTypeToReflectedTypeMap[rttiType] = ret;
    rttiNameToReflectedTypeMap[rttiName] = ret;

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
