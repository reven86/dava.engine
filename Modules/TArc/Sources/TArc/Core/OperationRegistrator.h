#pragma once

#include "TArc/Core/OperationRegistrator.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace TArc
{
class OperationID
{
public:
    OperationID();

    const DAVA::uint32 ID;

private:
    static DAVA::uint32 nextOperationID;
};
} // namespace TArc
} // namespace DAVA

#define DECLARE_OPERATION_ID(name) \
    extern DAVA::TArc::OperationID name

#define IMPL_OPERATION_ID(name) \
    DAVA::TArc::OperationID name
