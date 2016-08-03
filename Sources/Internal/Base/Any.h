#pragma once

#include <typeindex>
#include <string>

#include "BaseTypes.h"
#include "Type.h"
#include "Private/AutoStorage.h"

namespace DAVA
{
class Any final
{
public:
    using AnyStorage = AutoStorage<>;

    using CompareOP = bool (*)(const void*, const void*);
    using LoadOP = void (*)(AnyStorage&, const void* src);
    using SaveOP = void (*)(const AnyStorage&, void* dst);
    using CastOP = void (*)(const void* src, void* dst);

    template <typename T>
    using NotAny = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, bool>::type;

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

    inline Any() = default;
    inline ~Any() = default;

    Any(Any&&);
    Any(const Any&) = default;

    template <typename T>
    Any(T&& value, NotAny<T> = true);

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
    void SaveValue(void* data, size_t size) const;

    Any& operator=(Any&&);
    Any& operator=(const Any&) = default;

    bool operator==(const Any&) const;
    bool operator!=(const Any&) const;

    template <typename T>
    static void RegisterDefaultOPs(const LoadOP& lop, const SaveOP& sop, const CompareOP& cop);

    // TODO:
    // implement castOP
    // ...
    //
    // template <typename From, typename To>
    // static void RegisterCastOP(Any::CastOP &castOP);

private:
    struct AnyOPs
    {
        CompareOP compare = nullptr;
        LoadOP load = nullptr;
        SaveOP save = nullptr;

        // TODO:
        // implement castOP
        // ...
        // Map<const Type*, CastOP> casts;
    };

    const Type* type = nullptr;
    AnyStorage anyStorage;

    // TODO:
    // for plugins here should be pointer
    // ...
    static UnorderedMap<const Type*, AnyOPs>* operations;
};

} // namespace DAVA

#define __Dava_Any__
#include "Base/Private/Any_impl.h"

// TODO
// ...
// #include "Base/Private/AnyCast_impl.h"
