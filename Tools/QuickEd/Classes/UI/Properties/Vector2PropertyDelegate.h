#ifndef __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
#define __PROPERTIESTREEQVARIANTITEMDELEGATE_H__

#include <QWidget>
#include "BasePropertyDelegate.h"

class Vector2PropertyDelegate: public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit Vector2PropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~Vector2PropertyDelegate();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    virtual void setEditorData( QWidget * editor, const QModelIndex & index) const override;
    virtual bool setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

private slots:
    void OnEditingFinished();
};

#endif // __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
