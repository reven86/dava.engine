#ifndef __QT_SCENE_TREE_DELEGATE_H__
#define __QT_SCENE_TREE_DELEGATE_H__

#include <QStyledItemDelegate>

class SceneTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SceneTreeDelegate(QWidget* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    void customDraw(QPainter* painter, QStyleOptionViewItem* option, const QModelIndex& index) const;
};

#endif // __QT_SCENE_TREE_DELEGATE_H__
