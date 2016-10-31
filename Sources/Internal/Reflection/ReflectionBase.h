#pragma once

namespace DAVA
{
class ReflectedType;
class ReflectionBase
{
public:
    virtual const ReflectedType* GetReflectedType() const = 0;

private:
    virtual ~ReflectionBase() = default;
};

} // namespace DAVA
