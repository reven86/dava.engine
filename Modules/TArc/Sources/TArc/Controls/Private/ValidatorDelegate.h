#pragma once

#include <Reflection/ReflectedMeta.h>
#include <Base/Any.h>
#include <QString>

namespace DAVA
{
namespace TArc
{
class ValidatorDelegate
{
public:
    virtual M::ValidationResult FixUp(const Any& value) const;
    virtual M::ValidationResult Validate(const Any& value) const = 0;
    virtual void ShowHint(const QString& message) = 0;
};

} // namespace TArc
} // namespace DAVA