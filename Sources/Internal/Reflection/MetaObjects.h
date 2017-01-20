#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/EnumMap.h"

namespace DAVA
{
namespace Metas
{
class ReadOnly
{
};

class Range
{
public:
    Range(const Any& minValue, const Any& maxValue);
    const Any minValue;
    const Any maxValue;
};

struct ValidationResult
{
    enum class eState
    {
        Invalid,
        Intermediate,
        Valid
    };

    eState state;
    Any fixedValue;
    String message;
};

using TValidationFn = ValidationResult (*)(const Any& value, const Any& prevValue);

class Validator
{
public:
    Validator(const TValidationFn& fn);

    ValidationResult Validate(const Any& value, const Any& prevValue) const;

private:
    TValidationFn fn;
};

class Enum
{
public:
    virtual const EnumMap* GetEnumMap() const = 0;
};

template <typename T>
class EnumT : public Enum
{
public:
    const EnumMap* GetEnumMap() const override;
};

template <typename T>
inline const EnumMap* EnumT<T>::GetEnumMap() const
{
    return GlobalEnumMap<T>::Instance();
}

class Flags
{
public:
    virtual const EnumMap* GetFlagsMap() const = 0;
};

template <typename T>
class FlagsT : public Flags
{
public:
    const EnumMap* GetFlagsMap() const override;
};

template <typename T>
inline const EnumMap* FlagsT<T>::GetFlagsMap() const
{
    return GlobalEnumMap<T>::Instance();
}

class File
{
public:
    File(bool shouldExists = true);

    const bool shouldExists;
};

class Directory
{
public:
    Directory(bool shouldExists = true);

    const bool shouldExists;
};

class Group
{
public:
    Group(const char* groupName);
    const char* groupName;
};

using TValueDescriptorFn = String (*)(const Any&);
class ValueDescription
{
public:
    ValueDescription(const TValueDescriptorFn& fn);

    String GetDescription(const Any& v) const;

private:
    TValueDescriptorFn fn;
};

} // namespace Mates
} // namespace DAVA