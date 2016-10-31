#pragma once

#include <sstream>
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/RttiType.h"

#include "Reflection/ReflectionBase.h"
#include "Reflection/ReflectionRaw.h"
#include "Reflection/ReflectedMeta.h"
#include "Reflection/ReflectedObject.h"

namespace DAVA
{
class Reflection final
{
public:
    class Field;
    class Method;

    Reflection() = default;
    Reflection(ReflectionRaw&& raw);

    bool IsValid() const;
    bool IsReadonly() const;

    const RttiType* GetValueType() const;
    ReflectedObject GetValueObject() const;

    const ReflectedType* GetReflectedType() const;

    Any GetValue() const;
    bool SetValue(const Any& value) const;

    bool HasFields() const;
    Reflection GetField(const Any& name) const;
    Vector<Field> GetFields() const;

    bool CanAddFields() const;
    bool CanInsertFields() const;
    bool CanRemoveFields() const;
    bool CanCreateFieldValue() const;

    Any CreateFieldValue() const;
    bool AddField(const Any& key, const Any& value) const;
    bool InsertField(const Any& beforeKey, const Any& key, const Any& value) const;
    bool RemoveField(const Any& key) const;

    bool HasMethods() const;
    AnyFn GetMethod(const String& key) const;
    Vector<Method> GetMethods() const;

    void Dump(std::ostream& out, size_t deep = 0) const;
    void DumpMethods(std::ostream& out) const;

    template <typename Meta>
    bool HasMeta() const;

    template <typename Meta>
    const Meta* GetMeta() const;

    template <typename T>
    static Reflection Create(T& ptr, const ReflectedMeta* meta = nullptr);

private:
    ReflectedObject object;

    const ValueWrapper* vw = nullptr;
    const StructureWrapper* sw = nullptr;
    const ReflectedMeta* meta = nullptr;
    const ReflectedType* objectType = nullptr;
};

/// \brief A reflection field.
class Reflection::Field
{
public:
    Any key; ///< field key (usually name or index)
    Reflection ref; ///< field reflection

    template <typename T>
    static Reflection::Field Create(const Any& key, T* ptr, const ReflectedMeta* meta = nullptr);
};

/// \brief A reflection method.
class Reflection::Method
{
public:
    String key; ///< method key (usually its name)
    AnyFn fn; ///< method itself with binded runtime object it belongs to
};

} // namespace DAVA

#define __DAVA_Reflection__
#include "Reflection/Private/Reflection_impl.h"
