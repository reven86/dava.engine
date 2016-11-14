#pragma once
#include "Reflection/Wrappers.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/StructureWrapperDefault.h"

namespace DAVA
{
template <typename T>
class StructureWrapperPtr final : public StructureWrapperDefault
{
public:
    StructureWrapperPtr() = default;

    bool HasFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->HasFields(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::HasFields(obj, vw);
    }

    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetField(derefObj, &ptrVW, key);
        }

        return StructureWrapperDefault::GetField(obj, vw, key);
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetFields(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::GetFields(obj, vw);
    }

    bool HasMethods(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->HasMethods(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::HasMethods(obj, vw);
    }

    AnyFn GetMethod(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetMethod(derefObj, &ptrVW, key);
        }

        return StructureWrapperDefault::GetMethod(obj, vw, key);
    }

    Vector<Reflection::Method> GetMethods(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetMethods(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::GetMethods(obj, vw);
    }

    const Reflection::FieldsCaps& GetFieldsCaps(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(obj).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetFieldsCaps(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::GetFieldsCaps(obj, vw);
    }

    Any CreateValue(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(object).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->CreateValue(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::CreateValue(object, vw);
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(object).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->AddField(derefObj, &ptrVW, key, value);
        }

        return StructureWrapperDefault::AddField(object, vw, key, value);
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(object).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->InsertField(derefObj, &ptrVW, beforeKey, key, value);
        }

        return StructureWrapperDefault::InsertField(object, vw, beforeKey, key, value);
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        ReflectedObject derefObj = vw->GetValueObject(object).Deref();
        const StructureWrapper* sw = AppyInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->RemoveField(derefObj, &ptrVW, key);
        }

        return StructureWrapperDefault::RemoveField(object, vw, key);
    }

protected:
    ValueWrapperDefault<T*> ptrVW;

    const StructureWrapper* AppyInternalWrapper(ReflectedObject& derefObj) const
    {
        const StructureWrapper* sw = nullptr;

        if (derefObj.IsValid())
        {
            T* ptr = derefObj.GetPtr<T>();

            const ReflectedType* reflectedType = ReflectedTypeDB::GetByPointer(ptr);

            derefObj = ReflectedObject(ptr, reflectedType->GetRttiType()->Pointer());
            sw = reflectedType->GetStrucutreWrapper();
        }

        return sw;
    }
};

template <typename T>
struct StructureWrapperCreator<T*>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperPtr<T>();
    }
};

} // namespace DAVA
