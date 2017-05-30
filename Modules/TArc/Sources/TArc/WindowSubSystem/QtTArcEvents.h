#pragma once

#include <QtTools/Utils/QtEventIds.h>

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
enum class EventsTable : DAVA::int32
{
    Start = static_cast<DAVA::int32>(QtToolsEventsTable::End),
    End
};
} // namespace TArc
} // namespace DAVA