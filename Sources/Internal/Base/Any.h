#pragma once

#include <typeindex>
#include <string>

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Base/Exception.h"
#include "Base/Private/AutoStorage.h"

namespace DAVA
{
class Any final
{
public:
    using AnyStorage = AutoStorage<>;

    //     using LoadOP = void (*)(AnyStorage&, const void* src);
    //     using StoreOP = void (*)(const AnyStorage&, void* dst);
    //     using CastOP = Any (*)(const Any& from);
    //
    //     struct AnyOPs
    //     {
    //         LoadOP load = nullptr;
    //         StoreOP store = nullptr;
    //         CompareOP compare = nullptr;
    //     };

    template <typename T>
    using NotAny = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, bool>::type;

    Any() = default;
    ~Any() = default;
    Any(const Any&) = default;

    Any(Any&& any);

    template <typename T>
    Any(T&& value, NotAny<T> = true);

    void Swap(Any& any);

    bool IsEmpty() const;
    void Clear();

    const Type* GetType() const;

    template <typename T>
    bool CanGet() const;

    template <typename T>
    const T& Get() const;

    template <typename T>
    const T& Get(const T& defaultValue) const;

    void Set(Any&& any);
    void Set(const Any& any);

    template <typename T>
    void Set(T&& value, NotAny<T> = true);

    template <typename T>
    bool CanCast() const;

    template <typename T>
    T Cast() const;

    void LoadValue(const Type* type_, void* data);
    void StoreValue(void* data, size_t size) const;

    Any& operator=(Any&&);
    Any& operator=(const Any&) = default;

    bool operator==(const Any&) const;
    bool operator!=(const Any&) const;

    //     template <typename T>
    //     static void RegisterDefaultOPs();
    //
    //     template <typename T>
    //     static void RegisterOPs(LoadOP& lop, StoreOP &sop, CompareOP& cop);
    //
    //     template <typename T1, typename T2>
    //     static void RegisterDefaultCastOP();
    //
    //     template <typename T1, typename T2>
    //     static void RegisterCastOP(CastOP&, CastOP&);

private:
    //     struct CastOPKey
    //     {
    //         const Type* from;
    //         const Type* to;
    //
    //         bool operator==(const CastOPKey&) const;
    //     };
    //
    //     struct CastOPKeyHasher
    //     {
    //         size_t operator()(const CastOPKey&) const;
    //     };

    //     using CastOPsMap = UnorderedMap<CastOPKey, CastOP, CastOPKeyHasher>;

    using CompareFn = bool (*)(const Any&, const Any&);

    const Type* type = nullptr;
    AnyStorage anyStorage;
    CompareFn compareFn = nullptr;

    //     template <typename T>
    //     bool CanCastImpl(std::true_type isPointer) const;
    //
    //     template <typename T>
    //     bool CanCastImpl(std::false_type isPointer) const;
    //
    //     template <typename T>
    //     T CastImpl(std::true_type isPointer) const;
    //
    //     template <typename T>
    //     T CastImpl(std::false_type isPointer) const;

    //    using CompareMap = UnorderedMap<const Type*, CompareFn>;
    //    static std::unique_ptr<CompareMap> compareMap;
};

template <typename T>
struct AnyCompare
{
    static bool IsEqual(const Any&, const Any&);
};

template <typename T>
struct AnyCast
{
    static bool CanCast(const Any&);
    static T Cast(const Any&);
};

} // namespace DAVA

#define __Dava_Any__
#include "Base/Private/Any_impl.h"
#include "Base/Private/Any_implCompare.h"
#include "Base/Private/Any_implCast.h"

// TODO
// ...
// #include "Base/Private/AnyCast_impl.h"
