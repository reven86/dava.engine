#pragma once

#include "Base/FastName.h"

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/Common.h"

namespace DAVA
{
class ReflectedType;
namespace TArc
{
class ContextAccessor;
class FieldBinder final
{
public:
    FieldBinder(ContextAccessor* accessor);
    ~FieldBinder();

    void BindField(const FieldDescriptor& fieldDescr, const Function<void(const Any&)>& fn);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
} // namespace TArc
} // namespace DAVA