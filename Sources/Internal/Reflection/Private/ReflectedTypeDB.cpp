#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
UnorderedMap<const RttiType*, ReflectedType*> ReflectedTypeDB::rttiTypeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::rttiNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedTypeDB::permanentNameToReflectedTypeMap;

void ReflectedTypeDBB : SetPermanentName(const String& name) const
{
    ReflectedType* rt = const_cast<ReflectedType*>(this);

    assert(permanentName.empty() && "Name is already set");
    assert(permanentNameToReflectedTypeMap.count(name) == 0 && "Permanent name alredy in use");

    rt->permanentName = name;
    rt->permanentNameToReflectedTypeMap[permanentName] = rt;
}

#ifdef __REFLECTION_FEATURE__
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

const ReflectedType* ReflectedType::GetByRttiType(const RttiType* type)
{
    const ReflectedType* ret = nullptr;

    auto it = rttiTypeToReflectedTypeMap.find(type);
    if (it != rttiTypeToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedType::GetByRttiName(const String& name)
{
    const ReflectedType* ret = nullptr;

    auto it = rttiNameToReflectedTypeMap.find(name);
    if (it != rttiNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

const ReflectedType* ReflectedType::GetByPermanentName(const String& name)
{
    const ReflectedType* ret = nullptr;

    auto it = permanentNameToReflectedTypeMap.find(name);
    if (it != permanentNameToReflectedTypeMap.end())
    {
        ret = it->second;
    }

    return ret;
}

} // namespace DAVA
