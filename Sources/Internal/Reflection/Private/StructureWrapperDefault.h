#pragma once
#include "Reflection/WrappersRuntime.h"

namespace DAVA
{
class StructureWrapperDefault : public StructureWrapper
{
public:
    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        return Reflection();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw) const override
    {
        return Vector<Reflection::Field>();
    }

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return AnyFn();
    }

    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return Vector<Reflection::Method>();
    }

    bool CanAdd(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    bool CanInsert(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    bool CanRemove(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    bool CanCreateValue(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    Any CreateValue(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return Any();
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return false;
    }
};

template <typename T>
struct StructureWrapperCreator
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperDefault();
    }
};

} // namespace DAVA
