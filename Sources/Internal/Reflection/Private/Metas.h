#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/EnumMap.h"
#include "Base/GlobalEnum.h"

namespace DAVA
{
namespace Metas
{
/** Defines that Reflected Field can't be changed */
class ReadOnly
{
};

/**
    Defines valid range of value
    Control will try to cast minValue and maxValue to control specific type T
    If some of bound couldn't be casted to T, this bound will be equal std::numeric_limits<T>::min\max.
*/
class Range
{
public:
    Range(const Any& minValue, const Any& maxValue, const Any& step);
    const Any minValue;
    const Any maxValue;
    const Any step;
};

/** Validation result */
struct ValidationResult
{
    /** This enum type defines the state in which a validated value can exist */
    enum class eState
    {
        /** Inputted value isn't valid and should be discarded */
        Invalid,
        /**
            Inputted value can be valid. For example value 4 invalid for Range(10, 99)
            but we should allow user continue editing because value 40 is valid
        */
        Intermediate,
        /** Inputted value is completely valid */
        Valid
    };

    /**
        \anchor validator_state
        Current state of validator
    */
    eState state;
    /**
        Validator can change value that user inputted
        Control will use this value only if \ref validator_state "state" equal Valid or Intermediate
    */
    Any fixedValue;
    /**
        Hint text for user, that describe why inputted value isn't valid
        Control use this only if \ref validator_state "state" equal Invalid
    */
    String message;
};

using TValidationFn = ValidationResult (*)(const Any& value, const Any& prevValue);

/** Validator for Reflected Field's value */
class Validator
{
public:
    Validator(const TValidationFn& fn);

    /**
        Validate value
        \arg \c value Inputted value by user
        \arg \c current value of Reflected Field
    */
    ValidationResult Validate(const Any& value, const Any& prevValue) const;

private:
    TValidationFn fn;
};

/** Base class for all mate Enum types */
class Enum
{
public:
    virtual ~Enum() = default;
    /** Returns EnumMap that describe values of Enum */
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

/** Base class for all mate Enum types */
class Flags
{
public:
    virtual ~Flags() = default;
    /** Returns EnumMap that describe values of bitfield Enum */
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

/** Defines that value of Reflected Field should be File */
class File
{
public:
    /** \arg \c shouldExists defines rule should file exists of not */
    File(bool shouldExists = true);

    const bool shouldExists;
};

/** Defines that value of Reflected Field should be Directory */
class Directory
{
public:
    /** \arg \c shouldExists defines rule should directory exists of not */
    Directory(bool shouldExists = true);

    const bool shouldExists;
};

/** Defines logical group of set of Reflected Fields under the same name */
class Group
{
public:
    /** \arg \c groupName name of logical group */
    Group(const char* groupName);
    const char* groupName;
};

using TValueDescriptorFn = String (*)(const Any&);

/** Defines function that can provide string representation of value. */
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