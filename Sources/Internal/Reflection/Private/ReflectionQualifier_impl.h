#pragma once

#ifndef __DAVA_Reflection_Qualifier__
#include "Reflection/ReflectionQualifier.h"
#endif

namespace DAVA
{
template <typename C>
ReflectionQualifier<C>& ReflectionQualifier<C>::Begin()
{
    static ReflectionQualifier<C> rq;

    if (nullptr == rq.structure)
    {
        rq.structure = new ReflectedStructure();
    }

    return rq;
}

template <typename C>
template <typename... Args>
ReflectionQualifier<C>& ReflectionQualifier<C>::Constructor()
{
    ReflectedType* type = ReflectedTypeDB::Edit<C>();
    structure->ctors.emplace_back(new CtorWrapperDefault<C, Args...>());
    return *this;
}

template <typename C>
ReflectionQualifier<C>& ReflectionQualifier<C>::Destructor()
{
    ReflectedType* type = ReflectedTypeDB::Edit<C>();
    structure->dtors.emplace_back(new DtorWrapperDefault<C>());
    return *this;
}

template <typename C>
template <typename T>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, T* field)
{
    auto valueWrapper = std::make_unique<ValueWrapperStatic<T>>(field);
    //sw->AddField<T>(name, std::move(valueWrapper));
    return *this;
}

template <typename C>
template <typename T>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, T C::*field)
{
    auto valueWrapper = std::make_unique<ValueWrapperClass<T, C>>(field);
    //sw->AddField<T>(name, std::move(valueWrapper));
    return *this;
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (*getter)(), std::nullptr_t)
{
    using SetT = typename std::remove_reference<GetT>::type;
    using SetFn = void (*)(SetT);

    return Field(name, getter, static_cast<SetFn>(nullptr));
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (*getter)(), void (*setter)(SetT))
{
    auto valueWrapper = std::make_unique<ValueWrapperStaticFnPtr<GetT, SetT>>(getter, setter);
    //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
    return *this;
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (C::*getter)(), std::nullptr_t)
{
    using SetT = typename std::remove_reference<GetT>::type;
    using SetFn = void (C::*)(SetT);

    return Field(name, getter, static_cast<SetFn>(nullptr));
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
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT))
{
    auto valueWrapper = std::make_unique<ValueWrapperClassFnPtr<GetT, SetT, C>>(getter, setter);
    //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
    return *this;
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT()>& getter, std::nullptr_t)
{
    using SetT = typename std::remove_reference<GetT>::type;
    using SetFn = Function<void(SetT)>;

    return Field(name, getter, SetFn());
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter)
{
    auto valueWrapper = std::make_unique<ValueWrapperStaticFn<GetT, SetT>>(getter, setter);
    //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
    return *this;
}

template <typename C>
template <typename GetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t)
{
    using SetT = typename std::remove_reference<GetT>::type;
    using SetFn = Function<void(C*, SetT)>;

    return Field(name, getter, SetFn());
}

template <typename C>
template <typename GetT, typename SetT>
ReflectionQualifier<C>& ReflectionQualifier<C>::Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter)
{
    auto valueWrapper = std::make_unique<ValueWrapperClassFn<C, GetT, SetT>>(getter, setter);
    //sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
    return *this;
}

template <typename C>
template <typename Mt>
ReflectionQualifier<C>& ReflectionQualifier<C>::Method(const char* name, const Mt& method)
{
    //sw->AddMethod(name, method);
    return *this;
}

template <typename C>
void ReflectionQualifier<C>::End()
{
    if (nullptr != structure)
    {
        // override children for class C in appropriate ReflectedType
        ReflectedType* type = ReflectedTypeDB::Edit<C>();
        type->structure.reset(structure);
        structure = nullptr;

        // TODO:
        // ...
        // Add class wrapper
        // ...
    }
}

template <typename C>
ReflectionQualifier<C>& ReflectionQualifier<C>::operator[](ReflectedMeta&& meta)
{
    //sw->AddMeta(std::move(meta));
    return *this;
}

} // namespace DAVA
