#pragma once

#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/RttiType.h"
#include "Reflection/ReflectedObject.h"

namespace DAVA
{
class ReflectedObject;
class ReflectedMeta;
class ReflectedType;

class PropertieWrapper
{
public:
    PropertieWrapper() = default;
    PropertieWrapper(const PropertieWrapper&) = delete;
    virtual ~PropertieWrapper() = default;

    virtual bool IsReadonly() const = 0;
    virtual const RttiType* GetType() const = 0;

    virtual Any GetValue(const ReflectedObject& object) const = 0;
    virtual bool SetValue(const ReflectedObject& object, const Any& value) const = 0;

    virtual ReflectedObject GetPropertieObject(const ReflectedObject& object) const = 0;
};

class MethodWrapper
{
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

class StructureWrapper
{
public:
    StructureWrapper() = default;
    StructureWrapper(const StructureWrapper&) = delete;
    virtual ~StructureWrapper() = default;

    virtual bool HasFields(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;
    virtual Reflection GetField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Field> GetFields(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;

    virtual bool HasMethods(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;
    virtual AnyFn GetMethod(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;

    virtual bool CanAdd(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;
    virtual bool CanInsert(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;
    virtual bool CanRemove(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;
    virtual bool CanCreateValue(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;

    virtual Any CreateValue(const ReflectedObject& object, const PropertieWrapper* vw) const = 0;
    virtual bool AddField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key, const Any& value) const = 0;
    virtual bool InsertField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const = 0;
    virtual bool RemoveField(const ReflectedObject& object, const PropertieWrapper* vw, const Any& key) const = 0;
};

template <typename T>
struct StructureWrapperCreator;

} // namespace DAVA
