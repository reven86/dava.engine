#ifndef QTTOOLS_WIDGETLISTMODEL_H
#define QTTOOLS_WIDGETLISTMODEL_H


#include <QAbstractListModel>
#include <QWidgetList>


class WidgetListModel
    : public QAbstractListModel
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
    explicit WidgetListModel( QObject *parent = nullptr );
    ~WidgetListModel();

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    QModelIndex	index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex	parent( const QModelIndex& index ) const override;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

    void setWidgetList( const QWidgetList& widgetList = QWidgetList() );

private slots:
    void onWidgetDestroyed();

private:
    QVariant textDataForColumn( const QModelIndex& index ) const;

    QWidgetList widgets;
};


Q_DECLARE_METATYPE( WidgetListModel::Columns );


#endif // QTTOOLS_WIDGETLISTMODEL_H
