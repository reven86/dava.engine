#pragma once

namespace DAVA
{
class ReflectedType;
class ReflectedBase
{
public:
    virtual const ReflectedType* GetReflectedType() const = 0;
};

} // namespace DAVA
