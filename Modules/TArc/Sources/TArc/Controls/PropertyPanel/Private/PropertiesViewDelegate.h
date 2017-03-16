#pragma once

#include <QAbstractItemDelegate>
#include <QPersistentModelIndex>
#include <QHash>

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class BaseComponentValue;
class PropertiesViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    PropertiesViewDelegate(QAbstractItemView* view, ReflectedPropertyModel* model, QObject* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    void UpdateSizeHints(int section, int newWidth);

private:
    BaseComponentValue* GetComponentValue(const QModelIndex& index) const;
    void AdjustEditorRect(QStyleOptionViewItem& opt) const;

private:
    ReflectedPropertyModel* model = nullptr;
    QAbstractItemView* view = nullptr;
    mutable QHash<QPersistentModelIndex, int> heightForWidthItems;
};
}
}