#ifndef __ITEMDELEGATEFORPROPERTYENUM_H__
#define __ITEMDELEGATEFORPROPERTYENUM_H__

#include "BasePropertyDelegate.h"
#include <QComboBox>
#include <QAbstractItemDelegate>
#include <QPointer>

class PropertiesTreeItemDelegate;

class EnumPropertyDelegate: public BasePropertyDelegate
{
    Q_OBJECT
public:
    EnumPropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~EnumPropertyDelegate();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;

private slots:
    void OnCurrentIndexChanged();
};

#endif // __ITEMDELEGATEFORPROPERTYENUM_H__
