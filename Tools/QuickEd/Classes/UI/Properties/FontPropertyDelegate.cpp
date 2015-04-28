#include "FontPropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include <QFileDialog>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "Helpers/ResourcesManageHelper.h"
#include "UI/Dialogs/editfontdialog.h"

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
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChanged()));

    return lineEdit;
}

void FontPropertyDelegate::setEditorData(QWidget * rawEditor, const QModelIndex & index) const
{
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    editor->setText(QString::fromStdString(variant.AsString()));
}

bool FontPropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DAVA::String str = QStringToString(editor->text());
    variantType.SetString(str);

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void FontPropertyDelegate::enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    QAction *editPresetAction = new QAction(tr("..."), parent);
    editPresetAction->setToolTip(tr("Edit preset"));
    actions.push_back(editPresetAction);
    connect(editPresetAction, SIGNAL(triggered(bool)), this, SLOT(editPresetClicked()));

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void FontPropertyDelegate::editPresetClicked()
{
    QAction *editPresetAction = qobject_cast<QAction *>(sender());
    if (!editPresetAction)
        return;

    QWidget *editor = editPresetAction->parentWidget();
    if (!editor)
        return;

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");

    EditFontDialog editFontDialog;
    editFontDialog.UpdateFontPreset(lineEdit->text());
    int dialogResult = editFontDialog.exec();
    QString presetResult = editFontDialog.lineEdit_fontPresetName->text();
    if (!presetResult.isEmpty())
    {
        lineEdit->setText(presetResult);

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FontPropertyDelegate::valueChanged()
{
    QWidget *lineEdit = qobject_cast<QWidget *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget();
    if (!editor)
        return;
    
    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
