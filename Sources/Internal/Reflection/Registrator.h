#pragma once

#include <type_traits>
#include "Reflection/Reflection.h"
#include "Reflection/Private/ValueWrapperDefault.h"
#include "Reflection/Private/ValueWrapperDirect.h"
#include "Reflection/Private/ValueWrapperClass.h"
#include "Reflection/Private/ValueWrapperClassFn.h"
#include "Reflection/Private/ValueWrapperStatic.h"
#include "Reflection/Private/ValueWrapperStaticFn.h"
#include "Reflection/Private/ValueWrapperDavaFn.h"
#include "Reflection/Private/ValueWrapperDavaClassFn.h"
#include "Reflection/Private/CtorWrapperDefault.h"
#include "Reflection/Private/DtorWrapperDefault.h"
#include "Reflection/Private/StructureWrapperClass.h"
#include "Reflection/Private/StructureWrapperPtr.h"
#include "Reflection/Private/StructureWrapperStd.h"
#include "Reflection/Private/StructureEditorWrapperPtr.h"
#include "Reflection/Private/StructureEditorWrapperStd.h"

#define DAVA_REFLECTION(Cls) \
    template <typename FT__> \
    friend struct DAVA::ReflectionDetail::ReflectionInitializerRunner; \
    static void __ReflectionInitializer() \
    { \
        static_assert(!std::is_base_of<DAVA::ReflectedBase, Cls>::value, "Use DAVA_VIRTUAL_REFLECTION for classes derived from ReflectedBase"); \
        DAVA::ReflectedType::Get<Cls>()->SetPermanentName(#Cls); \
        __ReflectionInitializer_Impl(); \
    } \
    static void __ReflectionInitializer_Impl()

#define DAVA_VIRTUAL_REFLECTION(Cls, ...) \
    template <typename FT__> \
    friend struct DAVA::ReflectionDetail::ReflectionInitializerRunner; \
    const DAVA::ReflectedType* GetReflectedType() const override \
    { \
        return DAVA::ReflectionDetail::GetByThisPointer(this); \
    } \
    static void __ReflectionInitializer() \
    { \
        static_assert(std::is_base_of<DAVA::ReflectedBase, Cls>::value, "Use DAVA_REFLECTION for classes that didn't derived from ReflectedBase"); \
        DAVA::ReflectedType::RegisterBases<Cls, ##__VA_ARGS__>(); \
        DAVA::ReflectedType::Get<Cls>()->SetPermanentName(#Cls); \
        __ReflectionInitializer_Impl(); \
    } \
    static void __ReflectionInitializer_Impl()

#define DAVA_REFLECTION_IMPL(Cls) \
    void Cls::__ReflectionInitializer_Impl()

namespace DAVA
{
template <typename C>
class ReflectionRegistrator final
{
public:
    static ReflectionRegistrator& Begin()
    {
        static ReflectionRegistrator ret;
        return ret;
    }

    template <typename... Args>
    ReflectionRegistrator& Constructor()
    {
        ReflectedType* type = ReflectedType::Edit<C>();
        auto ctorWrapper = std::make_unique<CtorWrapperDefault<C, Args...>>();
        type->ctorWrappers.emplace(std::move(ctorWrapper));
        return *this;
    }

    ReflectionRegistrator& Destructor()
    {
        ReflectedType* type = ReflectedType::Edit<C>();
        auto dtorWrapper = std::make_unique<DtorWrapperDefault<C>>();
        type->dtorWrapper = std::move(dtorWrapper);
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T* field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperStatic<T>>(field);
        sw->AddField<T>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename T>
    ReflectionRegistrator& Field(const char* name, T C::*field)
    {
        auto valueWrapper = std::make_unique<ValueWrapperClass<T, C>>(field);
        sw->AddField<T>(name, std::move(valueWrapper));
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
        sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
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
        sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
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
        auto valueWrapper = std::make_unique<ValueWrapperDavaFn<GetT, SetT>>(getter, setter);
        sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename GetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT(C*)>& getter, std::nullptr_t)
    {
        using SetT = typename std::remove_reference<GetT>::type;
        using SetFn = Function<void(C*, SetT)>;

        return Field(name, getter, SetFn());
    }

    template <typename GetT, typename SetT>
    ReflectionRegistrator& Field(const char* name, const Function<GetT(C*)>& getter, const Function<void(C*, SetT)>& setter)
    {
        auto valueWrapper = std::make_unique<ValueWrapperDavaClassFn<C, GetT, SetT>>(getter, setter);
        sw->AddFieldFn<GetT>(name, std::move(valueWrapper));
        return *this;
    }

    template <typename Mt>
    ReflectionRegistrator& Method(const char* name, const Mt& method)
    {
        sw->AddMethod(name, method);
        return *this;
    }

    ReflectionRegistrator& operator[](ReflectedMeta&& meta)
    {
        sw->AddMeta(std::move(meta));
        return *this;
    }

    void End()
    {
        // override children for class C in appropriate ReflectedType
        ReflectedType* type = ReflectedType::Edit<C>();
        type->structureWrapper = std::move(sw);
    }

private:
    ReflectionRegistrator() = default;
    std::unique_ptr<StructureWrapperClass> sw = std::make_unique<StructureWrapperClass>(Type::Instance<C>());
    std::unique_ptr<ReflectedMeta> meta;
};

} // namespace DAVA
