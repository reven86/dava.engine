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
    friend class WidgetItem;

    Q_OBJECT

private:
    using ItemCache = QMap < QWidget *, QSharedPointer<WidgetItem> > ;

public:
    explicit WidgetModel( QWidget *w );
    ~WidgetModel();

    QWidget* widgetFromIndex( const QModelIndex& index ) const override;
    QModelIndex indexFromWidget( QWidget *w ) const override;

    // QAbstractItemModel
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    QModelIndex	index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	parent( const QModelIndex& index ) const override;

private:
    void rebuildCache();

    QSharedPointer< WidgetItem > root;
    ItemCache cache;

private:
    static void rebuildCacheRecursive( ItemCache& cache, const QSharedPointer<WidgetItem>& item );
};


#endif // QTTOOLS_WIDGETMODEL_H
