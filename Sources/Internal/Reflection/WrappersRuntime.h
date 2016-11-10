#pragma once

#ifndef __DAVA_Reflection__
#error Don't include this file directly, instead use #include "Reflection/Reflection.h"
#else
namespace DAVA
{
class StructureWrapper
{
public:
    StructureWrapper() = default;
    StructureWrapper(const StructureWrapper&) = delete;
    virtual ~StructureWrapper() = default;

    virtual bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual bool CanAdd(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool CanInsert(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool CanRemove(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool CanCreateValue(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual Any CreateValue(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const = 0;
    virtual bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const = 0;
    virtual bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
};

template <typename T>
struct StructureWrapperCreator;

} // namespace DAVA

#endif
