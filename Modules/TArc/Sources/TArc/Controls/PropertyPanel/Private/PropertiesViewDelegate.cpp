#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "Engine/PlatformApi.h"

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
    : QAbstractItemDelegate(parent)
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
        QStyle* style = option.widget->style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        AdjustEditorRect(opt);
        valueComponent->Draw(view->viewport(), painter, opt);
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
        QWidget* viewport = view->viewport();
        if (valueComponent->HasHeightForWidth(viewport))
        {
            int height = valueComponent->GetHeightForWidth(viewport, view->columnWidth(1));
            heightForWidthItems[QPersistentModelIndex(index)] = height;
            sizeHint.setHeight(height);
        }
        else
        {
            sizeHint.setHeight(valueComponent->GetHeight(viewport));
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
    return valueComponent->AcquireEditorWidget(parent, opt);
}

void PropertiesViewDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    return valueComponent->ReleaseEditorWidget(editor);
}

void PropertiesViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    model->SyncWrapper();
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
    valueComponent->UpdateGeometry(view->viewport(), opt);
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
    return model->GetComponentValue(index);
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
    QWidget* viewportWidgte = view->viewport();
    for (auto iter = heightForWidthItems.begin(); iter != heightForWidthItems.end(); ++iter)
    {
        QPersistentModelIndex index = iter.key();
        BaseComponentValue* value = GetComponentValue(index);
        DVASSERT(value->HasHeightForWidth(viewportWidgte));
        int heightForWidth = value->GetHeightForWidth(viewportWidgte, sectionWidth);
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

} // namespace TArc
} // namespace DAVA
