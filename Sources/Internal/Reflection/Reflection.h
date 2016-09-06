#ifndef __DAVA_Reflection_Definition__
#define __DAVA_Reflection_Definition__

#include <sstream>
#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/Type.h"

#include "Reflection/ReflectedBase.h"
#include "Reflection/ReflectedObject.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class ValueWrapper;
class StructureWrapper;
class StructureEditorWrapper;

class Reflection final
{
public:
    struct Field;
    struct Method;

    Reflection() = default;
    Reflection(const ReflectedObject& object, ValueWrapper* vw, const ReflectedType* rtype, const ReflectedMeta* meta);

    bool IsValid() const;
    bool IsReadonly() const;

    const Type* GetValueType() const;
    ReflectedObject GetValueObject() const;

    Any GetValue() const;
    bool SetValue(const Any&) const;

    bool HasFields() const;
    Field GetField(const Any&) const;
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
    Method GetMethod(const String& key) const;
    Vector<Method> GetMethods() const;

    template <typename Meta>
    bool HasMeta() const;

    template <typename Meta>
    const Meta* GetMeta() const;

    void Dump(std::ostream& out, size_t maxlevel = 0) const;
    void DumpMethods(std::ostream& out) const;

    template <typename T>
    static Reflection Create(T* ptr, const ReflectedMeta* meta = nullptr);

private:
    const ValueWrapper* vw = nullptr;
    const StructureWrapper* sw = nullptr;
    const StructureEditorWrapper* sew = nullptr;
    const ReflectedMeta* meta = nullptr;

    ReflectedObject object;
};

struct Reflection::Field
{
    Any key;
    Reflection ref;

    template <typename T>
    static Reflection::Field Create(const Any& key, T* ptr, const ReflectedMeta* meta = nullptr);
};

struct Reflection::Method
{
    String key;
    AnyFn fn;
};

} // namespace DAVA

#endif // __DAVA_Reflection_Definition__

#ifndef __DAVA_Reflection_Definition_Only__
#define __DAVA_Reflection__
#include "Reflection/Wrappers.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/Private/Reflection_impl.h"
#include "Reflection/Private/StructureWrapperClass.h"
#endif
