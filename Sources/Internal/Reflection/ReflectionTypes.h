#pragma once

namespace DAVA
{
class Type;
class ReflectedType;

class ReflectionBase
{
public:
    virtual const ReflectedType* GetReflectedType() const = 0;
};

enum class ReflectionCtorPolicy
{
    ByValue,
    ByPointer
};

struct ReflectionCaps
{
    bool canAddField = false;
    bool canInsertField = false;
    bool canRemoveField = false;
    bool canCreateFieldValue = false;
    bool hasDynamicStruct = false;
    bool hasFlatStruct = false;
    const Type* flatKeyType = nullptr;
    const Type* flatValueType = nullptr;
};

} // namespace DAVA
