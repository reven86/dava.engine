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

    bool HasFields(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(obj).Deref();
        const StructureWrapper* sw = GetInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->HasFields(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::HasFields(obj, vw);
    }

    Reflection::Field GetField(const ReflectedObject& obj, const PropertieWrapper* vw, const Any& key) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(obj).Deref();

        const StructureWrapper* sw = GetInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetField(derefObj, &ptrVW, key);
        }

        return StructureWrapperDefault::GetField(obj, vw, key);
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(obj).Deref();

        const StructureWrapper* sw = GetInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetFields(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::GetFields(obj, vw);
    }

    bool HasMethods(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(obj).Deref();

        const StructureWrapper* sw = GetInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->HasMethods(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::HasMethods(obj, vw);
    }

    Reflection::Method GetMethod(const ReflectedObject& obj, const PropertieWrapper* vw, const Any& key) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(obj).Deref();

        const StructureWrapper* sw = GetInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetMethod(derefObj, &ptrVW, key);
        }

        return StructureWrapperDefault::GetMethod(obj, vw, key);
    }

    Vector<Reflection::Method> GetMethods(const ReflectedObject& obj, const PropertieWrapper* vw) const override
    {
        ReflectedObject derefObj = vw->GetPropertieObject(obj).Deref();

        const StructureWrapper* sw = GetInternalWrapper(derefObj);
        if (nullptr != sw)
        {
            return sw->GetMethods(derefObj, &ptrVW);
        }

        return StructureWrapperDefault::GetMethods(obj, vw);
    }

protected:
    ValueWrapperDefault<T*> ptrVW;

    const StructureWrapper* GetInternalWrapper(const ReflectedObject& derefObj) const
    {
        const StructureWrapper* sw = nullptr;

        if (derefObj.IsValid())
        {
            T* ptr = derefObj.GetPtr<T>();
            sw = ReflectedType::GetByPointer(ptr)->structureWrapper.get();
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
