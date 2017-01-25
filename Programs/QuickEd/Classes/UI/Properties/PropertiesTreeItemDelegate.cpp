#include "PropertiesTreeItemDelegate.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QPainter>
#include <QAction>
#include <QStylePainter>
#include <QApplication>
#include <QToolButton>
#include <QEvent>
#include <QMouseEvent>
#include <QSortFilterProxyModel>
#include <QAbstractItemModel>

#include "DAVAEngine.h"
#include "Model/ControlProperties/AbstractProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "Vector2PropertyDelegate.h"
#include "EnumPropertyDelegate.h"
#include "PropertiesModel.h"
#include "StringPropertyDelegate.h"
#include "ComboPropertyDelegate.h"
#include "FilePathPropertyDelegate.h"
#include "ColorPropertyDelegate.h"
#include "IntegerPropertyDelegate.h"
#include "FloatPropertyDelegate.h"
#include "BoolPropertyDelegate.h"
#include "ResourceFilePropertyDelegate.h"
#include "Vector4PropertyDelegate.h"
#include "FontPropertyDelegate.h"
#include "TablePropertyDelegate.h"
#include "CompletionsProviderForScrollBar.h"
#include "Project/Project.h"

using namespace DAVA;

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    propertyItemDelegates[AbstractProperty::TYPE_ENUM] = new EnumPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR2] = new Vector2PropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_STRING] = new StringPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_COLOR] = new ColorPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_WIDE_STRING] = new StringPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FILEPATH] = new FilePathPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT8] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT8] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT16] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT16] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT32] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT32] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT64] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT64] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FLOAT] = new FloatPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_BOOLEAN] = new BoolPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR4] = new Vector4PropertyDelegate(this);

    const QString& gfxExtension = Project::GetGraphicsFileExtension();
    const QString& particleExtension = Project::Get3dFileExtension();

    propertyNameTypeItemDelegates[PropertyPath("*", "Actions")] = new TablePropertyDelegate(QList<QString>({ "Action", "Shortcut" }), this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Sprite")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Mask")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Detail")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Gradient")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Contour")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Effect path")] = new ResourceFilePropertyDelegate(particleExtension, "/3d/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "Font")] = new FontPropertyDelegate(this);
    propertyNameTypeItemDelegates[PropertyPath("ScrollBarDelegate", "Delegate")] = new ComboPropertyDelegate(this, std::make_unique<CompletionsProviderForScrollBar>());

    propertyNameTypeItemDelegates[PropertyPath("*", "bg-sprite")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-mask")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-detail")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-gradient")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-contour")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "text-font")] = new FontPropertyDelegate(this);
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
    for (auto iter = propertyItemDelegates.begin(); iter != propertyItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }

    for (auto iter = variantTypeItemDelegates.begin(); iter != variantTypeItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }

    for (auto iter = propertyNameTypeItemDelegates.begin(); iter != propertyNameTypeItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }
}

QWidget* PropertiesTreeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QModelIndex sourceIndex = index;
    const QAbstractItemModel* model = index.model();
    const QSortFilterProxyModel* sortModel = dynamic_cast<const QSortFilterProxyModel*>(model);
    if (sortModel != nullptr)
    {
        sourceIndex = sortModel->mapToSource(index);
    }
    AbstractPropertyDelegate* currentDelegate = GetCustomItemDelegateForIndex(sourceIndex);
    if (currentDelegate)
    {
        PropertyWidget* editorWidget = new PropertyWidget(parent);
        editorWidget->setObjectName(QString::fromUtf8("editorWidget"));
        QWidget* editor = currentDelegate->createEditor(editorWidget, context, option, sourceIndex);
        if (!editor)
        {
            DAVA::SafeDelete(editorWidget);
        }
        else
        {
            editorWidget->editWidget = editor;
            editorWidget->setFocusPolicy(Qt::WheelFocus);

            QHBoxLayout* horizontalLayout = new QHBoxLayout(editorWidget);
            horizontalLayout->setSpacing(1);
            horizontalLayout->setContentsMargins(0, 0, 0, 0);
            horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
            editorWidget->setLayout(horizontalLayout);

            editorWidget->setAutoFillBackground(true);
            editorWidget->setFocusProxy(editor);

            editorWidget->layout()->addWidget(editor);

            QList<QAction*> actions;
            currentDelegate->enumEditorActions(editorWidget, sourceIndex, actions);

            foreach (QAction* action, actions)
            {
                QToolButton* toolButton = new QToolButton(editorWidget);
                toolButton->setDefaultAction(action);
                toolButton->setIconSize(QSize(15, 15));
                toolButton->setFocusPolicy(Qt::StrongFocus);
                editorWidget->layout()->addWidget(toolButton);
            }
        }

        return editorWidget;
    }

    if (sourceIndex.data(Qt::EditRole).type() == QVariant::Bool)
        return NULL;

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertiesTreeItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QModelIndex sourceIndex = index;
    const QAbstractItemModel* model = index.model();
    const QSortFilterProxyModel* sortModel = dynamic_cast<const QSortFilterProxyModel*>(model);
    if (sortModel != nullptr)
    {
        sourceIndex = sortModel->mapToSource(index);
    }

    AbstractPropertyDelegate* currentDelegate = GetCustomItemDelegateForIndex(sourceIndex);
    if (currentDelegate)
    {
        return currentDelegate->setEditorData(editor, sourceIndex);
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertiesTreeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QModelIndex sourceIndex = index;
    const QSortFilterProxyModel* sortModel = dynamic_cast<const QSortFilterProxyModel*>(model);
    if (sortModel != nullptr)
    {
        sourceIndex = sortModel->mapToSource(index);
    }

    AbstractPropertyDelegate* currentDelegate = GetCustomItemDelegateForIndex(sourceIndex);
    if (currentDelegate)
    {
        currentDelegate->setModelData(editor, model, index);
        return;
    }

    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit && !lineEdit->isModified())
        return;

    QStyledItemDelegate::setModelData(editor, model, index);
}

AbstractPropertyDelegate* PropertiesTreeItemDelegate::GetCustomItemDelegateForIndex(const QModelIndex& index) const
{
    AbstractProperty* property = static_cast<AbstractProperty*>(index.internalPointer());
    if (property)
    {
        auto prop_iter = propertyItemDelegates.find(property->GetType());
        if (prop_iter != propertyItemDelegates.end())
            return prop_iter.value();

        QString parentName;
        AbstractProperty* parentProperty = property->GetParent();
        if (parentProperty)
        {
            parentName = QString::fromStdString(parentProperty->GetName());
        }

        QMap<PropertyPath, AbstractPropertyDelegate*>::const_iterator propNameIt;
        propNameIt = propertyNameTypeItemDelegates.find(PropertyPath(parentName, QString::fromStdString(property->GetName())));
        if (propNameIt == propertyNameTypeItemDelegates.end())
        {
            propNameIt = propertyNameTypeItemDelegates.find(PropertyPath("*", QString::fromStdString(property->GetName())));
        }

        if (propNameIt != propertyNameTypeItemDelegates.end())
        {
            return propNameIt.value();
        }

        auto varIt = variantTypeItemDelegates.find(property->GetValueType());
        if (varIt != variantTypeItemDelegates.end())
        {
            return varIt.value();
        }
    }

    return nullptr;
}

void PropertiesTreeItemDelegate::SetProject(const Project* project)
{
    context.project = project;
}

void PropertiesTreeItemDelegate::emitCommitData(QWidget* editor)
{
    emit commitData(editor);
}

void PropertiesTreeItemDelegate::emitCloseEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
    emit closeEditor(editor, hint);
}

void PropertiesTreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
    QStyleOptionViewItemV3 opt = option;

    QStyledItemDelegate::paint(painter, opt, index);

    opt.palette.setCurrentColorGroup(QPalette::Active);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));

    int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
    painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
    painter->restore();
}

PropertyWidget::PropertyWidget(QWidget* parent /*= NULL*/)
    : QWidget(parent)
    , editWidget(NULL)
{
}

bool PropertyWidget::event(QEvent* e)
{
    switch (e->type())
    {
    case QEvent::ShortcutOverride:
        if (static_cast<QObject*>(editWidget)->event(e))
            return true;
        break;

    case QEvent::InputMethod:
        return static_cast<QObject*>(editWidget)->event(e);

    default:
        break;
    }

    return QWidget::event(e);
}

void PropertyWidget::keyPressEvent(QKeyEvent* e)
{
    static_cast<QObject*>(editWidget)->event(e);
}

void PropertyWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
        return;

    e->ignore();
}

void PropertyWidget::mouseReleaseEvent(QMouseEvent* e)
{
    e->accept();
}

void PropertyWidget::focusInEvent(QFocusEvent* e)
{
    static_cast<QObject*>(editWidget)->event(e);
    QWidget::focusInEvent(e);
}

void PropertyWidget::focusOutEvent(QFocusEvent* e)
{
    static_cast<QObject*>(editWidget)->event(e);
    QWidget::focusOutEvent(e);
}
