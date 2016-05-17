#pragma once
#define DAVA_REFLECTION__H

#include <cassert>
#include <vector>

#include "Base/Any.h"
#include "Reflection/ReflectedObject.h"
#include "Reflection/ReflectionVirt.h"

#define DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION                                        \
    const DAVA::ReflectionDB* GetVirtualReflectionDB() const override               \
    {                                                                               \
        return DAVA::Ref::AutoGetReflectionDB(this);                                \
    }

namespace DAVA
{
namespace Ref
{
using ParamsList = DAVA::Vector<const Type*>;
}

class ValueWrapper;
class CtorWrapper;
class DtorWrapper;
class MethodWrapper;
class StructureWrapper;

class Reflection final
{
public:
    Reflection() = default;
    Reflection(const ReflectedObject& obj, const ValueWrapper* vw_, const ReflectionDB* db_)
        : that(obj)
        , vw(vw_)
        , db(db_)
    {
    }

    void Dump(std::ostream& stream);

    bool IsValid() const;
    bool IsReadonly() const;

    Any GetValue() const;
    bool SetValue(const Any&) const;

    const Type* GetValueType() const;
    ReflectedObject GetValueObject() const;

    const CtorWrapper* GetCtor() const;
    const CtorWrapper* GetCtor(const Ref::ParamsList& params) const;
    DAVA::Vector<const CtorWrapper*> GetCtors() const;

    const DtorWrapper* GetDtor() const;

    const MethodWrapper* GetMethod(const char* name);
    const MethodWrapper* GetMethod(const char* name, const Ref::ParamsList& params);
    DAVA::Vector<const MethodWrapper*> GetMethods() const;

    const StructureWrapper* GetStructure() const;

    template <typename T>
    static Reflection Reflect(T* object);

private:
    ReflectedObject that;

    const ValueWrapper* vw = nullptr;
    const ReflectionDB* db = nullptr;
};

} // namespace DAVA
    
#include "Private/ReflectionImpl.h"
