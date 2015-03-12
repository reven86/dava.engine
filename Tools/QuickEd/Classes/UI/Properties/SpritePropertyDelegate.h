#ifndef __SPRITE_PROPERTY_DELEGATE_H__
#define __SPRITE_PROPERTY_DELEGATE_H__

#include "BasePropertyDelegate.h"

class SpritePropertyDelegate : public BasePropertyDelegate
{
    Q_OBJECT
public:
    explicit SpritePropertyDelegate(PropertiesTreeItemDelegate *delegate);
    ~SpritePropertyDelegate();

    virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    virtual bool setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    virtual void enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const;

private slots:
    void openFileDialogClicked();
    void clearSpriteClicked();
    void valueChanged();
};

#endif // __SPRITE_PROPERTY_DELEGATE_H__
