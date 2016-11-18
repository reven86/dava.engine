#pragma once

#ifndef __DAVA_Reflection_Qualifier__
#include "Reflection/ReflectionRegistrator.h"
#endif

namespace DAVA
{
namespace ReflectionQualifierDetail
{
template <typename T>
struct FnReturnTypeToReflectedType
{
    using type = typename std::conditional<std::is_pointer<T>::value, typename std::remove_pointer<T>::type, typename std::conditional<std::is_reference<T>::value, typename std::remove_reference<T>::type, std::nullptr_t>::type>::type;
};
}

template <typename C>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Begin()
{
    static ReflectionRegistrator<C> rq;

    if (nullptr == rq.structure)
    {
        rq.structure = new ReflectedStructure();
    }

    rq.lastMeta = &rq.structure->meta;

    return rq;
}

template <typename C>
template <typename... Args>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::ConstructorByValue()
{
    structure->ctors.emplace_back(new CtorWrapperByValue<C, Args...>());
    return *this;
}

template <typename C>
template <typename... Args>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::ConstructorByPointer()
{
    structure->ctors.emplace_back(new CtorWrapperByPointer<C, Args...>());
    return *this;
}

template <typename C>
template <typename... Args>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::ConstructorByPointer(C* (*fn)(Args...))
{
    structure->ctors.emplace_back(new CtorWrapperByPointer<C, Args...>(fn));
    return *this;
}

template <typename C>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::DestructorByPointer()
{
    structure->dtor.reset(new DtorWrapperByPointer<C>());
    return *this;
}

template <typename C>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::DestructorByPointer(void (*fn)(C*))
{
    structure->dtor.reset(new DtorWrapperByPointer<C>(fn));
    return *this;
}

template <typename C>
template <typename T>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::AddField(const char* name, ValueWrapper* vw)
{
    ReflectedStructure::Field* f = new ReflectedStructure::Field();
    f->name = name;
    f->valueWrapper.reset(vw);
    f->reflectedType = ReflectedTypeDB::Get<T>();

    lastMeta = &f->meta;
    structure->fields.emplace_back(f);

    return *this;
}

template <typename C>
template <typename T>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, T* field)
{
    return AddField<T>(name, new ValueWrapperStatic<T>(field));
}

template <typename C>
template <typename T>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, T C::*field)
{
    return AddField<T>(name, new ValueWrapperClass<T, C>(field));
}

template <typename C>
template <typename GetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, GetT (*getter)(), std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperStaticFnPtr<GetT, SetT>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, GetT (*getter)(), void (*setter)(SetT))
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    return AddField<T>(name, new ValueWrapperStaticFnPtr<GetT, SetT>(getter, setter));
}

template <typename C>
template <typename GetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, GetT (C::*getter)(), std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFnPtr<GetT, SetT, C>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT))
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFnPtr<GetT, SetT, C>(getter, setter));
}

template <typename C>
template <typename GetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, GetT (C::*getter)() const, std::nullptr_t)
{
    using SetT = typename std::remove_reference<GetT>::type;
    using GetFn = GetT (C::*)();
    using SetFn = void (C::*)(SetT);

    return Field(name, reinterpret_cast<GetFn>(getter), static_cast<SetFn>(nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT))
{
    using GetFn = GetT (C::*)();

    return Field(name, reinterpret_cast<GetFn>(getter), setter);
}

template <typename C>
template <typename GetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, const Function<GetT()>& getter, std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperStaticFn<GetT, SetT>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;

    return AddField<T>(name, new ValueWrapperStaticFn<GetT, SetT>(getter, setter));
}

template <typename C>
template <typename GetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFn<GetT, SetT, C>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFn<GetT, SetT, C>(getter, setter));
}

template <typename C>
template <typename Mt>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::Method(const char* name, const Mt& method)
{
    MethodWrapper* methodWrapper = new MethodWrapper();
    methodWrapper->anyFn = AnyFn(method);

    ReflectedStructure::Method* m = new ReflectedStructure::Method();
    m->name = name;
    m->methodWrapper.reset(methodWrapper);

    lastMeta = &m->meta;
    structure->methods.emplace_back(m);

    return *this;
}

template <typename C>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::BindMeta(ReflectedMeta&& meta)
{
    if (nullptr != lastMeta)
    {
        lastMeta->reset(new ReflectedMeta(std::move(meta)));
    }

    return *this;
}

template <typename C>
ReflectionRegistrator<C>& ReflectionRegistrator<C>::operator[](ReflectedMeta&& meta)
{
    return BindMeta(std::move(meta));
}

template <typename C>
void ReflectionRegistrator<C>::End()
{
    if (nullptr != structure)
    {
        ReflectedType* type = ReflectedTypeDB::Edit<C>();

        type->structure.reset(structure);
        type->structureWrapper.reset(new StructureWrapperClass(Type::Instance<C>()));

        structure = nullptr;
    }

    lastMeta = nullptr;
}

} // namespace DAVA
