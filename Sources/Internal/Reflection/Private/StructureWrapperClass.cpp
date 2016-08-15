#include "Base/Platform.h"
#ifndef __DAVAENGINE_ANDROID__

#include "Reflection/Private/StructureWrapperClass.h"
#include "Reflection/Public/ReflectedType.h"
#include "Reflection/Public/ReflectedObject.h"

namespace DAVA
{
StructureWrapperClass::StructureWrapperClass(const Type* type)
    : thisType(type)
{
}

bool StructureWrapperClass::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    bool ret = !fields.empty();

    if (!ret)
    {
        InitBaseClasses();

        for (ClassBase& base : bases)
        {
            const StructureWrapper* baseSW = base.refType->structureWrapper.get();
            ret |= baseSW->HasFields(base.GetBaseObject(object), vw);

            if (ret)
            {
                break;
            }
        }
    }

    return ret;
}

Reflection::Field StructureWrapperClass::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    Reflection::Field ret;

    String name;

    if (key.CanGet<String>())
    {
        name = key.Get<String>();
    }
    else if (key.CanGet<const char*>())
    {
        name = key.Get<const char*>();
    }

    if (!name.empty())
    {
        auto end = fields.end();
        auto it = fields.begin();

        for (; it != end; ++it)
        {
            if (it->first == name)
            {
                ret.key = key;
                ret.ref = Reflection(vw->GetValueObject(object), it->second.vw.get(), it->second.type, it->second.meta.get());

                break;
            }
        }

        if (it == end)
        {
            InitBaseClasses();

            for (ClassBase& base : bases)
            {
                if (nullptr != base.refType)
                {
                    const StructureWrapper* baseSW = base.refType->structureWrapper.get();

                    ret = baseSW->GetField(base.GetBaseObject(object), vw, key);
                    if (ret.ref.IsValid())
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

Vector<Reflection::Field> StructureWrapperClass::GetFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Field> ret;

    InitBaseClasses();

    for (auto& baseIt : bases)
    {
        const ReflectedType* baseRefType = baseIt.refType;
        if (nullptr != baseRefType)
        {
            const StructureWrapper* baseSW = baseRefType->structureWrapper.get();
            auto baseFields = baseSW->GetFields(baseIt.GetBaseObject(object), vw);

            std::move(baseFields.begin(), baseFields.end(), std::inserter(ret, ret.end()));
        }
    }

    for (auto& it : fields)
    {
        Reflection::Field rf;

        rf.key = it.first;
        rf.ref = Reflection(vw->GetValueObject(object), it.second.vw.get(), it.second.type, it.second.meta.get());

        ret.emplace_back(std::move(rf));
    }

    return ret;
}

bool StructureWrapperClass::HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    bool ret = !methods.empty();

    if (!ret)
    {
        InitBaseClasses();

        for (ClassBase& base : bases)
        {
            const StructureWrapper* baseSW = base.refType->structureWrapper.get();
            ret |= baseSW->HasMethods(base.GetBaseObject(object), vw);

            if (ret)
            {
                break;
            }
        }
    }

    return ret;
}

Reflection::Method StructureWrapperClass::GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    Reflection::Method ret;

    if (key.CanCast<String>())
    {
        const String& name = key.Cast<String>();

        auto end = methods.end();
        auto it = methods.begin();

        for (; it != end; ++it)
        {
            if (it->first == name)
            {
                ret = { it->first, it->second };
                break;
            }
        }

        if (it == end)
        {
            InitBaseClasses();

            for (ClassBase& base : bases)
            {
                if (nullptr != base.refType)
                {
                    const StructureWrapper* baseSW = base.refType->structureWrapper.get();

                    ret = baseSW->GetMethod(base.GetBaseObject(object), vw, key);
                    if (ret.fn.IsValid())
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

Vector<Reflection::Method> StructureWrapperClass::GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Method> ret;

    InitBaseClasses();

    for (auto& baseIt : bases)
    {
        const ReflectedType* baseRefType = baseIt.refType;
        if (nullptr != baseRefType)
        {
            const StructureWrapper* baseSW = baseRefType->structureWrapper.get();
            auto baseMethods = baseSW->GetMethods(baseIt.GetBaseObject(object), vw);

            std::move(baseMethods.begin(), baseMethods.end(), std::inserter(ret, ret.end()));
        }
    }

    for (auto& it : methods)
    {
        ret.emplace_back(Reflection::Method{ it.first, it.second });
    }

    return ret;
}

void StructureWrapperClass::InitBaseClasses() const
{
    if (!basesInitialized)
    {
        auto bases_ = thisType->GetBaseTypes();

        bases.reserve(bases_.size());
        for (auto bc : bases_)
        {
            ClassBase classBase;
            classBase.type = bc.first;
            classBase.castToBaseOP = bc.second;
            classBase.refType = ReflectedType::GetByType(bc.first);

            bases.emplace_back(std::move(classBase));
        }

        basesInitialized = true;
    }
}

ReflectedObject StructureWrapperClass::ClassBase::GetBaseObject(const ReflectedObject& object) const
{
    void* derVoidPtr = object.GetVoidPtr();
    void* baseVoidPtr = (*castToBaseOP)(derVoidPtr);

    return ReflectedObject(baseVoidPtr, type->Pointer());
}

} // namespace DAVA

#endif
