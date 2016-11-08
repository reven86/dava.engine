#pragma once

#include "Base/Any.h"
#include "Base/AnyFn.h"

namespace DAVA
{
class ReflectedObject;

class ValueWrapper
{
public:
    ValueWrapper() = default;
    ValueWrapper(const ValueWrapper&) = delete;
    virtual ~ValueWrapper() = default;

    virtual bool IsReadonly() const = 0;
    virtual const RttiType* GetType() const = 0;

    virtual Any GetValue(const ReflectedObject& object) const = 0;
    virtual bool SetValue(const ReflectedObject& object, const Any& value) const = 0;

    virtual ReflectedObject GetValueObject(const ReflectedObject& object) const = 0;
};

class MethodWrapper
{
public:
    AnyFn method;
};

class EnumWrapper
{
    // TODO: implement
};

class CtorWrapper
{
public:
    CtorWrapper() = default;
    CtorWrapper(const CtorWrapper&) = delete;
    virtual ~CtorWrapper() = default;

    virtual const AnyFn::Params& GetInvokeParams() const = 0;

    virtual Any Create() const = 0;
    virtual Any Create(const Any&) const = 0;
    virtual Any Create(const Any&, const Any&) const = 0;
    virtual Any Create(const Any&, const Any&, const Any&) const = 0;
    virtual Any Create(const Any&, const Any&, const Any&, const Any&) const = 0;
    virtual Any Create(const Any&, const Any&, const Any&, const Any&, const Any&) const = 0;
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
} // namespace DAVA
