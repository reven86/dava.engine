#ifndef QTTOOLS_SPYWIDGETINFO_H
#define QTTOOLS_SPYWIDGETINFO_H


#include <QObject>
#include <QPointer>
#include <QModelIndex>


class QWidget;
class QTimer;

class SpyWidget;
class WidgetModel;
class WidgetHighlightModel;


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

private slots:
    void onChangeWidget( const QModelIndex& index );
    void onSelectWidget();

private:
    void selectWidget( QWidget *w );

    QPointer< SpyWidget > view;
    QPointer< QWidget > widget;
    QPointer< QTimer > updateTimer;
    QPointer< WidgetModel > widgetModel;
    QPointer< WidgetHighlightModel > widgetHighlightModel;
};


#endif // QTTOOLS_SPYWIDGETINFO_H
