#include "BoolPropertyDelegate.h"
#include <QComboBox>
#include "DAVAEngine.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"


BoolPropertyDelegate::BoolPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{

}

BoolPropertyDelegate::~BoolPropertyDelegate()
{

}

QWidget * BoolPropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setObjectName(QString::fromUtf8("comboBox"));
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged()));

    comboBox->blockSignals(true);
    comboBox->addItem(QVariant(false).toString(), QVariant(false));
    comboBox->addItem(QVariant(true).toString(), QVariant(true));
    comboBox->blockSignals(false);

    return comboBox;
}

void BoolPropertyDelegate::setEditorData(QWidget * rawEditor, const QModelIndex & index) const
{
    QComboBox *editor = rawEditor->findChild<QComboBox *>("comboBox");

    editor->blockSignals(true);
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DVASSERT(variant.GetType() == DAVA::VariantType::TYPE_BOOLEAN);
    int comboIndex = editor->findData(QVariant(variant.AsBool()));
    editor->setCurrentIndex(comboIndex);
    editor->blockSignals(false);

    BasePropertyDelegate::SetValueModified(editor, false);
}

bool BoolPropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QComboBox *comboBox = rawEditor->findChild<QComboBox *>("comboBox");

    DAVA::VariantType variantType(comboBox->itemData(comboBox->currentIndex()).toBool());
    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void BoolPropertyDelegate::OnCurrentIndexChanged()
{
    QWidget *comboBox = qobject_cast<QWidget *>(sender());
    if (!comboBox)
        return;

    QWidget *editor = comboBox->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
