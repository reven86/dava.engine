#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QTime>
#include "ui_LogWidget.h"

namespace Ui
{
    class LogWidget;
};


class QTimer;
class LogModel;
class LogFilterModel;


class LogWidget
    : public QWidget, public Ui::LogWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget() = default;

    LogModel *Model();
public slots:
    void OnFilterChanged();

private slots:
    void OnTextFilterChanged(const QString& text);
    void OnCopy();
    void OnClear();
    void DetectAutoScroll();
    void DoAutoScroll();

private:
    void FillFiltersCombo();
    bool eventFilter( QObject* watched, QEvent* event ) override;

    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    QPointer<QTimer> eventSkipper;
    bool doAutoScroll;
    bool scrollStateDetected;
    QTime time;
};


#endif // __LOGWIDGET_H__
