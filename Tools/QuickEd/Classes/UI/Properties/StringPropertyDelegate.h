#ifndef __ITEMDELEGATEFORSTRING_H__
#define __ITEMDELEGATEFORSTRING_H__

#include "BasePropertyDelegate.h"

class StringPropertyDelegate: public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit StringPropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~StringPropertyDelegate();

    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    virtual bool setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

private slots:
    void OnEditingFinished();
};


#endif // __ITEMDELEGATEFORSTRING_H__
