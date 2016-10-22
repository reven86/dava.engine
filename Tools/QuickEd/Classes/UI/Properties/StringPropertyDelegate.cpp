#include "StringPropertyDelegate.h"
#include <QLineEdit>
#include <QLayout>
#include "DAVAEngine.h"
#include "PropertiesModel.h"
#include "Utils/QtDavaConvertion.h"
#include "QtTools/Utils/Utils.h"
#include "PropertiesTreeItemDelegate.h"

StringPropertyDelegate::StringPropertyDelegate(PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
{
}

QWidget* StringPropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));

    return lineEdit;
}

void StringPropertyDelegate::setEditorData(QWidget* rawEditor, const QModelIndex& index) const
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
    UnescapeString(stringValue);

    editor->blockSignals(true);
    editor->setText(stringValue);
    editor->blockSignals(false);
}

bool StringPropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();

    QString stringValue = EscapeString(editor->text());

    if (variantType.GetType() == DAVA::VariantType::TYPE_STRING)
    {
        variantType.SetString(QStringToString(stringValue));
    }
    else
    {
        variantType.SetWideString(QStringToWideString(stringValue));
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void StringPropertyDelegate::OnEditingFinished()
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