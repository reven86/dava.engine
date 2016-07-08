#include "Vector2PropertyDelegate.h"

#include <QLineEdit>
#include <QLayout>
#include "PropertiesModel.h"
#include "Utils/QtDavaConvertion.h"
#include "Utils/Utils.h"
#include "PropertiesTreeItemDelegate.h"

using namespace DAVA;

Vector2PropertyDelegate::Vector2PropertyDelegate(PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
{
}

Vector2PropertyDelegate::~Vector2PropertyDelegate()
{
}

QWidget* Vector2PropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, &QLineEdit::editingFinished, this, &Vector2PropertyDelegate::OnEditingFinished);
    return lineEdit;
}

void Vector2PropertyDelegate::setEditorData(QWidget* rawEditor, const QModelIndex& index) const
{
    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    QString stringValue;
    if (variant.GetType() == VariantType::TYPE_VECTOR2)
    {
        const Vector2& v = variant.AsVector2();
        stringValue.QString::sprintf("%g; %g", v.x, v.y);
    }
    else
    {
        stringValue = "?";
        DVASSERT(false);
    }
    editor->blockSignals(true);
    editor->setText(stringValue);
    editor->blockSignals(false);
}

bool Vector2PropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType;

    QStringList tokens = editor->text().split(";");

    Vector2 val;
    int count = Min(tokens.size(), 2);
    for (int i = 0; i < count; i++)
    {
        val.data[i] = tokens[i].toFloat();
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(VariantType(val));

    return model->setData(index, variant, Qt::EditRole);
}

void Vector2PropertyDelegate::OnEditingFinished()
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
