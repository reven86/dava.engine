#include "TArc/WindowSubSystem/Private/HierarchyEventsNotifier.h"
#include "TArc/WindowSubSystem/QtTArcEvents.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <Engine/PlatformApi.h>

#include <QApplication>

namespace DAVA
{
namespace TArc
{
HierarchyEventsNotifier::HierarchyEventsNotifier(QObject* parent)
    : QObject(parent)
{
}

bool HierarchyEventsNotifier::eventFilter(QObject* obj, QEvent* e)
{
    SCOPED_VALUE_GUARD(bool, isInFiltering, true, false);

    DAVA::Array<QEvent::Type, 2> hierarchyVisitTypes =
    {
      QT_EVENT_TYPE(EventsTable::FocusInToParent),
      QT_EVENT_TYPE(EventsTable::FocusOutToParent),
    };

    QEvent::Type eventType = e->type();
    bool needVisitHierarchy = std::any_of(hierarchyVisitTypes.begin(), hierarchyVisitTypes.end(), [eventType](QEvent::Type t)
                                          {
                                              return eventType == t;
                                          });

    QApplication* app = PlatformApi::Qt::GetApplication();
    if (needVisitHierarchy)
    {
        QWidget* w = qobject_cast<QWidget*>(obj);
        while (w != nullptr)
        {
            app->notify(w, e);
            w = w->parentWidget();
        }
    }

    return false;
}

} // namespace TArc
} // namespace DAVA