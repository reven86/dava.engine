#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Type;
class ReflectedStructure;
class StructureWrapper;

/**
    \ingroup reflection
    Reflected type.
    Holds some compile-time and runtime information about type `T`:
        - type `T` ctor/dtor
        - type `T` compile-time structure 
        - type `T` runtime structure

    Typically this class should be used for creating or destroing objects with type `T`.
    Object can be created with different policies: by value, by pointer etc. 
    \see `ReflectedType::CreatePolicy`

    \code
    struct Foo;

    Foo CreateFooByValue()
    {
        ReflectedType *fooReflectedType = ReflectedTypeDB::Get<Foo>();
        Any f = fooReflectedType->Create(ReflectedType::CreatePolicy::ByValue); 

        // f should contain Foo instance
        return f.Get<Foo>();
    }
    \endcode

    Holds info about type static(registred by user) structure `ReflectedStructure` and 
    runtime structure wrapper `StructureWrapper`. Also it holds type permanent name, if it was
    specified by user.
*/
class ReflectedType final
{
    template <typename T>
    friend class ReflectionRegistrator;

    friend class ReflectedTypeDB;

public:
    /** Object creating policy. */
    enum class CreatePolicy
    {
        ByValue, //!< Create by value.
        ByPointer //!< Create by pointer.
    };

    ~ReflectedType();

    /** Return type. */
    const Type* GetType() const;

    /** */
    const String& GetPermanentName() const;

    const ReflectedStructure* GetStrucutre() const;
    const StructureWrapper* GetStrucutreWrapper() const;

    Vector<const AnyFn*> GetCtors() const;

    template <typename... Args>
    const AnyFn* GetCtor(const Type* retType = nullptr) const;

    const AnyFn* GetDtor() const;

    template <typename... Args>
    Any Create(CreatePolicy policy, Args... args) const;

    void Destroy(Any&& v) const;

protected:
    String permanentName;
    const Type* type;

    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType(const Type* rttiType_);
};
} // namespace DAVA
