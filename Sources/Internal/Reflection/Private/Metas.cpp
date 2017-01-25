#include "Reflection/Private/Metas.h"

#include <Debug/DVAssert.h>

namespace DAVA
{
namespace Metas
{
Range::Range(const Any& minValue_, const Any& maxValue_)
    : minValue(minValue_)
    , maxValue(maxValue_)
{
}

Validator::Validator(const TValidationFn& fn_)
    : fn(fn_)
{
    DVASSERT(fn != nullptr);
}

ValidationResult Validator::Validate(const Any& value, const Any& prevValue) const
{
    return fn(value, prevValue);
}

File::File(bool shouldExists_)
    : shouldExists(shouldExists_)
{
}

Directory::Directory(bool shouldExists_)
    : shouldExists(shouldExists_)
{
}

Group::Group(const char* groupName_)
    : groupName(groupName_)
{
}

ValueDescription::ValueDescription(const TValueDescriptorFn& fn_)
    : fn(fn_)
{
    DVASSERT(fn != nullptr);
}

String ValueDescription::GetDescription(const Any& v) const
{
    return fn(v);
}

} // namespace Metas
} // namespace DAVA