#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "TArc/WindowSubSystem/QtTArcEvents.h"

#include <Engine/PlatformApi.h>

#include <QTreeView>
#include <QApplication>
#include <QtEvents>
#include <QtGlobal>

namespace DAVA
{
namespace TArc
{
namespace PropertiesViewDelegateDetail
{
void FixFont(QFont& font)
{
    font.setFamily(font.family());
}

void InitStyleOptions(QStyleOptionViewItem& options, BaseComponentValue* componentValue)
{
    FixFont(options.font);
    options.fontMetrics = QFontMetrics(options.font);
    const BaseComponentValue::Style& style = componentValue->GetStyle();
    if (style.fontBold.IsEmpty() == false)
    {
        options.font.setBold(style.fontBold.Cast<bool>());
    }

    if (style.fontItalic.IsEmpty() == false)
    {
        options.font.setItalic(style.fontItalic.Cast<bool>());
    }

    if (style.bgColor.IsEmpty() == false)
    {
        QPalette::ColorRole role = style.bgColor.Cast<QPalette::ColorRole>(QPalette::Base);
        options.backgroundBrush = options.palette.brush(role);
    }

    if (style.fontColor.IsEmpty() == false)
    {
        QPalette::ColorRole role = style.fontColor.Cast<QPalette::ColorRole>(QPalette::Text);
        options.palette.setBrush(QPalette::Text, options.palette.brush(role));
    }
}
}

PropertiesViewDelegate::PropertiesViewDelegate(QTreeView* view_, ReflectedPropertyModel* model_, QObject* parent)
    : QStyledItemDelegate(parent)
    , model(model_)
    , view(view_)
{
    implFilter.delegate = this;
}

void PropertiesViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    BaseComponentValue* valueComponent = GetComponentValue(index);
    PropertiesViewDelegateDetail::InitStyleOptions(opt, valueComponent);

    DVASSERT(valueComponent != nullptr);
    bool isSpanned = valueComponent->IsSpannedControl();
    UpdateSpanning(index, isSpanned);
    if (index.column() == 0 && isSpanned == false)
    {
        QStyle* style = option.widget->style();
        opt.text = valueComponent->GetPropertyName();
        opt.features |= QStyleOptionViewItem::HasDisplay;
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
    else
    {
        painter->fillRect(opt.rect, opt.palette.window());

        AdjustEditorRect(opt);
        valueComponent->Draw(painter, opt);
    }
}

QSize PropertiesViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return QSize();
    }

    QStyleOptionViewItem opt = option;
    QStyle* style = opt.widget->style();
    BaseComponentValue* valueComponent = GetComponentValue(index);
    DVASSERT(valueComponent != nullptr);

    QSize sizeHint(opt.rect.size());
    if (index.column() == 0 && valueComponent->IsSpannedControl() == false)
    {
        opt.text = valueComponent->GetPropertyName();
        sizeHint = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget);
    }
    else
    {
        if (valueComponent->HasHeightForWidth())
        {
            int height = valueComponent->GetHeightForWidth(view->columnWidth(1));
            heightForWidthItems[QPersistentModelIndex(index)] = height;
            sizeHint.setHeight(height);
        }
        else
        {
            sizeHint.setHeight(valueComponent->GetHeight());
        }
    }

    return sizeHint;
}

QWidget* PropertiesViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return nullptr;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    QStyleOptionViewItem opt = option;
    AdjustEditorRect(opt);
    UpdateSpanning(index, valueComponent->IsSpannedControl());
    QWidget* result = valueComponent->AcquireEditorWidget(opt);
    activeEditors.insert(result);
    return result;
}

void PropertiesViewDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (editor != nullptr)
    {
        activeEditors.remove(editor);
    }
}

void PropertiesViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    BaseComponentValue* valueComponent = GetComponentValue(index);
    valueComponent->ForceUpdate();
}

void PropertiesViewDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
}

void PropertiesViewDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (editor == nullptr)
    {
        return;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    QStyleOptionViewItem opt = option;
    AdjustEditorRect(opt);
    UpdateSpanning(index, valueComponent->IsSpannedControl());
    valueComponent->UpdateGeometry(opt);
}

bool PropertiesViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return false;
}

bool PropertiesViewDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return false;
}

BaseComponentValue* PropertiesViewDelegate::GetComponentValue(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    BaseComponentValue* value = model->GetComponentValue(index);
    QWidget* w = value->EnsureEditorCreated(const_cast<EventFilterImpl*>(&implFilter), view->viewport());
    indexMap[w] = index;
    return value;
}

void PropertiesViewDelegate::AdjustEditorRect(QStyleOptionViewItem& opt) const
{
    opt.rect.setTop(opt.rect.top() + 1);
    opt.rect.setHeight(opt.rect.height() - 1);
}

bool PropertiesViewDelegate::UpdateSizeHints(int section, int newWidth)
{
    {
        auto iter = heightForWidthItems.begin();
        while (iter != heightForWidthItems.end())
        {
            if (!iter.key().isValid())
            {
                iter = heightForWidthItems.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    int sectionWidth = view->columnWidth(1);
    QList<QModelIndex> sizeHintChangedIndexes;
    for (auto iter = heightForWidthItems.begin(); iter != heightForWidthItems.end(); ++iter)
    {
        QPersistentModelIndex index = iter.key();
        BaseComponentValue* value = GetComponentValue(index);
        DVASSERT(value->HasHeightForWidth());
        int heightForWidth = value->GetHeightForWidth(sectionWidth);
        if (iter.value() != heightForWidth)
        {
            iter.value() = heightForWidth;
            sizeHintChangedIndexes.push_back(index);
        }
    }

    foreach (const QModelIndex& index, sizeHintChangedIndexes)
    {
        emit sizeHintChanged(index);
    }

    return sizeHintChangedIndexes.isEmpty() == false;
}

bool PropertiesViewDelegate::eventFilter(QObject* object, QEvent* event)
{
    return false;
}

bool PropertiesViewDelegate::eventEditorFilter(QObject* obj, QEvent* e)
{
    QWidget* editor = qobject_cast<QWidget*>(obj);
    if (!editor)
        return false;
    auto iter = indexMap.find(editor);
    DVASSERT(iter != indexMap.end());
    QModelIndex index = iter.value();

    if (e->type() == QT_EVENT_TYPE(EventsTable::FocusInToParent))
    {
        QItemSelectionModel* selectionModel = view->selectionModel();
        QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Clear | QItemSelectionModel::Select | QItemSelectionModel::Rows;
        if (selectionModel->currentIndex() == index)
        {
            if (activeEditors.contains(editor) == false)
            {
                view->edit(index);
            }
        }
        else
        {
            selectionModel->setCurrentIndex(index, flags);
        }
    }

    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        if (keyEvent->matches(QKeySequence::Cancel))
        {
            if (index.isValid())
            {
                setEditorData(editor, index);
            }
        }
    }

    return QStyledItemDelegate::eventFilter(obj, e);
}

void PropertiesViewDelegate::UpdateSpanning(const QModelIndex& index, bool isSpanned) const
{
    if (view->isFirstColumnSpanned(index.row(), index.parent()) != isSpanned)
    {
        view->setFirstColumnSpanned(index.row(), index.parent(), isSpanned);
    }
}

bool PropertiesViewDelegate::EventFilterImpl::eventFilter(QObject* obj, QEvent* e)
{
    DVASSERT(delegate != nullptr);
    return delegate->eventEditorFilter(obj, e);
}

} // namespace TArc
} // namespace DAVA
