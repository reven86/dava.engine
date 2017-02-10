#include "TArc/Controls/Private/ValidatorDelegate.h"

namespace DAVA
{
namespace TArc
{
M::ValidationResult ValidatorDelegate::FixUp(const Any& value) const
{
    return Validate(value);
}
} // namespace TArc
} // namespace DAVA