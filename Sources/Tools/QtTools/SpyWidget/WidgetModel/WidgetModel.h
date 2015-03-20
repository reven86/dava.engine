#ifndef QTTOOLS_WIDGETMODEL_H
#define QTTOOLS_WIDGETMODEL_H


#include <QPointer>
#include <QWidget>

#include "AbstractWidgetModel.h"


class WidgetModel
    : public AbstractWidgetModel
{
    Q_OBJECT

public:
    explicit WidgetModel( QObject *parent );
    ~WidgetModel();

    void trackWidget( QWidget *w );

    // QObject
    bool eventFilter( QObject *obj, QEvent *e ) override;

    // QAbstractItemModel
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    bool canFetchMore( const QModelIndex& parent ) const override;
    void fetchMore( const QModelIndex& parent ) override;
    bool hasChildren( const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	parent( const QModelIndex& index ) const override;

private:
    QPointer< QWidget > root;
};


#endif // QTTOOLS_WIDGETMODEL_H
