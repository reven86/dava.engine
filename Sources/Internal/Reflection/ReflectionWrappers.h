#pragma once

#if !defined(__DAVAENGINE_ANDROID__)

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedObject.h"

namespace DAVA
{
namespace Ref
{
struct Field
{
    Any key;
    Reflection valueRef;
};

using FieldsList = Vector<Field>;

} // namespace Ref

class ValueWrapper
{
public:
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
    virtual ~StructureWrapper() = default;

    virtual bool IsDynamic() const = 0;

    virtual bool CanAdd() const = 0;
    virtual bool CanInsert() const = 0;
    virtual bool CanRemove() const = 0;

    virtual Ref::Field GetField(const ReflectedObject& object, const Any& key) const = 0;
    virtual Ref::FieldsList GetFields(const ReflectedObject& object) const = 0;

    virtual bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const = 0;
    virtual bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const = 0;
    virtual bool RemoveField(const ReflectedObject& object, const Any& key) const = 0;
};

class CtorWrapper
{
public:
    virtual ~CtorWrapper() = default;

    virtual const Ref::ParamsList& GetParamsList() const = 0;

    virtual Any Create() const = 0;
    virtual Any Create(const Any&) const = 0;
    virtual Any Create(const Any&, const Any&) const = 0;
    virtual Any Create(const Any&, const Any&, const Any&) const = 0;
};

class DtorWrapper
{
public:
    virtual ~DtorWrapper() = default;

    virtual void Destroy(Any&& value) const = 0;
    virtual void Destroy(ReflectedObject&& object) const = 0;
};

class MethodWrapper
{
public:
    virtual ~MethodWrapper() = default;
};

} // namespace DAVA

#endif