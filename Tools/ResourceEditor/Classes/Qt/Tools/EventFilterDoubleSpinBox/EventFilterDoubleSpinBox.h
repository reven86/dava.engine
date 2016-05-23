#ifndef __EVENT_FILTER_DOUBLE_SPIN_BOX_H__
#define __EVENT_FILTER_DOUBLE_SPIN_BOX_H__

#include <QObject>
#include <QDoubleSpinBox>
#include <QWidget>
#include <QKeyEvent>

class EventFilterDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    explicit EventFilterDoubleSpinBox(QWidget* parent = 0);

private:
    void keyPressEvent(QKeyEvent* event);
};

#endif /* defined(__EVENT_FILTER_DOUBLE_SPIN_BOX_H__) */
