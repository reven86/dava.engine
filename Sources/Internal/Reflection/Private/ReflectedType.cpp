#include "Reflection/Public/ReflectedType.h"
#include "Reflection/Public/Wrappers.h"

namespace DAVA
{
UnorderedMap<const Type*, ReflectedType*> ReflectedType::typeToReflectedTypeMap;
UnorderedMap<String, ReflectedType*> ReflectedType::nameToReflectedTypeMap;

const Type* ReflectedType::GetType() const
{
    return type;
}

const String& ReflectedType::GetName() const
{
    return name;
}

void ReflectedType::SetName(const String& name_) const
{
    assert(name.empty() && "Name is already set");
    name = name_;
}

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

const DtorWrapper *ReflectedType::GetDtor() const
{
    return nullptr;
}

const ChildrenWrapper *ReflectedType::GetChildren() const
{
    return nullptr;
}

const ChildrenEditorWrapper *ReflectedType::GetChildrenEditor() const
{
    return nullptr;
}

const MethodWrapper *ReflectedType::GetMethod(const String &name, const Vector<const Type *> &params) const
{
    return nullptr;
}

Vector<const MethodWrapper *> ReflectedType::GetMethods(const String &name) const
{
    return Vector<const MethodWrapper *>();
}

Vector<const MethodWrapper *> ReflectedType::GetMethods() const
{
    return Vector<const MethodWrapper *>();
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

const ReflectedType* ReflectedType::GetByName(const String& name)
{
    assert(false && "not implemented");
    return nullptr;
}

} // namespace DAVA
