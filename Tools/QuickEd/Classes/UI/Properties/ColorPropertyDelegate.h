#ifndef __ITEMDELEGATEFORCOLOR_H__
#define __ITEMDELEGATEFORCOLOR_H__

#include "BasePropertyDelegate.h"

class QToolButton;

class ColorPropertyDelegate: public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit ColorPropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~ColorPropertyDelegate();

    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    virtual void enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const override;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    virtual bool setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
private slots:
    void OnChooseColorClicked();
    void OnEditingFinished();
};


#endif // __ITEMDELEGATEFORCOLOR_H__
