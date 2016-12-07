#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/Wrappers/StructureWrapperDefault.h"
#include "Reflection/Private/Wrappers/StructureWrapperPtr.h"
#include "Reflection/Private/Wrappers/StructureWrapperStdIdx.h"
#include "Reflection/Private/Wrappers/StructureWrapperStdMap.h"
#include "Reflection/Private/Wrappers/StructureWrapperStdSet.h"

namespace DAVA
{
namespace ReflectedTypeDBDetail
{
template <typename T>
const ReflectedType* GetVirtualReflectedTypeImpl(const T* ptr, std::false_type)
{
    return nullptr;
}

template <typename T>
const ReflectedType* GetVirtualReflectedTypeImpl(const T* ptr, std::true_type)
{
    static_assert(std::is_convertible<const T*, const ReflectionBase*>::value,
                  "T* can't be converted to ReflectionBase*. "
                  "It seems that there is inheritance Diamond Problem. "
                  "Try to use virtual inheritance!");

    return static_cast<const ReflectionBase*>(ptr)->GetReflectedType();
}

template <typename T>
const ReflectedType* GetVirtualReflectedType(const T* ptr)
{
    static const bool isRefBase = std::is_base_of<ReflectionBase, T>::value;
    return GetVirtualReflectedTypeImpl(ptr, std::integral_constant<bool, isRefBase>());
}

template <typename T>
const ReflectedType* GetByThisPointer(const T* this_)
{
    return ReflectedTypeDB::Get<T>();
}

template <typename T>
struct ReflectionInitializerRunner
{
protected:
    template <typename U, void (*)()>
    struct SFINAE
    {
    };

    template <typename U>
    static char Test(SFINAE<U, &U::__ReflectionInitializer>*);

    template <typename U>
    static int Test(...);

    static const bool value = std::is_same<decltype(Test<T>(0)), char>::value;

    inline static void RunImpl(std::true_type)
    {
        // T has TypeInitializer function,
        // so we should run it
        T::__ReflectionInitializer();
    }

    inline static void RunImpl(std::false_type)
    {
        // T don't have TypeInitializer function,
        // so nothing to do here
    }

public:
    static void Run()
    {
        using CheckType = typename std::conditional<std::is_class<T>::value, ReflectionInitializerRunner<T>, std::false_type>::type;
        RunImpl(std::integral_constant<bool, CheckType::value>());
    }
};
} // namespace ReflectedTypeDBDetail

template <typename T>
ReflectedType* ReflectedTypeDB::CreateStatic()
{
    static ReflectedType ret(nullptr);
    return &ret;
}

template <typename T>
ReflectedType* ReflectedTypeDB::Edit()
{
    using DecayT = Type::DecayT<T>;
    ReflectedType* ret = CreateStatic<DecayT>();

    if (nullptr == ret->type)
    {
        ret->type = Type::Instance<DecayT>();
        ret->structureWrapper.reset(StructureWrapperCreator<DecayT>::Create());

        typeToReflectedTypeMap[ret->type] = ret;
        typeNameToReflectedTypeMap[String(ret->type->GetName())] = ret;

        ReflectedTypeDBDetail::ReflectionInitializerRunner<DecayT>::Run();
    }

    return ret;
}

template <typename T>
const ReflectedType* ReflectedTypeDB::Get()
{
    return Edit<T>();
}

template <typename T>
const ReflectedType* ReflectedTypeDB::GetByPointer(const T* ptr)
{
    const ReflectedType* ret = nullptr;

    if (nullptr != ptr)
    {
        ret = ReflectedTypeDBDetail::GetVirtualReflectedType(ptr);
        if (nullptr == ret)
        {
            ret = ReflectedTypeDB::Edit<T>();
        }
    }

    return ret;
}

template <typename T, typename... Bases>
void ReflectedTypeDB::RegisterBases()
{
    TypeInheritance::RegisterBases<T, Bases...>();
    bool basesUnpack[] = { false, ReflectedTypeDB::Edit<Bases>() != nullptr... };
}

} // namespace DAVA
