#pragma once

#include <Reflection/MetaObjects.h>
#include <Base/Any.h>
#include <QString>

namespace DAVA
{
namespace TArc
{
class ValidatorDelegate
{
public:
    virtual M::ValidatorResult Validate(const Any& value) const = 0;
    virtual void ShowHint(const QString& message) = 0;
};

} // namespace TArc
} // namespace DAVA