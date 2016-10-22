#pragma once

#include "Base/Type.h"
#include "Base/Any.h"
#include "Base/AnyFn.h"

#define __DAVA_Reflection_Definition_Only__
#include "Reflection/Reflection.h"
#undef __DAVA_Reflection_Definition_Only__

namespace DAVA
{
class ValueWrapper;
class StructureWrapper;
class StructureEditorWrapper;
class ReflectedObject;

class ValueWrapper
{
public:
    ValueWrapper() = default;
    ValueWrapper(const ValueWrapper&) = delete;
    virtual ~ValueWrapper() = default;

    virtual bool IsReadonly() const = 0;
    virtual const Type* GetType() const = 0;

    virtual Any GetValue(const ReflectedObject& object) const = 0;
    virtual bool SetValue(const ReflectedObject& object, const Any& value) const = 0;

    virtual ReflectedObject GetValueObject(const ReflectedObject& object) const = 0;
};

class StructureWrapper
{
public:
    StructureWrapper() = default;
    StructureWrapper(const StructureWrapper&) = delete;
    virtual ~StructureWrapper() = default;

    virtual bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual Reflection::Field GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual Reflection::Method GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
};

class StructureEditorWrapper
{
public:
    StructureEditorWrapper() = default;
    StructureEditorWrapper(const StructureEditorWrapper&) = delete;
    virtual ~StructureEditorWrapper() = default;

    virtual bool CanAdd(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool CanInsert(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool CanRemove(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool CanCreateValue(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual Any CreateValue(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const = 0;
    virtual bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const = 0;
    virtual bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
};

class DtorWrapper
{
public:
    DtorWrapper() = default;
    DtorWrapper(const DtorWrapper&) = delete;
    virtual ~DtorWrapper() = default;

    virtual void Destroy(Any&& value) const = 0;
    virtual void Destroy(ReflectedObject&& object) const = 0;
};

class CtorWrapper
{
public:
    enum class Policy
    {
        ByValue,
        ByPointer
    };

    CtorWrapper() = default;
    CtorWrapper(const CtorWrapper&) = delete;
    virtual ~CtorWrapper() = default;

    virtual const AnyFn::Params& GetInvokeParams() const = 0;

    virtual Any Create(Policy) const = 0;
    virtual Any Create(Policy, const Any&) const = 0;
    virtual Any Create(Policy, const Any&, const Any&) const = 0;
    virtual Any Create(Policy, const Any&, const Any&, const Any&) const = 0;
    virtual Any Create(Policy, const Any&, const Any&, const Any&, const Any&) const = 0;
    virtual Any Create(Policy, const Any&, const Any&, const Any&, const Any&, const Any&) const = 0;
};

template <typename T>
struct StructureWrapperCreator;

template <typename T>
struct StructureEditorWrapperCreator;

} // namespace DAVA
