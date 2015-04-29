#include "FontPropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include <QCompleter>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "Project/Project.h"
#include "Project/EditorFontSystem.h"
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
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setCompleter(new QCompleter(Project::Instance()->GetEditorFontSystem()->GetDefaultPresetNames(), lineEdit));
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

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");
    DialogEditPresetName dialogEditPresetName(lineEdit->text(), qApp->activeWindow());
    if(dialogEditPresetName.exec())
    {
        lineEdit->setCompleter(new QCompleter(Project::Instance()->GetEditorFontSystem()->GetDefaultPresetNames(), lineEdit));
        lineEdit->setText(dialogEditPresetName.lineEdit_newFontPresetName->text());

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

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");
    DialogConfigurePreset dialogConfigurePreset(lineEdit->text(), qApp->activeWindow());
    if (dialogConfigurePreset.exec())
    {
        lineEdit->setCompleter(new QCompleter(Project::Instance()->GetEditorFontSystem()->GetDefaultPresetNames(), lineEdit));
        lineEdit->setText(dialogConfigurePreset.lineEdit_currentFontPresetName->text());

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
