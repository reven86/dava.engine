#pragma once

namespace DAVA
{
class ReflectedType;
class ReflectedObject final
{
public:
    ReflectedObject() = default;
    ReflectedObject(void* ptr, const ReflectedType* reflectedType);

    template <typename T>
    ReflectedObject(T* ptr);

    template <typename T>
    ReflectedObject(const T* ptr);

    bool operator==(const ReflectedObject& other) const;
    bool operator!=(const ReflectedObject& other) const;

    bool IsValid() const;
    bool IsConst() const;

    const ReflectedType* GetReflectedType() const;

    template <typename T>
    T* GetPtr() const;

    void* GetVoidPtr() const;

protected:
    void* ptr = nullptr;
    const ReflectedType* reflectedType = nullptr;

    bool isConst = false;
};
} // namespace DAVA
