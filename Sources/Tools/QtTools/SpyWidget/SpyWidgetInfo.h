#ifndef QTTOOLS_SPYWIDGETINFO_H
#define QTTOOLS_SPYWIDGETINFO_H

#include <QObject>
#include <QPointer>


class QWidget;
class SpyWidget;


class SpyWidgetInfo
    : public QObject
{
    Q_OBJECT

public:
    explicit SpyWidgetInfo( QObject *parent = nullptr );
    ~SpyWidgetInfo();

    void trackWidget( QWidget *w );

public slots:
    void show();

private:
    QPointer< SpyWidget > view;
    QPointer< QWidget > widget;
};


#endif // QTTOOLS_SPYWIDGETINFO_H
