#pragma once
#ifndef DAVAENGINE_TYPE__H
// include Type.h for help IDE parse types here.
#include "Base/Type.h"
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
const Type* GetTypeIfTrue(std::false_type)
{
    return nullptr;
}

template <typename T>
const Type* GetTypeIfTrue(std::true_type)
{
    return Type::Instance<T>();
}

} // namespace TypeDetails

template <typename T>
void Type::Init()
{
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20141030
    // android old-style way
    using T0 = typename std::remove_cv<T>::type;
    using T1 = typename std::remove_reference<T0>::type;
    using DerefT = typename std::remove_pointer<T1>::type;
    using DecayT = std::conditional<std::is_pointer<T>::value, std::add_pointer<std::decay<std::remove_pointer<T>::type>::type>::type, std::decay<T>::type>;
#else
    // standard c++14 way
    using DerefT = std::remove_pointer_t<std::remove_reference_t<std::remove_cv_t<T>>>;
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;
#endif

    static const bool needDeref = (!std::is_same<T, DerefT>::value && !std::is_same<T, void*>::value);
    static const bool needDecay = (!std::is_same<T, DecayT>::value);

    name = typeid(T).name();
    size = sizeof(T);

    isConst = std::is_const<T>::value;
    isPointer = std::is_pointer<T>::value;
    isReference = std::is_reference<T>::value;

    auto condDeref = std::integral_constant<bool, needDeref>();
    derefType = TypeDetails::GetTypeIfTrue<DerefT>(condDeref);

    auto condDecay = std::integral_constant<bool, needDecay>();
    decayType = TypeDetails::GetTypeIfTrue<DecayT>(condDecay);

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
