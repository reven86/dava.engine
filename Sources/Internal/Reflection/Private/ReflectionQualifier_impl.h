#pragma once

#ifndef __DAVA_Reflection_Qualifier__
#include "Reflection/ReflectionQualifier.h"
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
ReflectionQualifier<C>& ReflectionQualifier<C>::Begin()
{
    static ReflectionQualifier<C> rq;

    if (nullptr == rq.structure)
    {
        rq.structure = new ReflectedStructure();
    }

    rq.lastMeta = &rq.structure->meta;

    return rq;
}

template <typename C>
template <typename... Args>
ReflectionQualifier<C>& ReflectionQualifier<C>::Constructor()
{
    structure->ctors.emplace_back(new CtorWrapperDefault<C, Args...>());
    return *this;
}

template <typename C>
ReflectionQualifier<C>& ReflectionQualifier<C>::Destructor()
{
    structure->dtors.emplace_back(new DtorWrapperDefault<C>());
    return *this;
}

template <typename C>
template <typename T>
ReflectionQualifier<C>& ReflectionQualifier<C>::AddField(const char* name, ValueWrapper* vw)
{
    ReflectedType* reflectedType = ReflectedTypeDB::Edit<T>();
    if (nullptr == reflectedType->structureWrapper)
    {
        using DecayT = RttiType::DecayT<T>;
        reflectedType->structureWrapper.reset(StructureWrapperCreator<DecayT>::Create());
    }

    ReflectedStructure::Field* f = new ReflectedStructure::Field();
    f->name = name;
    f->valueWrapper.reset(vw);
    f->reflectedType = reflectedType;
    lastMeta = &f->meta;

    structure->fields.emplace_back(f);

    return *this;
}

template <typename C>
template <typename T>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, T* field)
{
    return AddField<T>(name, new ValueWrapperStatic<T>(field));
}

template <typename C>
template <typename T>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, T C::*field)
{
    return AddField<T>(name, new ValueWrapperClass<T, C>(field));
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (*getter)(), std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperStaticFnPtr<GetT, SetT>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (*getter)(), void (*setter)(SetT))
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    return AddField<T>(name, new ValueWrapperStaticFnPtr<GetT, SetT>(getter, setter));
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (C::*getter)(), std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFnPtr<GetT, SetT, C>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT))
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFnPtr<GetT, SetT, C>(getter, setter));
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (C::*getter)() const, std::nullptr_t)
{
    using SetT = typename std::remove_reference<GetT>::type;
    using GetFn = GetT (C::*)();
    using SetFn = void (C::*)(SetT);

    return Field(name, reinterpret_cast<GetFn>(getter), static_cast<SetFn>(nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT))
{
    using GetFn = GetT (C::*)();

    return Field(name, reinterpret_cast<GetFn>(getter), setter);
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT()>& getter, std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperStaticFn<GetT, SetT>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;

    return AddField<T>(name, new ValueWrapperStaticFn<GetT, SetT>(getter, setter));
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;
    using SetT = typename std::remove_reference<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFn<GetT, SetT, C>(getter, nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter)
{
    using T = typename ReflectionQualifierDetail::FnReturnTypeToReflectedType<GetT>::type;

    return AddField<T>(name, new ValueWrapperClassFn<GetT, SetT, C>(getter, setter));
}

template <typename C>
template <typename Mt>
ReflectionQualifier<C>& ReflectionQualifier<C>::Method(const char* name, const Mt& method)
{
    MethodWrapper* methodWrapper = new MethodWrapper();
    methodWrapper->method = AnyFn(method);

    ReflectedStructure::Method* m = new ReflectedStructure::Method();
    m->name = name;
    m->methodWrapper.reset(methodWrapper);

    lastMeta = &m->meta;
    structure->methods.emplace_back(m);

    return *this;
}

template <typename C>
void ReflectionQualifier<C>::End()
{
    if (nullptr != structure)
    {
        ReflectedType* type = ReflectedTypeDB::Edit<C>();

        type->structure.reset(structure);
        type->structureWrapper.reset(new StructureWrapperClass(type));

        structure = nullptr;
    }

    lastMeta = nullptr;
}

template <typename C>
ReflectionQualifier<C>& ReflectionQualifier<C>::operator[](ReflectedMeta&& meta)
{
    if (nullptr != lastMeta)
    {
        lastMeta->reset(new ReflectedMeta(std::move(meta)));
    }

    return *this;
}

} // namespace DAVA
