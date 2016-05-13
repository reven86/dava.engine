#pragma once
#ifndef DAVAENGINE_TYPE__H
#include "../Type.h"
#endif
#include <atomic>

namespace DAVA
{
namespace TypeDetails
{
template <typename T>
struct TypeInitializerRunner
{
protected:
    template <typename U, void (*)()>
    struct SFINAE
    {
    };

    template <typename U>
    static char Test(SFINAE<U, &U::__TypeInitializer>*);

    template <typename U>
    static int Test(...);

    static const bool value = std::is_same<decltype(Test<T>(0)), char>::value;

    inline static void RunImpl(std::true_type)
    {
        // T has TypeInitializer function,
        // so we should run it
        T::__TypeInitializer();
    }

    inline static void RunImpl(std::false_type)
    {
        // T don't have TypeInitializer function,
        // so nothing to do here
    }

public:
    static void Run()
    {
        using CheckType = typename std::conditional<std::is_class<T>::value, TypeInitializerRunner<T>, std::false_type>::type;
        RunImpl(std::integral_constant<bool, CheckType::value>());
    }
};

template <typename T>
const Type* GetDerefType(std::false_type)
{
    return nullptr;
}

template <typename T>
const Type* GetDerefType(std::true_type)
{
    return Type::Instance<T>();
}

} // namespace TypeDetails

template <typename T>
void Type::Init()
{
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20141030
    // android old-style way
    using T0 = typename std::remove_const<T>::type;
    using T1 = typename std::remove_reference<T0>::type;
    using DerefT = typename std::remove_pointer<T1>::type;
#else
    // standard c++14 way
    using DerefT = std::remove_pointer_t<std::remove_reference_t<std::remove_const_t<T>>>;
#endif

    static const bool needDeref = (!std::is_same<T, DerefT>::value && !std::is_same<T, void*>::value);

    name = typeid(T).name();
    size = sizeof(T);

    isConst = std::is_const<T>::value;
    isPointer = std::is_pointer<T>::value;
    isReference = std::is_reference<T>::value;

    auto cond = std::integral_constant<bool, needDeref>();
    derefType = TypeDetails::GetDerefType<DerefT>(cond);

    // try to run TypeInitializer if T has this function
    TypeDetails::TypeInitializerRunner<T>::Run();
}

template <typename T>
inline const Type* Type::Instance()
{
    static Type* type = nullptr;

    if (nullptr == type)
    {
        type = new Type();
        type->Init<T>();
    }

    return type;
}

} // namespace DAVA
