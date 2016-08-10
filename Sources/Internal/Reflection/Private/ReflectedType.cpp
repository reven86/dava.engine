#include "Reflection/Public/ReflectedType.h"
#include "Reflection/Public/Wrappers.h"

namespace DAVA
{
UnorderedMap<const Type*, ReflectedType*> ReflectedType::typeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedType::rttiNameToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedType::permanentNameToReflectedTypeMap;

void ReflectedType::SetPermanentName(const String& name) const
{
    ReflectedType* rt = const_cast<ReflectedType*>(this);

    assert(permanentName.empty() && "Name is already set");
    assert(permanentNameToReflectedTypeMap.count(name) == 0 && "Permanent name alredy in use");

    rt->permanentName = name;
    rt->permanentNameToReflectedTypeMap[permanentName] = rt;
}

// TODO:
//
// this is example access methods for ctorsWrappers
// should be reviewed and rewrited later
//
/*
const CtorWrapper *ReflectedType::GetCtor() const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : ctorWrappers)
    {
        if (it->GetParamsList().size() == 0)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

const CtorWrapper *ReflectedType::GetCtor(const Vector<const Type *> &params) const
{
    const CtorWrapper* ret = nullptr;

    for (auto& it : ctorWrappers)
    {
        if (it->GetParamsList() == params)
        {
            ret = it.get();
            break;
        }
    }

    return ret;
}

const Set<const CtorWrapper *> &ReflectedType::GetCtors() const
{
    Vector<const CtorWrapper*> ret;

    ret.reserve(ctorWrappers.size());
    for (auto& it : ctorWrappers)
    {
        ret.push_back(it.get());
    }

    return ret;
}
*/

const ReflectedType* ReflectedType::GetByType(const Type* type)
{
    const ReflectedType* ret = nullptr;

    auto it = typeToReflectedTypeMap.find(type);
    if (it != typeToReflectedTypeMap.end())
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
