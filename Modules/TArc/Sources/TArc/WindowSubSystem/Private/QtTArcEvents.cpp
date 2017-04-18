#include "TArc/WindowSubSystem/QtTArcEvents.h"

namespace DAVA
{
namespace TArc
{
FocusToParentEvent::FocusToParentEvent(QEvent::Type type, const QFocusEvent& sourceEvent_)
    : QEvent(type)
    , sourceEvent(sourceEvent_)
{
}

} // namespace TArc
} // namespace DAVA