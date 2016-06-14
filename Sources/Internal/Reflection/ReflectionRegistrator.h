#pragma once

#include <cassert>
#include <tuple>

#if !defined(__DAVAENGINE_ANDROID__)

#include "Functional/Function.h"
#include "Reflection/ReflectionWrappersDefault.h"
#include "Reflection/Private/StructureWrapperClass.h"
#include "Reflection/Private/ValueWrapperStatic.h"
#include "Reflection/Private/ValueWrapperStaticFn.h"
#include "Reflection/Private/ValueWrapperFn.h"
#include "Reflection/Private/ValueWrapperClass.h"
#include "Reflection/Private/ValueWrapperClassFn.h"
#include "Reflection/Private/CtorWrapperDefault.h"
#include "Reflection/Private/DtorWrapperDefault.h"

namespace DAVA
{
template <typename C>
struct ReflectionRegistrator
{
    static ReflectionRegistrator& Begin()
    {
        static ReflectionRegistrator ret;
        return ret;
    }

    template <typename B>
    ReflectionRegistrator& Base()
    {
        ReflectionDB::RegisterBaseClass<B, C>();
        childrenWrapper->AddBase<B, C>();
        return *this;
    }

    template <typename B, typename B1, typename... Args>
    ReflectionRegistrator& Base()
    {
        ReflectionDB::RegisterBaseClass<B, C>();
        childrenWrapper->AddBase<C, B>();
        return Base<B1, Args...>();
    }

    template <typename... Args>
    ReflectionRegistrator& Constructor()
    {
        ReflectionDB* db = ReflectionDB::EditGlobalDB<C>();
        auto ctorWrapper = std::make_unique<CtorWrapperDefault<C, Args...>>();
        db->ctorWrappers.emplace_back(std::move(ctorWrapper));
        return *this;
    }

    ReflectionRegistrator& Destructor()
    {
        ReflectionDB* db = ReflectionDB::EditGlobalDB<C>();
        auto dtorWrapper = std::make_unique<DtorWrapperDefault<C>>();
        db->dtorWrapper = std::move(dtorWrapper);
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T* field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperStatic<T>>(field);
        childrenWrapper->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T C::*field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperClass<T, C>>(field);
        childrenWrapper->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = void (*)(SetT);

        return Field(name, getter, static_cast<SetFn>(nullptr));
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (*getter)(), void (*setter)(SetT))
    {
        auto valueWrapper = std::make_unique<ValueWrapperStaticFn<GetT, SetT>>(getter, setter);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = void (C::*)(SetT);

        return Field(name, getter, static_cast<SetFn>(nullptr));
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using GetFn = GetT (C::*)();
        using SetFn = void (C::*)(SetT);

        return Field(name, reinterpret_cast<GetFn>(getter), static_cast<SetFn>(nullptr));
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)() const, void (C::*setter)(SetT))
    {
        using GetFn = GetT (C::*)();

        return Field(name, reinterpret_cast<GetFn>(getter), setter);
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, GetT (C::*getter)(), void (C::*setter)(SetT))
    {
        auto valueWrapper = std::make_unique<ValueWrapperClassFn<GetT, SetT, C>>(getter, setter);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT()>& getter, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = Function<void(SetT)>;

        return Field(name, getter, SetFn());
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT()>& getter, const Function<void(SetT)>& setter)
    {
        auto valueWrapper = std::make_unique<ValueWrapperFn<GetT, SetT>>(getter, setter);
        childrenWrapper->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& operator[](T&& meta)
    {
    }

    void End()
    {
        ReflectionDB* db = ReflectionDB::EditGlobalDB<C>();

        // override children for class C in global DB
        db->structureWrapper = std::move(childrenWrapper);
    }

protected:
    ReflectionRegistrator() = default;
    std::unique_ptr<StructureWrapperClass> childrenWrapper = std::make_unique<StructureWrapperClass>();
};

} // namespace DAVA

#endif