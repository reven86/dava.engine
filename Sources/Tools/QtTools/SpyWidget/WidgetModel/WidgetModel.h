#ifndef QTTOOLS_WIDGETMODEL_H
#define QTTOOLS_WIDGETMODEL_H

#include <QAbstractItemModel>


class WidgetModel
    : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit WidgetModel( QObject *parent );
    ~WidgetModel();

    bool eventFilter( QObject *obj, QEvent *e ) override;

private:
    bool applicationEventFilterInternal( QObject *obj, QEvent *e );
    bool widgetEventFilterInternal( QObject *obj, QEvent *e );

    bool isGlobal;
};


#endif // QTTOOLS_WIDGETMODEL_H
