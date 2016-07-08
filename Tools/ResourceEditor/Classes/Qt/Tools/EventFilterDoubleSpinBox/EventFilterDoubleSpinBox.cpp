#include "EventFilterDoubleSpinBox.h"

#include <QChar>
#include <QLocale>

EventFilterDoubleSpinBox::EventFilterDoubleSpinBox(QWidget* parent)
    :
    QDoubleSpinBox(parent)
{
    setKeyboardTracking(false);
}

void EventFilterDoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    QKeyEvent* changedKeyEvent = NULL;
    // Get decimal point specific to current system
    QChar decimalPoint = QLocale().decimalPoint();

    if (event->key() == Qt::Key_Comma && decimalPoint.toLatin1() == Qt::Key_Period)
    {
        // Change comma key event to period key event
        changedKeyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Period, Qt::NoModifier, decimalPoint, 0);
    }
    else if (event->key() == Qt::Key_Period && decimalPoint.toLatin1() == Qt::Key_Comma)
    {
        // Change period key event to comma key event
        changedKeyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Comma, Qt::NoModifier, decimalPoint, 0);
    }

    // Default behaviour
    QDoubleSpinBox::keyPressEvent(changedKeyEvent ? changedKeyEvent : event);
}