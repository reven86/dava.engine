#pragma once

#include <QtTools/Utils/QtEventIds.h>

#include <Base/BaseTypes.h>

#include <QtEvents>

namespace DAVA
{
namespace TArc
{
enum class EventsTable : DAVA::int32
{
    FocusInToParent = static_cast<DAVA::int32>(QtToolsEventsTable::End),
    FocusOutToParent,
    End
};

class FocusToParentEvent : public QEvent
{
public:
    FocusToParentEvent(QEvent::Type type, const QFocusEvent& sourceEvent);

    const QFocusEvent sourceEvent;
};
} // namespace TArc
} // namespace DAVA