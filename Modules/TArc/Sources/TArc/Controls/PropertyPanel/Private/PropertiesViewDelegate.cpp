#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "Engine/PlatformApi.h"

#include <QApplication>
#include <QtEvents>
#include <QtGlobal>

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

    QStyle* style = PropertiesViewDelegateDetail::GetStyle();
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    DVASSERT(valueComponent != nullptr);
    if (index.column() == 0)
    {
        opt.text = valueComponent->GetPropertyName();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
    else
    {
        valueComponent->GetStaticEditor().Draw(style, painter, opt);
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
        return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget);
    }

    return QSize(0, valueComponent->GetStaticEditor().GetHeight(style, option));
}

QWidget* PropertiesViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    return valueComponent->GetInteractiveEditor().AcquireEditorWidget(parent, option);
}

void PropertiesViewDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    return valueComponent->GetInteractiveEditor().ReleaseEditorWidget(editor);
}

void PropertiesViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    ReflectedPropertyModel* model = const_cast<ReflectedPropertyModel*>(qobject_cast<const ReflectedPropertyModel*>(index.model()));
    model->SyncWrapper();
}

void PropertiesViewDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);
    valueComponent->GetInteractiveEditor().CommitData();
}

void PropertiesViewDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!editor)
        return;

    BaseComponentValue* valueComponent = PropertiesViewDelegateDetail::GetComponentValue(index);

    editor->setGeometry(valueComponent->GetInteractiveEditor().GetEditorRect(PropertiesViewDelegateDetail::GetStyle(), option));
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
    QWidget* w = qobject_cast<QWidget*>(obj);
    if (w == nullptr)
    {
        return false;
    }

    if (e->type() == QEvent::FocusOut)
    {
        emit commitData(w);
        emit closeEditor(w);
    }

    if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        switch (keyEvent->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            emit commitData(w);
            emit closeEditor(w, QAbstractItemDelegate::EditNextItem);
            break;
        }
        case Qt::Key_Escape:
        {
            emit closeEditor(w, QAbstractItemDelegate::RevertModelCache);
            break;
        }
        default:
            break;
        }
    }

    return false;
}

} // namespace TArc
} // namespace DAVA