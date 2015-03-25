#ifndef QTTOOLS_SPYWIDGETINFO_H
#define QTTOOLS_SPYWIDGETINFO_H

#include <QObject>
#include <QPointer>


class QWidget;
class QTimer;

class SpyWidget;
class WidgetModel;


class SpyWidgetInfo
    : public QObject
{
    Q_OBJECT

public:
    explicit SpyWidgetInfo( QObject *parent = nullptr );
    ~SpyWidgetInfo();

    void trackWidget( QWidget *w );

    bool eventFilter( QObject *obj, QEvent *e ) override;

public slots:
    void show();
    void updateInformation();

private:
    QPointer< SpyWidget > view;
    QPointer< QWidget > widget;
    QPointer< QTimer > updateTimer;
    QPointer< WidgetModel > widgetModel;
};


#endif // QTTOOLS_SPYWIDGETINFO_H
