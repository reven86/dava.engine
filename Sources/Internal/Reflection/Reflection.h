#pragma once

#include <sstream>
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/RtType.h"
#include "Base/RtTypeInheritance.h"

#include "Debug/DVAssert.h"

#include "Reflection/ReflectionTypes.h"
#include "Reflection/Private/ReflectedMeta.h"
#include "Reflection/Private/ReflectedType.h"
#include "Reflection/Private/ReflectedTypeDB.h"
#include "Reflection/Private/ReflectedStructure.h"
#include "Reflection/Private/ReflectedObject.h"

// TODO: usage comments
#define DAVA_REFLECTION(Cls) DAVA_REFLECTION__IMPL(Cls)

// TODO: usage comments
#define DAVA_VIRTUAL_REFLECTION(Cls, ...) DAVA_VIRTUAL_REFLECTION__IMPL(Cls, ##__VA_ARGS__)

// TODO: usage comments
#define DAVA_REFLECTION_IMPL(Cls) DAVA_REFLECTION_IMPL__IMPL(Cls)

namespace DAVA
{
class ReflectedType;
class ReflectedTypeDB;
class ReflectedObject;
class ReflectedMeta;

class ValueWrapper;
class MethodWrapper;
class CtorWrapper;
class DtorWrapper;
class StructureWrapper;

class Reflection final
{
public:
    struct Field;
    struct Method;

    Reflection() = default;
    Reflection(const Reflection&) = default;
    Reflection(const ReflectedObject& object, const ValueWrapper* valueWrapper);

    bool IsValid() const;
    bool IsReadonly() const;

    const RtType* GetValueType() const;
    ReflectedObject GetValueObject() const;

    Any GetValue() const;
    bool SetValue(const Any& value) const;

    bool HasFields() const;
    Reflection GetField(const Any& name) const;
    Vector<Field> GetFields() const;

    const ReflectionCaps& GetFieldsCaps() const;

    bool AddField(const Any& key, const Any& value) const;
    bool InsertField(const Any& beforeKey, const Any& key, const Any& value) const;
    bool RemoveField(const Any& key) const;
    Any CreateFieldValue() const;

    bool HasMethods() const;
    AnyFn GetMethod(const String& key) const;
    Vector<Method> GetMethods() const;

    void Dump(std::ostream& out, size_t deep = 0) const;

    template <typename Meta>
    bool HasMeta() const;

    template <typename Meta>
    const Meta* GetMeta() const;

    template <typename T>
    static Reflection Create(T* objectPtr, const ReflectedMeta* objectMeta = nullptr);

    static Reflection Create(const Any& any, const ReflectedMeta* objectMeta = nullptr);

private:
    ReflectedObject object;
    const ValueWrapper* valueWrapper = nullptr;
    const StructureWrapper* structureWrapper = nullptr;
};

struct Reflection::Field
{
    Any key;
    Reflection ref;
};

struct Reflection::Method
{
    String key;
    AnyFn fn;
};

class ValueWrapper
{
public:
    ValueWrapper() = default;
    ValueWrapper(const ValueWrapper&) = delete;
    virtual ~ValueWrapper() = default;

    virtual const RtType* GetType() const = 0;

    virtual bool IsReadonly(const ReflectedObject& object) const = 0;
    virtual Any GetValue(const ReflectedObject& object) const = 0;
    virtual bool SetValue(const ReflectedObject& object, const Any& value) const = 0;

    virtual ReflectedObject GetValueObject(const ReflectedObject& object) const = 0;
};

class MethodWrapper
{
public:
    AnyFn anyFn;
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

    virtual ReflectionCtorPolicy GetCtorPolicy() const = 0;
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

    virtual bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual const ReflectionCaps& GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
    virtual Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const = 0;

    virtual Any CreateValue(const ReflectedObject& object, const ValueWrapper* vw) const = 0;
    virtual bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const = 0;
    virtual bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const = 0;
    virtual bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const = 0;
};

template <typename T>
struct StructureWrapperCreator;

} // namespace DAVA

#define __DAVA_Reflection__
#include "Reflection/Private/Reflection_impl.h"
#include "Reflection/Private/ReflectedMeta_impl.h"
#include "Reflection/Private/ReflectedType_impl.h"
#include "Reflection/Private/ReflectedTypeDB_impl.h"
#include "Reflection/Private/ReflectedObject_impl.h"
