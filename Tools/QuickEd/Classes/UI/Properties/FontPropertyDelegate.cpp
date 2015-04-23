#include "FontPropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include <QFileDialog>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "Helpers/ResourcesManageHelper.h"

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
    QString stringValue = StringToQString(variant.AsString());
    editor->setText(QString::fromStdString(variant.AsString()));
}

bool FontPropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DAVA::FilePath filePath = QStringToString(editor->text());
    variantType.SetFilePath(filePath);

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void FontPropertyDelegate::enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    QAction *openFileDialogAction = new QAction(tr("..."), parent);
    openFileDialogAction->setToolTip(tr("Select sprite descriptor"));
    actions.push_back(openFileDialogAction);
    connect(openFileDialogAction, SIGNAL(triggered(bool)), this, SLOT(openFileDialogClicked()));

    QAction *clearSpriteAction = new QAction(QIcon(":/Icons/editclear.png"), tr("clear"), parent);
    clearSpriteAction->setToolTip(tr("Clear sprite descriptor"));
    actions.push_back(clearSpriteAction);
    connect(clearSpriteAction, SIGNAL(triggered(bool)), this, SLOT(clearSpriteClicked()));

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void FontPropertyDelegate::openFileDialogClicked()
{
    QAction *openFileDialogAction = qobject_cast<QAction *>(sender());
    if (!openFileDialogAction)
        return;

    QWidget *editor = openFileDialogAction->parentWidget();
    if (!editor)
        return;

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");

    QString dir;
    QString pathText = lineEdit->text();
    if (!pathText.isEmpty())
    {
        DAVA::FilePath filePath = QStringToString(pathText);
        dir = StringToQString(filePath.GetDirectory().GetAbsolutePathname());
    }
    else
    {
        dir = ResourcesManageHelper::GetSpritesDirectory();
    }

    QString filePathText = QFileDialog::getOpenFileName(editor->parentWidget(), tr("Select sprite descriptor"), dir, QString("*.txt"));
    if (!filePathText.isEmpty())
    {
        lineEdit->setText(filePathText);

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FontPropertyDelegate::clearSpriteClicked()
{
    QAction *clearSpriteAction = qobject_cast<QAction *>(sender());
    if (!clearSpriteAction)
        return;

    QWidget *editor = clearSpriteAction->parentWidget();
    if (!editor)
        return;

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");

    lineEdit->setText(QString(""));

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
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
