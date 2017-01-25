#include "IntegerPropertyDelegate.h"
#include <QSpinBox>
#include <QLayout>
#include "FileSystem/VariantType.h"
#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

IntegerPropertyDelegate::IntegerPropertyDelegate(PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
{
}

IntegerPropertyDelegate::~IntegerPropertyDelegate()
{
}

QWidget* IntegerPropertyDelegate::createEditor(QWidget* parent, const PropertiesContext& context, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QSpinBox* spinBox = new QSpinBox(parent);
    spinBox->setObjectName(QString::fromUtf8("spinBox"));
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));

    return spinBox;
}

void IntegerPropertyDelegate::setEditorData(QWidget* rawEditor, const QModelIndex& index) const
{
    QSpinBox* editor = rawEditor->findChild<QSpinBox*>("spinBox");

    editor->blockSignals(true);
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    editor->setMinimum(-99999);
    editor->setMaximum(99999);
    switch (variant.GetType())
    {
    case DAVA::VariantType::TYPE_INT8:
        editor->setValue(variant.AsInt8());
        break;
    case DAVA::VariantType::TYPE_UINT8:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt8());
        break;
    case DAVA::VariantType::TYPE_INT16:
        editor->setValue(variant.AsInt16());
        break;
    case DAVA::VariantType::TYPE_UINT16:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt16());
        break;
    case DAVA::VariantType::TYPE_INT32:
        editor->setValue(variant.AsInt32());
        break;
    case DAVA::VariantType::TYPE_UINT32:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt32());
        break;
    case DAVA::VariantType::TYPE_INT64:
        editor->setValue(variant.AsInt64());
        break;
    case DAVA::VariantType::TYPE_UINT64:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt64());
        break;
    default:
        break;
    }
    editor->blockSignals(false);
    BasePropertyDelegate::SetValueModified(editor, false);
}

bool IntegerPropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QSpinBox* editor = rawEditor->findChild<QSpinBox*>("spinBox");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();

    switch (variantType.GetType())
    {
    case DAVA::VariantType::TYPE_INT8:
        variantType.SetInt8(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT8:
        variantType.SetUInt8(editor->value());
        break;
    case DAVA::VariantType::TYPE_INT16:
        variantType.SetInt16(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT16:
        variantType.SetUInt16(editor->value());
        break;
    case DAVA::VariantType::TYPE_INT32:
        variantType.SetInt32(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT32:
        variantType.SetUInt32(editor->value());
        break;
    case DAVA::VariantType::TYPE_INT64:
        variantType.SetInt64(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT64:
        variantType.SetUInt64(editor->value());
        break;
    default:
        break;
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void IntegerPropertyDelegate::OnValueChanged()
{
    QWidget* spinBox = qobject_cast<QWidget*>(sender());
    if (!spinBox)
        return;

    QWidget* editor = spinBox->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
