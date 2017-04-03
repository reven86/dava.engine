#pragma once

#include <QStyledItemDelegate>
#include <QPersistentModelIndex>
#include <QHash>
#include <QSet>

class QTreeView;

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class BaseComponentValue;
class PropertiesViewDelegatePrivate;
class PropertiesViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PropertiesViewDelegate(QTreeView* view, ReflectedPropertyModel* model, QObject* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    bool UpdateSizeHints(int section, int newWidth);

protected:
    bool eventFilter(QObject* object, QEvent* event);
    bool eventEditorFilter(QObject* obj, QEvent* e);

private:
    BaseComponentValue* GetComponentValue(const QModelIndex& index) const;
    void AdjustEditorRect(QStyleOptionViewItem& opt) const;
    void UpdateSpanning(const QModelIndex& index, bool isSpanned) const;

private:
    ReflectedPropertyModel* model = nullptr;
    QTreeView* view = nullptr;
    mutable QHash<QPersistentModelIndex, int> heightForWidthItems;
    mutable QHash<QWidget*, QModelIndex> indexMap;
    mutable QSet<QWidget*> activeEditors;

    class EventFilterImpl : public QObject
    {
    public:
        PropertiesViewDelegate* delegate = nullptr;

        bool eventFilter(QObject* obj, QEvent* e);
    };

    EventFilterImpl implFilter;
};
}
}