#pragma once

namespace DAVA
{
class ReflectedType;
class ReflectionBase
{
public:
    virtual const ReflectedType* GetReflectedType() const = 0;

protected:
    virtual ~ReflectionBase() = default;
};
} // namespace DAVA
