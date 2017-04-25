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
    result->setProperty("modelIndex", index);
    return result;
}

void PropertiesViewDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    editor->setProperty("modelIndex", QVariant());
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
    if (index.isValid() == false || index.column() == 0)
    {
        return false;
    }

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    {
        QMouseEvent* ev = static_cast<QMouseEvent*>(event);

        BaseComponentValue* value = GetComponentValue(index);
        QWidget* target = view->viewport();
        QPoint localPos = ev->pos();
        QWidget* childAt = value->AcquireEditorWidget(option);
        while (childAt != nullptr && childAt != target)
        {
            localPos = childAt->mapFrom(target, localPos);
            target = childAt;
            childAt = target->childAt(localPos);
        }

        QMouseEvent* newEvent = new QMouseEvent(ev->type(), localPos, ev->windowPos(), ev->screenPos(),
                                                ev->button(), ev->buttons(), ev->modifiers(), ev->source());
        newEvent->setTimestamp(ev->timestamp());
        PlatformApi::Qt::GetApplication()->postEvent(target, newEvent);
    }
    default:
        break;
    }
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
    value->EnsureEditorCreated(view->viewport());
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

void PropertiesViewDelegate::UpdateSpanning(const QModelIndex& index, bool isSpanned) const
{
    if (view->isFirstColumnSpanned(index.row(), index.parent()) != isSpanned)
    {
        view->setFirstColumnSpanned(index.row(), index.parent(), isSpanned);
    }
}

bool PropertiesViewDelegate::eventFilter(QObject* object, QEvent* event)
{
    QWidget* w = qobject_cast<QWidget*>(object);
    if (w == nullptr)
    {
        return false;
    }

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->matches(QKeySequence::Cancel))
        {
            QVariant value = w->property("modelIndex");
            if (value.isValid() && value.canConvert<QModelIndex>())
            {
                QModelIndex index = value.value<QModelIndex>();
                GetComponentValue(index)->ForceUpdate();
            }
            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

} // namespace TArc
} // namespace DAVA
