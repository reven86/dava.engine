#include "TablePropertyDelegate.h"

#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "EditorCore.h"
#include "UI/Dialogs/TableEditorDialog.h"

using namespace DAVA;

TablePropertyDelegate::TablePropertyDelegate(const QList<QString>& header_, PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
    , header(header_)
{
}

TablePropertyDelegate::~TablePropertyDelegate()
{
}

QWidget* TablePropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName("lineEdit");
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    return lineEdit;
}

void TablePropertyDelegate::setEditorData(QWidget* rawEditor, const QModelIndex& index) const
{
    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    QString stringValue;
    if (variant.GetType() == DAVA::VariantType::TYPE_STRING)
    {
        stringValue = StringToQString(variant.AsString());
    }
    else
    {
        stringValue = WideStringToQString(variant.AsWideString());
    }
    editor->blockSignals(true);
    editor->setText(stringValue);
    editor->blockSignals(false);
}

bool TablePropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();

    if (variantType.GetType() == DAVA::VariantType::TYPE_STRING)
    {
        variantType.SetString(QStringToString(editor->text()));
    }
    else
    {
        variantType.SetWideString(QStringToWideString(editor->text()));
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void TablePropertyDelegate::enumEditorActions(QWidget* parent, const QModelIndex& index, QList<QAction*>& actions)
{
    editTableAction = new QAction(QIcon(":/Icons/configure.png"), tr("edit"), parent);
    editTableAction->setToolTip(tr("edit table"));
    actions.push_back(editTableAction);
    connect(editTableAction, &QAction::triggered, this, &TablePropertyDelegate::editTableClicked);

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void TablePropertyDelegate::editTableClicked()
{
    QAction* editPresetAction = qobject_cast<QAction*>(sender());
    if (!editPresetAction)
        return;

    QWidget* rawEditor = editPresetAction->parentWidget();
    if (!rawEditor)
        return;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");
    TableEditorDialog dialog(editor->text(), header, qApp->activeWindow());
    if (dialog.exec())
    {
        editor->setText(dialog.GetValues());
        BasePropertyDelegate::SetValueModified(rawEditor, true);
        itemDelegate->emitCommitData(rawEditor);
    }
}

void TablePropertyDelegate::OnEditingFinished()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if (!lineEdit)
        return;

    QWidget* editor = lineEdit->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}
