#ifndef QTTOOLS_WIDGETMODEL_H
#define QTTOOLS_WIDGETMODEL_H


#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>


class WidgetModel
    : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Columns
    {
        TITLE,
        CLASSNAME,
        OBJECTNAME,

        COLUMN_COUNT    // Last
    };

public:
    explicit WidgetModel( QObject *parent );
    ~WidgetModel();

    void trackWidget( QWidget *w );

    // QObject
    bool eventFilter( QObject *obj, QEvent *e ) override;

    // QAbstractItemModel
    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    bool canFetchMore( const QModelIndex& parent ) const override;
    void fetchMore( const QModelIndex& parent ) override;
    bool hasChildren( const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	parent( const QModelIndex& index ) const override;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

private:
    QPointer< QWidget > root;
};


Q_DECLARE_METATYPE( WidgetModel::Columns );


#endif // QTTOOLS_WIDGETMODEL_H
