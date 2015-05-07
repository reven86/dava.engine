#include "FontPropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QComboBox>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "EditorCore.h"
#include "UI/Dialogs/DialogConfigurePreset.h"
#include "UI/Dialogs/DialogEditPresetName.h"

using namespace DAVA;


FontPropertyDelegate::FontPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{

}

FontPropertyDelegate::~FontPropertyDelegate()
{

}

QWidget * FontPropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setObjectName("comboBox");
    comboBox->addItem("");
    comboBox->addItems(GetEditorFontSystem()->GetDefaultPresetNames());
    connect(comboBox, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated), this, &FontPropertyDelegate::valueChanged);
    return comboBox;
}

void FontPropertyDelegate::setEditorData(QWidget *rawEditor, const QModelIndex &index) const
{
    QComboBox *editor = rawEditor->findChild<QComboBox*>("comboBox");

    VariantType variant = index.data(Qt::EditRole).value<VariantType>();
    editor->setCurrentText(QString::fromStdString(variant.AsString()));
}

bool FontPropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QComboBox *editor = rawEditor->findChild<QComboBox*>("comboBox");

    VariantType variantType = index.data(Qt::EditRole).value<VariantType>();
    String str = QStringToString(editor->currentText());
    variantType.SetString(str);

    QVariant variant;
    variant.setValue<VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void FontPropertyDelegate::enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    QAction *configurePresetAction = new QAction(QIcon(":/Icons/configure.png"), tr("configure"), parent);
    configurePresetAction->setToolTip(tr("configure preset"));
    actions.push_back(configurePresetAction);
    connect(configurePresetAction, &QAction::triggered, this, &FontPropertyDelegate::configurePresetClicked);

    QAction *addPresetAction = new QAction(QIcon(":/Icons/add.png"), tr("configure"), parent);
    addPresetAction->setToolTip(tr("add preset"));
    actions.push_back(addPresetAction);
    connect(addPresetAction, &QAction::triggered, this, &FontPropertyDelegate::addPresetClicked);

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void FontPropertyDelegate::addPresetClicked()
{
    QAction *editPresetAction = qobject_cast<QAction *>(sender());
    if (!editPresetAction)
        return;

    QWidget *editor = editPresetAction->parentWidget();
    if (!editor)
        return;
    QComboBox *comboBox = editor->findChild<QComboBox*>("comboBox");
    DialogEditPresetName dialogEditPresetName(comboBox->currentText(), qApp->activeWindow());
    if(dialogEditPresetName.exec())
    {
        comboBox->clear();
        comboBox->addItems(GetEditorFontSystem()->GetDefaultPresetNames());
        comboBox->setCurrentText(dialogEditPresetName.lineEdit_newFontPresetName->text());

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FontPropertyDelegate::configurePresetClicked()
{
    QAction *editPresetAction = qobject_cast<QAction *>(sender());
    if (!editPresetAction)
        return;

    QWidget *editor = editPresetAction->parentWidget();
    if (!editor)
        return;

    QComboBox *comboBox = editor->findChild<QComboBox*>("comboBox");
    DialogConfigurePreset dialogConfigurePreset(comboBox->currentText(), qApp->activeWindow());
    if (dialogConfigurePreset.exec())
    {
        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FontPropertyDelegate::valueChanged()
{
    QWidget *widget = qobject_cast<QWidget *>(sender());
    if (nullptr == widget)
    {
        return;
    }
    QWidget *editor = widget->parentWidget();
    if (nullptr == editor)
    {
        return;
    }

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}