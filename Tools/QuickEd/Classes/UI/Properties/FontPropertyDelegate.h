#ifndef __FONT_PROPERTY_DELEGATE_H__
#define __FONT_PROPERTY_DELEGATE_H__

#include "BasePropertyDelegate.h"

class FontPropertyDelegate : public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit FontPropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~FontPropertyDelegate();

    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    virtual bool setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    virtual void enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const override;

private slots:
    void addPresetClicked();
    void configurePresetClicked();
    void valueChanged();
};

#endif // __FONT_PROPERTY_DELEGATE_H__s
