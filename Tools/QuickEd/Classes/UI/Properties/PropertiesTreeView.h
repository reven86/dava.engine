#ifndef __PROPERTIESTREEVIEW_H__
#define __PROPERTIESTREEVIEW_H__

#include <QTreeView>

class PropertiesTreeView: public QTreeView
{
    Q_OBJECT
public:
    explicit PropertiesTreeView(QWidget *parent = NULL);
    ~PropertiesTreeView();

    virtual void drawRow(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex & index) const override;
};

#endif // __PROPERTIESTREEVIEW_H__
