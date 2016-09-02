#pragma once

#include <typeindex>
#include <string>

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Base/Exception.h"
#include "Base/Private/AutoStorage.h"

namespace DAVA
{
/// \brief  The class Any is a type-safe container for single values of any type. Implementations is encouraged to
///         avoid dynamic allocations for small objects, but such an optimization may only be applied to types that
///         for which std::is_nothrow_move_constructible returns true. This class cannot be inherited.
class Any final
{
public:
    using AnyStorage = AutoStorage<>;

    template <typename T>
    using NotAny = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, bool>::type;

    Any() = default;
    ~Any() = default;
    Any(const Any&) = default;

    Any(Any&& any);

    /// \brief Constructor.
    /// \param [in,out] value   The value.
    /// \param          notAny  (Optional) Used to prevent creating Any from other Any. Shouldn't be specified by user.
    template <typename T>
    Any(T&& value, NotAny<T> notAny = true);

    /// \brief Swaps this with the given Any.
    /// \param [in,out] any Any to swap with.
    void Swap(Any& any);

    /// \brief Checks if Any is empty.
    /// \return true if empty, false if not.
    bool IsEmpty() const;

    /// \brief Clears Any to its empty state.
    void Clear();

    /// \brief Gets the type of contained value.
    /// \return null if it Any is empty, else the contained value type.
    const Type* GetType() const;

    /// \brief Determine if value with specified type T can get be from Any.
    /// \return true if we can be get, false if not.
    template <typename T>
    bool CanGet() const;

    /// \brief Gets the value with specified type T.
    /// \exception  DAVA::Exception value with specified T can't be get due to type mismatch with contained value.
    /// \return A reference to a const T.
    template <typename T>
    const T& Get() const;

    /// \brief Gets the value with specified type T. If such value can't be get defaultValue will be returned.
    /// \param  defaultValue    The value to return if getting value with specified T can't be done.
    /// \return A reference to a const T.
    template <typename T>
    const T& Get(const T& defaultValue) const;

    void Set(Any&& any);
    void Set(const Any& any);

    /// \brief Sets the value. It will be copied|moved into Any depending on lvalue|rvalue.
    /// \param [in,out] value   The value to set.
    /// \param          notAny  (Optional) Used to prevent creating Any from other Any. Shouldn't be specified by user.
    template <typename T>
    void Set(T&& value, NotAny<T> notAny = true);

    /// \brief Determine if contained value can be cast to specified T.
    /// \return true if we can be casted, false if not.
    /// \see AnyCast
    template <typename T>
    bool CanCast() const;

    /// \brief Casts contained value into value with specified type T.
    /// \exception  DAVA::Exception contained value can't be casted to value with specified type T.
    /// \return value with specified type T.
    /// \see AnyCast
    template <typename T>
    T Cast() const;

    /// \brief  Loads value into Any from specified memory location with specified Type. Loading can be done only from
    ///         types for which Type::IsTrivial is true.
    /// \param [in,out] data    Pointer on source memory, from where value should be loaded.
    /// \param          type    The type of the loading value.
    void LoadValue(void* data, const Type* type);

    /// \brief  Stores contained value into specified memory location. Storing can
    ///          be done only for values whose type Type::IsTrivial is true.
    /// \param [in,out] data    Pointer on destination memory, where contained value should be stored.
    /// \param          size    The size of the destination memory.
    void StoreValue(void* data, size_t size) const;

    Any& operator=(Any&&);
    Any& operator=(const Any&) = default;

    /// \brief  Equality operator. Two Any objects can be equal only if they both contain values of the same Any::type.
    ///         Values of type for which Type::IsTrivial is true are compared by ::std::memcmp function. Values with
    ///         other types can be compared only if AnyCompate class has specialization for thous types.
    /// \exception  DAVA::Exception there is no appropriate compare operation for contaited values.
    /// \sa AnyCompare
    bool operator==(const Any&) const;

    /// \brief Inequality operator.
    /// \see Any::operator==
    bool operator!=(const Any&) const;

private:
    using CompareFn = bool (*)(const Any&, const Any&);

    const Type* type = nullptr;
    AnyStorage anyStorage;
    CompareFn compareFn = nullptr;
};

/// \brief any compare.
template <typename T>
struct AnyCompare
{
    static bool IsEqual(const Any&, const Any&);
};

/// \brief any cast.
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
