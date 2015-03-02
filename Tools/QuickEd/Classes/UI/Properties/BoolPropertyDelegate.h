#ifndef __BOOL_PROPERTY_DELEGATE_H__
#define __BOOL_PROPERTY_DELEGATE_H__

#include "BasePropertyDelegate.h"
class PropertiesTreeItemDelegate;

class BoolPropertyDelegate : public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit BoolPropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~BoolPropertyDelegate();

    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    virtual bool setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
private slots:
    void OnCurrentIndexChanged();
};


#endif // __BOOL_PROPERTY_DELEGATE_H__
