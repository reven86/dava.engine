#ifndef QTTOOLS_WIDGETMODEL_H
#define QTTOOLS_WIDGETMODEL_H


#include <QPointer>
#include <QWidget>
#include <QSharedPointer>

#include "AbstractWidgetModel.h"


class WidgetItem;


class WidgetModel
    : public AbstractWidgetModel
{
    Q_OBJECT

public:
    explicit WidgetModel( QWidget *w );
    ~WidgetModel();

    QWidget* widgetFromIndex( const QModelIndex& index ) const override;

    // QAbstractItemModel
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    QModelIndex	index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	parent( const QModelIndex& index ) const override;

private:
    QSharedPointer< WidgetItem > root;
};


#endif // QTTOOLS_WIDGETMODEL_H
