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
    using AnyStorage = AutoStorage<>;

    using LoadOP = void (*)(AnyStorage&, const void* src);
    using StoreOP = void (*)(const AnyStorage&, void* dst);
    using CompareOP = bool (*)(const void* a, const void* b);

    struct AnyOPs
    {
        CompareOP compare = nullptr;
        LoadOP load = nullptr;
        StoreOP store = nullptr;
    };

    using AnyOPsMap = UnorderedMap<const Type*, AnyOPs>;

#ifdef ANY_EXPERIMENTAL_CAST_IMPL
    struct CastOPKey
    {
        const Type* from;
        const Type* to;

        size_t operator()(const CastOPKey& key) const
        {
            return std::hash<const Type*>()(key.from) ^ std::hash<const Type*>()(key.from);
        }
    };

    using CastOP = void (*)(const void* from, void* to);
    using CastOPsMap = UnorderedMap<CastOPKey, CastOP, CastOPKey>;
#endif

    struct Exception : public std::runtime_error
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

    template <typename T>
    using NotAny = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, bool>::type;

    inline Any() = default;
    inline ~Any() = default;

    template <typename T>
    Any(T&& value, NotAny<T> = true);

    Any(Any&&);
    Any(const Any&) = default;

    void Swap(Any&);

    bool IsEmpty() const;
    void Clear();

    const Type* GetType() const;

    template <typename T>
    bool CanGet() const;

    template <typename T>
    const T& Get() const;

    template <typename T>
    const T& Get(const T& defaultValue) const;

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

#ifdef ANY_EXPERIMENTAL_CAST_IMPL
    template <typename From, typename To>
    static void RegisterCastOP(CastOP& castOP);
#endif

private:
    const Type* type = nullptr;
    AnyStorage anyStorage;

    static std::unique_ptr<AnyOPsMap> anyOPsMap;

#ifdef ANY_EXPERIMENTAL_CAST_IMPL
    static std::unique_ptr<CastOPsMap> castOPsMap;
#endif
};

} // namespace DAVA

#define __Dava_Any__
#include "Base/Private/Any_impl.h"

// TODO
// ...
// #include "Base/Private/AnyCast_impl.h"
