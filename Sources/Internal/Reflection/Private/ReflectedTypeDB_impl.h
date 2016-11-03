#pragma once

#ifndef __DAVA_ReflectedTypeDB__
#include "Reflection/ReflectedTypeDB.h"
#endif

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
    return static_cast<const ReflectedBase*>(ptr)->GetReflectedType();
}

template <typename T>
const ReflectedType* GetVirtualReflectedType(const T* ptr)
{
    static const bool isReflectedBase = std::is_base_of<ReflectedBase, T>::value;
    return GetVirtualReflectedTypeImpl(ptr, std::integral_constant<bool, isReflectedBase>());
}

template <typename T>
const ReflectedType* GetByThisPointer(const T* this_)
{
    return ReflectedType::Get<T>();
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
    static ReflectedType ret;
    return &ret;
}

template <typename T>
ReflectedType* ReflectedTypeDB::Edit()
{
    using DecayT = RttiType::DecayT<T>;
    ReflectedType* ret = CreateStatic<DecayT>();

    if (nullptr == ret->rttiType)
    {
        ret->rttiType = RttiType::Instance<DecayT>();

        //         ret->structure.reset(new Refle)
        //         ret->structureWrapper.reset(StructureWrapperCreator<DecayT>::Create());
        //         ret->structureEditorWrapper.reset(StructureEditorWrapperCreator<DecayT>::Create());
        //        broken on vs 2013!
        //        TODO: fix
        //        ...
        //
        //        static const bool isDfConstructible = std::is_default_constructible<DecayT>::value;
        //        static const bool isCpConstructible = std::is_copy_constructible<DecayT>::value;
        //        static const bool isMvConstructible = std::is_move_constructible<DecayT>::value;
        //        static const bool hasDefaultCtor = isDfConstructible && (isCpConstructible || isMvConstructible);
        //
        //         CtorWrapper* defaultCtor = ReflectionDetail::GetDefaultCtor<T>(std::integral_constant<bool, hasDefaultCtor>());
        //         if (nullptr != defaultCtor)
        //         {
        //             ret->ctorWrappers.insert(std::unique_ptr<CtorWrapper>(defaultCtor));
        //         }

        rttiTypeToReflectedTypeMap[ret->rttiType] = ret;
        rttiNameToReflectedTypeMap[String(ret->rttiType->GetName())] = ret;

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
            ret = ReflectedType::Edit<T>();
        }
    }

    return ret;
}

template <typename T, typename... Bases>
void ReflectedTypeDB::RegisterBases()
{
    RttiInheritance::RegisterBases<T, Bases...>();
    bool basesUnpack[] = { false, ReflectedType::Edit<Bases>() != nullptr... };
}

#ifdef __REFLECTION_FEATURE__
inline const RttiType* ReflectedType::GetRttiType() const
{
    return rttiType;
}

inline const String& ReflectedType::GetPermanentName() const
{
    return permanentName;
}

inline const String& ReflectedType::GetRttiName() const
{
    return rttiName;
}
#endif

} // namespace DAVA
