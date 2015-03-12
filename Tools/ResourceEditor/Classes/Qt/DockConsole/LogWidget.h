#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QScopedPointer>


namespace Ui
{
    class LogWidget;
};


class QTimer;
class LogModel;
class LogFilterModel;


class LogWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget();

    void SetRegisterLoggerAsLocal(bool isLocal);
    LogModel *Model();

private slots:
    void OnFilterChanged();
    void OnTextFilterChanged(const QString& text);
    void OnCopy();
    void OnClear();
    void DetectAutoScroll();
    void DoAutoScroll();
    void LoadSettings();

private:
    void FillFiltersCombo();
    void SaveSettings();
    bool eventFilter( QObject* watched, QEvent* event );

    QScopedPointer<Ui::LogWidget> ui;
    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    QPointer<QTimer> eventSkipper;
    bool doAutoScroll;
    bool scrollStateDetected;
    bool registerLoggerAsLocal;
};


#endif // __LOGWIDGET_H__
