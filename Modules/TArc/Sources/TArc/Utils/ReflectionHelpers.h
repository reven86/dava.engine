#pragma once

#include <Reflection/Reflection.h>
#include <Functional/Function.h>

namespace DAVA
{
namespace TArc
{
void ForEachField(const Reflection& r, const Function<void(Reflection::Field&& field)>& fn);
} // namespace TArc
} // namespace DAVA