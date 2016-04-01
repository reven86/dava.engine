#pragma once
#define DAVAENGINE_AUTO_STORAGE__H

#include <array>
#include <memory>
#include <cassert>
#include <type_traits>

namespace DAVA
{
template <size_t Count = 1>
class AutoStorage final
{
    using StorageT = std::array<void*, Count>;
    using SharedT = std::shared_ptr<void>;

    static_assert(Count > 0, "Size should be > 0");
    static_assert(sizeof(StorageT) >= sizeof(void*) && sizeof(StorageT) >= sizeof(SharedT), "Wrong Autostorage size");

public:
    template <typename T>
    using StorableType = typename std::decay<T>::type;

    AutoStorage();
    ~AutoStorage();

    AutoStorage(AutoStorage&&);
    AutoStorage(const AutoStorage&);

    bool IsEmpty() const;
    bool IsSimple() const;

    void Clear();
    void Swap(AutoStorage& value);

    template <typename T>
    void SetSimple(T&& value);

    template <typename T>
    void SetShared(T&& value);

    template <typename T>
    void SetAuto(T&& value);

    template <typename T>
    const T& GetSimple() const;

    template <typename T>
    const T& GetShared() const;

    template <typename T>
    const T& GetAuto() const;

    const void* GetData() const;

    AutoStorage& operator=(const AutoStorage& value);
    AutoStorage& operator=(AutoStorage&& value);

    bool operator==(const AutoStorage&) const = delete;
    bool operator!=(const AutoStorage&) const = delete;

    template <typename T>
    static bool TypeIsSimple()
    {
        return (sizeof(T) <= sizeof(StorageType))
        && std::is_trivially_destructible<T>::value
        && std::is_trivially_copy_constructible<T>::value
        && std::is_trivially_copy_assignable<T>::value;
    }

private:
    enum class StorageType
    {
        Empty,
        Simple,
        Shared
    };

    StorageT storage;
    StorageType type = StorageType::Empty;

    void DoCopy(const AutoStorage& value);
    void DoMove(AutoStorage&& value);

    inline SharedT* SharedPtr() const
    {
        return reinterpret_cast<SharedT*>(const_cast<void**>(storage.data()));
    }
};

} // namespace DAVA

#include "Private/AutoStorage_impl.h"
