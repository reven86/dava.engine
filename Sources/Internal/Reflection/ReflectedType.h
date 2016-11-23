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

    Typically `ReflectedType` should be used for creating or destroing objects with type `T`.
    Object can be created with different policies: by value, by pointer etc. 

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

    Also it holds info about type static(registred by user) structure `ReflectedStructure` and 
    runtime structure wrapper `StructureWrapper`, which are internally used by `Reflection` class.
    The user is encouraged to use `Reflection` class instead of using this info directly. However,
    there may be cases when user have to use `ReflectedStructure`, e.g. for performance reason:

    \code
    void FastSetValue(std::vector<Object> objects)
    {
        ReflectedType *objectReflectedType = ReflectedTypeDB::Get<Object>();
        ReflectedStructure* objectStructure = objectReflectedType->GetStrucutre();
        ValueWrapper* vw = objectStructure->fields[0]->valueWrapper.get();

        // we don't need to reflect each object, instead
        // we can access its first field by obtained ValueWrapper
        for(auto& object : objects)
        {
            vw->SetValue(ReflectedObject(&object), Any("new value"));
        }
    }
    \endcode
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

    /** 
        Return registred permanent name for that type.
        If name wasn't registred empty string will be returned. Permanent name can be registred
        will ReflectionTypeDB::RegisterPermanentName method.
    */
    const String& GetPermanentName() const;

    /** Return registred structure for that type. */
    const ReflectedStructure* GetStrucutre() const;

    /** Return registred structure for that type. */
    const StructureWrapper* GetStrucutreWrapper() const;

    /** Returns all registred ctors. */
    Vector<const AnyFn*> GetCtors() const;

    /** 
        Returns first registred ctor, that match specified arguments `Args` and return type `retType`. 
        If ctor isn't found `nullptr` will be returned.
    */
    template <typename... Args>
    const AnyFn* GetCtor(const Type* retType = nullptr) const;

    /** 
        Returns registred dtor. 
        If there is no registred dtor, `nullptr` will be returned. 
    */
    const AnyFn* GetDtor() const;

    /**
        Creates object, by invoking ctor with specified `Args`. 
        Creation is performed depending on specified policy - returned `Any` can
        contain object "by value" or object "by pointer". 
        If there is no appropriate ctor, exception will be raised.
        \throw DAVA::Exception
    */
    template <typename... Args>
    Any Create(CreatePolicy policy, Args... args) const;

    /**
        Destoy object contained in `v`, by invoking dtor.
        Destruction is performed on `v` contained object:
        - If `v` contains object "by value" just `Any::Clear()` will be innoked. 
        - If `v` contains object "by pointer" dtor will be called.
        If there is no appropriate dtor for `v` contained object, exception will be raised.
        \throw DAVA::Exception
    */
    void Destroy(Any&& v) const;

protected:
    String permanentName;
    const Type* type;

    std::unique_ptr<ReflectedStructure> structure;
    std::unique_ptr<StructureWrapper> structureWrapper;

    ReflectedType(const Type* rttiType_);
};
} // namespace DAVA
