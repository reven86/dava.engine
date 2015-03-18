#ifndef QTTOOLS_WIDGETMODEL_H
#define QTTOOLS_WIDGETMODEL_H

#include <QAbstractItemModel>


class WidgetModel
    : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit WidgetModel( QObject *parent = nullptr );
    ~WidgetModel();
};


#endif // QTTOOLS_WIDGETMODEL_H
