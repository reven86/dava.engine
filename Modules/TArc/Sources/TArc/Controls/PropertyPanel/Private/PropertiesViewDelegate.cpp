#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "Engine/PlatformApi.h"

#include <QApplication>

namespace DAVA
{
namespace TArc
{
namespace PropertiesViewDelegateDetail
{
QStyle* GetStyle()
{
    return PlatformApi::Qt::GetApplication()->style();
}

void FixFont(QFont& font)
{
    font.setFamily(font.family());
}

void InitStyleOptions(QStyleOptionViewItem& options)
{
    FixFont(options.font);
    options.fontMetrics = QFontMetrics(options.font);
}

BaseComponentValue* GetComponentValue(const QModelIndex& index)
{
    DVASSERT(index.isValid());
    const ReflectedPropertyModel* model = qobject_cast<const ReflectedPropertyModel*>(index.model());
    DVASSERT(model != nullptr);

    return model->GetComponentValue(index);
}
}

PropertiesViewDelegate::PropertiesViewDelegate(QObject* parent)
    : QAbstractItemDelegate(parent)
{
}

void PropertiesViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    PropertiesViewDelegateDetail::InitStyleOptions(opt);

    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    DVASSERT(valueComponent != nullptr);
    if (index.column() == 0)
    {
        opt.text = valueComponent->GetPropertyName();
        PropertiesViewDelegateDetail::GetStyle()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
    else
    {
        valueComponent->StaticEditorPaint(PropertiesViewDelegateDetail::GetStyle(), painter, opt);
    }
}

QSize PropertiesViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    QStyle* style = PropertiesViewDelegateDetail::GetStyle();
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    DVASSERT(valueComponent != nullptr);
    if (index.column() == 0)
    {
        opt.text = valueComponent->GetPropertyName();
    }
    else
    {
        opt.text = QString::number(valueComponent->GetPropertiesNodeCount());
    }

    return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget);
}

QWidget* PropertiesViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    return valueComponent->AcquireEditorWidget(parent, option, index);
}

void PropertiesViewDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    valueComponent->ReleaseEditorWidget(editor, index);
}

void PropertiesViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
}

void PropertiesViewDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
}

void PropertiesViewDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!editor)
        return;

    QStyleOptionViewItem opt = option;
    opt.showDecorationSelected = true;

    QStyle* style = PropertiesViewDelegateDetail::GetStyle();
    QRect geom = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, option.widget);
    editor->setGeometry(geom);
}

bool PropertiesViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return false;
}

bool PropertiesViewDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return false;
}

bool PropertiesViewDelegate::eventFilter(QObject* obj, QEvent* e)
{
    return false;
}

} // namespace TArc
} // namespace DAVA