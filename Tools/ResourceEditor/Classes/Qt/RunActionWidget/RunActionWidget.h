#ifndef __RUNACTIONWIDGET_H__
#define __RUNACTIONWIDGET_H__


#include <QWidget>
#include <QScopedPointer>


namespace Ui {
    class RunActionWidget;
}


class RunActionWidget
    : public QWidget
{
    Q_OBJECT

public:
    RunActionWidget( QWidget *parent = NULL );
    ~RunActionWidget();

private:
    QScopedPointer<Ui::RunActionWidget> ui;
};


#endif // __RUNACTIONWIDGET_H__
