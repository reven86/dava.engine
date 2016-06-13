#pragma once
#define DAVAENGINE_ANY__H

#include <typeindex>
#include <string>

#if !defined(__DAVAENGINE_ANDROID__)

#include "Type.h"
#include "AutoStorage.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Any
{
public:
    using Storage = AutoStorage<>;
    using CompareOP = bool (*)(const void*, const void*);
    using LoadOP = void (*)(Storage&, const void* src);
    using SaveOP = void (*)(const Storage&, void* dst);

    template <typename T>
    using NotAny = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, bool>::type;

    struct AnyOP
    {
        CompareOP compare = nullptr;
        LoadOP load = nullptr;
        SaveOP save = nullptr;
    };

    class Exception : public std::runtime_error
    {
    public:
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

    Any() = default;
    ~Any() = default;

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
    static void RegisterOP();

    template <typename T>
    static void RegisterCustomOP(const AnyOP& ops);

private:
    const Type* type = nullptr;
    Storage storage;

    static UnorderedMap<const Type*, AnyOP> operations;
};

} // namespace DAVA

#include "Private/Any_impl.h"

#endif
