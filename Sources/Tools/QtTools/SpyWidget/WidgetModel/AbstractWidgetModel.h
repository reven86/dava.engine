#ifndef QTTOOLS_ABSTRACTWIDGETMODEL_H
#define QTTOOLS_ABSTRACTWIDGETMODEL_H


#include <QAbstractItemModel>
#include <QWidget>


class AbstractWidgetModel
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
    explicit AbstractWidgetModel( QObject *parent = nullptr );
    ~AbstractWidgetModel();

    virtual QWidget *widgetFromIndex( const QModelIndex& index ) const = 0;

    int columnCount( const QModelIndex& parent ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;

    QVariant data( const QModelIndex& index, int role ) const;
    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

protected:
    QVariant textDataForColumn( QWidget *w, int column ) const;
};


Q_DECLARE_METATYPE( AbstractWidgetModel::Columns );


#endif // QTTOOLS_ABSTRACTWIDGETMODEL_H
