#ifndef QTTOOLS_WIDGETLISTMODEL_H
#define QTTOOLS_WIDGETLISTMODEL_H


#include <QWidgetList>

#include "AbstractWidgetModel.h"


class WidgetListModel
    : public AbstractWidgetModel
{
    Q_OBJECT

public:
    explicit WidgetListModel( QObject *parent = nullptr );
    ~WidgetListModel();

    QWidget *widgetFromIndex( const QModelIndex& index ) const override;
    void setWidgetList( const QWidgetList& widgetList = QWidgetList() );

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    QModelIndex	index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	parent( const QModelIndex& index ) const override;

private slots:
    void onWidgetDestroyed();

private:
    QWidgetList widgets;
};


#endif // QTTOOLS_WIDGETLISTMODEL_H
