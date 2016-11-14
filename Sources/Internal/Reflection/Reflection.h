#pragma once

#include <sstream>
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/RttiType.h"
#include "Debug/DVAssert.h"

#include "Reflection/ReflectedMeta.h"
#include "Reflection/ReflectedObject.h"
#include "Reflection/ReflectedType.h"

// TODO: usage comments
#define DAVA_REFLECTION(Cls) DAVA_REFLECTION__IMPL(Cls)

// TODO: usage comments
#define DAVA_VIRTUAL_REFLECTION(Cls, ...) DAVA_VIRTUAL_REFLECTION__IMPL(Cls, ##__VA_ARGS__)

// TODO: usage comments
#define DAVA_REFLECTION_IMPL(Cls) DAVA_REFLECTION_IMPL__IMPL(Cls)

namespace DAVA
{
class ValueWrapper;
class StructureWrapper;

class Reflection final
{
public:
    struct Field;
    struct Method;

    struct FieldsCaps
    {
        bool add = false;
        bool insert = false;
        bool remove = false;
        bool createValue = false;
        bool dynamic = false;
        bool flat = false;
        const RttiType* flatKeyType = nullptr;
        const RttiType* flatValueType = nullptr;
    };

    Reflection() = default;
    Reflection(const Reflection&) = default;
    Reflection(const ReflectedObject& object_, const ReflectedType* objectType_, const ReflectedMeta* objectMeta_, const ValueWrapper* valueWrapper_);

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

    const FieldsCaps& GetFieldsCaps() const;

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
    const ReflectedMeta* objectMeta = nullptr;
    const ReflectedType* objectType = nullptr;
    const ValueWrapper* valueWrapper = nullptr;
    const StructureWrapper* structureWrapper = nullptr;
};

/// \brief A reflection field.
struct Reflection::Field
{
    Any key;
    Reflection ref;
};

/// \brief A reflection method.
struct Reflection::Method
{
    String key; ///< method key (usually its name)
    AnyFn fn; ///< method itself with binded runtime object it belongs to
};

} // namespace DAVA

#define __DAVA_Reflection__
#include "Reflection/Wrappers.h"
#include "Reflection/WrappersRuntime.h"
#include "ReflectedTypeDB.h"
#include "Reflection/Private/Reflection_impl.h"
