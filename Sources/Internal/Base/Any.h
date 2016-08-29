#pragma once

#include <typeindex>
#include <string>

#include "BaseTypes.h"
#include "Type.h"
#include "Private/AutoStorage.h"

// #define ANY_EXPERIMENTAL_CAST_IMPL

namespace DAVA
{
class Any final
{
public:
    struct Exception;

    using AnyStorage = AutoStorage<>;
    using LoadOP = void (*)(AnyStorage&, const void* src);
    using StoreOP = void (*)(const AnyStorage&, void* dst);
    using CompareOP = bool (*)(const void* a, const void* b);
    using CastOP = Any (*)(const Any& from);

    struct AnyOPs
    {
        LoadOP load = nullptr;
        StoreOP store = nullptr;
        CompareOP compare = nullptr;
    };

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

    void LoadValue(const Type* type, void* data);
    void StoreValue(void* data, size_t size) const;

    Any& operator=(Any&&);
    Any& operator=(const Any&) = default;

    bool operator==(const Any&) const;
    bool operator!=(const Any&) const;

    template <typename T>
    static void RegisterDefaultOPs();

    template <typename T>
    static void RegisterOPs(AnyOPs&& ops);

    template <typename T1, typename T2>
    static void RegisterDefaultCastOP();

    template <typename T1, typename T2>
    static void RegisterCastOP(CastOP&, CastOP&);

private:
    struct CastOPKey
    {
        const Type* from;
        const Type* to;

        bool operator==(const CastOPKey&) const;
    };

    struct CastOPKeyHasher
    {
        size_t operator()(const CastOPKey&) const;
    };

    using AnyOPsMap = UnorderedMap<const Type*, AnyOPs>;
    using CastOPsMap = UnorderedMap<CastOPKey, CastOP, CastOPKeyHasher>;

    const Type* type = nullptr;
    AnyStorage anyStorage;

    template <typename T>
    const T& GetImpl() const;

    template <typename T>
    bool CanCastImpl(std::true_type isPointer) const;

    template <typename T>
    bool CanCastImpl(std::false_type isPointer) const;

    template <typename T>
    T CastImpl(std::true_type isPointer) const;

    template <typename T>
    T CastImpl(std::false_type isPointer) const;

    static std::unique_ptr<AnyOPsMap> anyOPsMap;
    static std::unique_ptr<CastOPsMap> castOPsMap;
};

struct Any::Exception : public std::runtime_error
{
    enum ErrorCode
    {
        BadGet,
        BadCast,
        BadOperation,
        BadSize
    };

    Exception(ErrorCode code, const std::string& message);
    Exception(ErrorCode code, const char* message);

    ErrorCode errorCode;
};

} // namespace DAVA

#define __Dava_Any__
#include "Base/Private/Any_impl.h"

// TODO
// ...
// #include "Base/Private/AnyCast_impl.h"
