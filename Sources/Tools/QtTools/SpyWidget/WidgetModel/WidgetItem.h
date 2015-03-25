#ifndef QTTOOLS_WIDGETITEM_H
#define QTTOOLS_WIDGETITEM_H


#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QList>


class QWidget;
class QAbstractItemModel;
class WidgetModel;


class WidgetItem
    : public QObject
{
    friend class QSharedPointer < WidgetItem > ;
    friend class WidgetModel;

    Q_OBJECT

private:
    using ItemList = QList < QSharedPointer< WidgetItem > > ;

public:
    ~WidgetItem();

    bool eventFilter( QObject *obj, QEvent *e ) override;

    QSharedPointer< WidgetItem > getParent() const;

private:
    explicit WidgetItem( QWidget *w );
    void rebuildChildren();
    void onChildAdd( QWidget *w );
    void onChildRemove( QWidget *w );

    QPointer< QWidget > widget;
    QSharedPointer< WidgetItem > parentItem;
    QWeakPointer < WidgetItem > self;
    QPointer< QAbstractItemModel > model;
    ItemList children;

public:
    static QSharedPointer < WidgetItem > create( QWidget *w );
};


#endif // QTTOOLS_WIDGETITEM_H
