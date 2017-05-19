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

#include "Model/ControlProperties/AbstractProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "Vector2PropertyDelegate.h"
#include "EnumPropertyDelegate.h"
#include "PropertiesModel.h"
#include "StringPropertyDelegate.h"
#include "ComboPropertyDelegate.h"
#include "FilePathPropertyDelegate.h"
#include "FMODEventPropertyDelegate.h"
#include "ColorPropertyDelegate.h"
#include "IntegerPropertyDelegate.h"
#include "FloatPropertyDelegate.h"
#include "BoolPropertyDelegate.h"
#include "ResourceFilePropertyDelegate.h"
#include "Vector4PropertyDelegate.h"
#include "FontPropertyDelegate.h"
#include "TablePropertyDelegate.h"
#include "CompletionsProviderForScrollBar.h"
#include "PredefinedCompletionsProvider.h"
#include "Modules/LegacySupportModule/Private/Project.h"

using namespace DAVA;

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    propertyItemDelegates[AbstractProperty::TYPE_ENUM] = new EnumPropertyDelegate(this);
    anyItemDelegates[Type::Instance<Vector2>()] = new Vector2PropertyDelegate(this);
    anyItemDelegates[Type::Instance<String>()] = new StringPropertyDelegate(this);
    anyItemDelegates[Type::Instance<Color>()] = new ColorPropertyDelegate(this);
    anyItemDelegates[Type::Instance<WideString>()] = new StringPropertyDelegate(this);
    anyItemDelegates[Type::Instance<FilePath>()] = new FilePathPropertyDelegate(this);
    anyItemDelegates[Type::Instance<int8>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<uint8>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<int16>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<uint16>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<int32>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<uint32>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<int64>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<uint64>()] = new IntegerPropertyDelegate(this);
    anyItemDelegates[Type::Instance<float32>()] = new FloatPropertyDelegate(this);
    anyItemDelegates[Type::Instance<bool>()] = new BoolPropertyDelegate(this);
    anyItemDelegates[Type::Instance<Vector4>()] = new Vector4PropertyDelegate(this);

    const QString& gfxExtension = Project::GetGraphicsFileExtension();
    const QString& particleExtension = Project::Get3dFileExtension();

    QStringList formulaCompletions;
    formulaCompletions << "childrenSum"
                       << "maxChild"
                       << "firstChild"
                       << "lastChild"
                       << "content"
                       << "parent"
                       << "parentRest"
                       << "parentLine"
                       << "min(parentRest, content)"
                       << "max(parent, childrenSum)";

    propertyNameTypeItemDelegates[PropertyPath("*", "actions")] = new TablePropertyDelegate(QList<QString>({ "Action", "Shortcut" }), this);
    propertyNameTypeItemDelegates[PropertyPath("*", "sprite")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "mask")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "detail")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "gradient")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "contour")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "effectPath")] = new ResourceFilePropertyDelegate(particleExtension, "/3d/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "font")] = new FontPropertyDelegate(this);
    propertyNameTypeItemDelegates[PropertyPath("ScrollBarDelegate", "delegate")] = new ComboPropertyDelegate(this, std::make_unique<CompletionsProviderForScrollBar>());

    propertyNameTypeItemDelegates[PropertyPath("*", "bg-sprite")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-mask")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-detail")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-gradient")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "bg-contour")] = new ResourceFilePropertyDelegate(gfxExtension, "/Gfx/", this);
    propertyNameTypeItemDelegates[PropertyPath("*", "text-font")] = new FontPropertyDelegate(this);

    propertyNameTypeItemDelegates[PropertyPath("RichContent", "aliases")] = new TablePropertyDelegate(QList<QString>({ "Alias", "Xml" }), this);
    propertyNameTypeItemDelegates[PropertyPath("Sound", "*")] = new FMODEventPropertyDelegate(this);
    propertyNameTypeItemDelegates[PropertyPath("*", "sound-touchDown")] = new FMODEventPropertyDelegate(this);
    propertyNameTypeItemDelegates[PropertyPath("*", "sound-touchUpInside")] = new FMODEventPropertyDelegate(this);
    propertyNameTypeItemDelegates[PropertyPath("*", "sound-touchUpOutside")] = new FMODEventPropertyDelegate(this);
    propertyNameTypeItemDelegates[PropertyPath("*", "sound-touchValueChanged")] = new FMODEventPropertyDelegate(this);

    propertyNameTypeItemDelegates[PropertyPath("SizePolicy", "horizontalFormula")] = new ComboPropertyDelegate(this, std::make_unique<PredefinedCompletionsProvider>(formulaCompletions));
    propertyNameTypeItemDelegates[PropertyPath("SizePolicy", "verticalFormula")] = new ComboPropertyDelegate(this, std::make_unique<PredefinedCompletionsProvider>(formulaCompletions));
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
    for (auto iter = propertyItemDelegates.begin(); iter != propertyItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }

    for (auto iter = anyItemDelegates.begin(); iter != anyItemDelegates.end(); ++iter)
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
    QModelIndex sourceIndex = GetSourceIndex(index, nullptr);
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
    QModelIndex sourceIndex = GetSourceIndex(index, nullptr);

    AbstractPropertyDelegate* currentDelegate = GetCustomItemDelegateForIndex(sourceIndex);
    if (currentDelegate)
    {
        return currentDelegate->setEditorData(editor, sourceIndex);
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertiesTreeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QModelIndex sourceIndex = GetSourceIndex(index, model);
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

        if (propNameIt == propertyNameTypeItemDelegates.end())
        {
            propNameIt = propertyNameTypeItemDelegates.find(PropertyPath(parentName, "*"));
        }

        if (propNameIt != propertyNameTypeItemDelegates.end())
        {
            return propNameIt.value();
        }

        auto varIt = anyItemDelegates.find(property->GetValueType());
        if (varIt != anyItemDelegates.end())
        {
            return varIt.value();
        }

        varIt = anyItemDelegates.find(property->GetValueType()->Decay());
        if (varIt != anyItemDelegates.end())
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

QModelIndex PropertiesTreeItemDelegate::GetSourceIndex(QModelIndex index, QAbstractItemModel* itemModel) const
{
    QModelIndex sourceIndex = index;
    const QAbstractItemModel* model = itemModel ? itemModel : index.model();
    const QSortFilterProxyModel* sortModel = dynamic_cast<const QSortFilterProxyModel*>(model);
    if (sortModel != nullptr)
    {
        sourceIndex = sortModel->mapToSource(index);
    }

    return sourceIndex;
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
