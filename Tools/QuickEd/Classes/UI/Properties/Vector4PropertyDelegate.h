#ifndef __QUICKED_VECTOR4_PROPERTY_DELEGATE_H__
#define __QUICKED_VECTOR4_PROPERTY_DELEGATE_H__

#include "BasePropertyDelegate.h"

class Vector4PropertyDelegate: public BasePropertyDelegate
{
    Q_OBJECT
public:
    Vector4PropertyDelegate(PropertiesTreeItemDelegate *delegate);
    virtual ~Vector4PropertyDelegate();
    
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    
private slots:
    void OnEditingFinished();
};


#endif // __QUICKED_VECTOR4_PROPERTY_DELEGATE_H__
