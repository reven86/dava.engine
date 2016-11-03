#include "TArc/Core/OperationRegistrator.h"

namespace DAVA
{
namespace TArc
{
OperationID::OperationID()
    : ID(nextOperationID)
{
    nextOperationID++;
}

DAVA::uint32 OperationID::nextOperationID = 0;

} // namespace TArc
} // namespace DAVA