#pragma once
#define DAVAENGINE_TYPE__H

#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>

#define DAVA_DECLARE_TYPE_INITIALIZER                       \
protected:                                                  \
    template <typename T> \
    friend struct DAVA::TypeDetails::TypeInitializerRunner; \
    friend class DAVA::Type;                                \
    static void __TypeInitializer();

#define DAVA_TYPE_INITIALIZER(T)                            \
    void T::__TypeInitializer()

#define DAVA_TYPE_REGISTER(T)                               \
    DAVA::Type::Instance<T>()->RegisterPermanetName(#T)

namespace DAVA
{
class ReflectionDB;
class Type
{
    friend class ReflectionDB;

public:
    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    void RegisterPermanetName(const std::string& permanentName) const;

    size_t GetSize() const;
    const char* GetName() const;
    const char* GetPermanentName() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;

    const Type* Decay() const;
    const Type* Deref() const;
    const ReflectionDB* GetReflectionDB() const;

    template <typename T>
    static const Type* Instance();

    static const Type* Instance(const std::string& permanentName);

protected:
    Type() = default;

    const char* name = nullptr;
    std::string permanentName;
    size_t size = 0;

    bool isConst = false;
    bool isPointer = false;
    bool isReference = false;

    const Type* derefType = nullptr;
    const Type* decayType = nullptr;
    mutable ReflectionDB* reflectionDb = nullptr;

    static std::unordered_map<std::string, const Type*> nameToTypeMap;
};

inline size_t Type::GetSize() const
{
    return size;
}
inline const char* Type::GetName() const
{
    return name;
}
inline bool Type::IsConst() const
{
    return isConst;
}
inline bool Type::IsPointer() const
{
    return isPointer;
}
inline bool Type::IsReference() const
{
    return isReference;
}
inline const Type* Type::Decay() const
{
    return decayType;
}
inline const Type* Type::Deref() const
{
    return derefType;
}
inline const ReflectionDB* Type::GetReflectionDB() const
{
    return reflectionDb;
}

} // namespace DAVA

#include "Private/Type_impl.h"
