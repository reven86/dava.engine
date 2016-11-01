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
