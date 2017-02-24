#pragma once

#include <Reflection/ReflectedMeta.h>
#include <QValidator>

namespace DAVA
{
namespace TArc
{
QValidator::State ConvertValidationState(M::ValidationResult::eState state);
} // namespace TArc
} // namespace DAVA