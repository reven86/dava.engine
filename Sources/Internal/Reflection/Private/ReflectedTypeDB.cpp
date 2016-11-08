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

const CtorWrapper* ReflectedType::GetCtor(const AnyFn::Params& params) const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : ctorWrappers)
    {
        if (it->GetInvokeParams() == params)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

Vector<const CtorWrapper*> ReflectedType::GetCtors() const
{
    Vector<const CtorWrapper*> ret;

    ret.reserve(ctorWrappers.size());
    for (auto& it : ctorWrappers)
    {
        ret.push_back(it.get());
    }

    return ret;
}

const DtorWrapper* ReflectedType::GetDtor() const
{
    return dtorWrapper.get();
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

Vector<std::pair<const ReflectedType*, RttiInheritance::CastOP>> ReflectedTypeDB::GetRttiTypeHierarchy(const RttiType* rttiType)
{
    Vector<std::pair<const ReflectedType*, RttiInheritance::CastOP>> ret;

    ret.emplace_back(GetByRttiType(rttiType), nullptr);

    const RttiInheritance* inheritance = rttiType->GetInheritance();
    if (nullptr != inheritance)
    {
        auto& baseMap = inheritance->GetBaseTypes();
        for (auto& base : baseMap)
        {
            ret.emplace_back(GetByRttiType(base.first), base.second);
        }
    }

    return ret;
}

ReflectedType* ReflectedTypeDB::Create(const RttiType* rttiType, const String& permanentName)
{
    customReflectedTypes.emplace_back(new ReflectedType(rttiType));
    ReflectedType* ret = customReflectedTypes.back().get();

    String rttiName(rttiType->GetName());

    assert(rttiTypeToReflectedTypeMap.count(rttiType) == 0 && "ReflectedType with specified RttiType already exists");
    assert(rttiNameToReflectedTypeMap.count(rttiName) == 0 && "ReflectedType with specified RttiType::name already exists");

    rttiTypeToReflectedTypeMap[rttiType] = ret;
    rttiNameToReflectedTypeMap[rttiName] = ret;

    RegisterPermanentName(ret, permanentName);

    return ret;
}

void ReflectedTypeDB::RegisterPermanentName(const ReflectedType* reflectedType, const String& permanentName)
{
    ReflectedType* rt = const_cast<ReflectedType*>(reflectedType);

    assert(rt != nullptr);
    assert(rt->permanentName.empty() && "Name is already set");
    assert(permanentNameToReflectedTypeMap.count(permanentName) == 0 && "Permanent name alredy in use");

    rt->permanentName = permanentName;
    permanentNameToReflectedTypeMap[permanentName] = rt;
}

} // namespace DAVA
