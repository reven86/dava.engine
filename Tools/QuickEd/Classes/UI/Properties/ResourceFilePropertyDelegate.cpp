#include "ResourceFilePropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include <QApplication>

#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "QtTools/FileDialogs/FileDialog.h"
#include "ResourcesManageHelper.h"

using namespace DAVA;

ResourceFilePropertyDelegate::ResourceFilePropertyDelegate(const QString& resourceExtension_, const QString& resourceDir_, PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
    , resourceExtension(resourceExtension_)
    , resourceDir(resourceDir_)
{
}

ResourceFilePropertyDelegate::~ResourceFilePropertyDelegate()
{
}

QWidget* ResourceFilePropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&)
{
    lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, &QLineEdit::editingFinished, this, &ResourceFilePropertyDelegate::OnEditingFinished);
    connect(lineEdit, &QLineEdit::textChanged, this, &ResourceFilePropertyDelegate::OnTextChanged);
    return lineEdit;
}

void ResourceFilePropertyDelegate::setEditorData(QWidget*, const QModelIndex& index) const
{
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    QString stringValue = StringToQString(variant.AsFilePath().GetStringValue());
    DVASSERT(!lineEdit.isNull());
    lineEdit->setText(stringValue);
}

bool ResourceFilePropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DVASSERT(!lineEdit.isNull());
    if (!lineEdit->text().isEmpty())
    {
        DAVA::FilePath absoluteFilePath = QStringToString(lineEdit->text());
        DAVA::FilePath frameworkFilePath = absoluteFilePath.GetFrameworkPath();
        variantType.SetFilePath(frameworkFilePath);
    }
    else
    {
        variantType.SetFilePath(DAVA::FilePath());
    }
    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void ResourceFilePropertyDelegate::enumEditorActions(QWidget* parent, const QModelIndex& index, QList<QAction*>& actions)
{
    QAction* selectFileAction = new QAction(tr("..."), parent);
    selectFileAction->setToolTip(tr("Select resource file"));
    actions.push_back(selectFileAction);
    connect(selectFileAction, SIGNAL(triggered(bool)), this, SLOT(selectFileClicked()));

    QAction* clearFileAction = new QAction(QIcon(":/Icons/editclear.png"), tr("clear"), parent);
    clearFileAction->setToolTip(tr("Clear resource file"));
    actions.push_back(clearFileAction);
    connect(clearFileAction, SIGNAL(triggered(bool)), this, SLOT(clearFileClicked()));

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void ResourceFilePropertyDelegate::selectFileClicked()
{
    DVASSERT(!lineEdit.isNull());
    QWidget* editor = lineEdit->parentWidget();
    DVASSERT(editor != nullptr);

    QString dir;
    QString pathText = lineEdit->text();
    if (!pathText.isEmpty())
    {
        DAVA::FilePath filePath = QStringToString(pathText);
        dir = StringToQString(filePath.GetDirectory().GetAbsolutePathname());
    }
    else
    {
        dir = ResourcesManageHelper::GetResourceRootDirectory() + resourceDir;
    }

    QString filePathText = FileDialog::getOpenFileName(editor->parentWidget(), tr("Select resource file"), dir, "*" + resourceExtension);
    if (!filePathText.isEmpty())
    {
        DAVA::FilePath absoluteFilePath = QStringToString(filePathText);
        DAVA::FilePath frameworkFilePath = absoluteFilePath.GetFrameworkPath();
        lineEdit->setText(StringToQString(frameworkFilePath.GetStringValue()));

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void ResourceFilePropertyDelegate::clearFileClicked()
{
    DVASSERT(!lineEdit.isNull());
    QWidget* editor = lineEdit->parentWidget();
    DVASSERT(editor != nullptr);
    lineEdit->setText(QString(""));

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

void ResourceFilePropertyDelegate::OnEditingFinished()
{
    DVASSERT(!lineEdit.isNull());
    if (!lineEdit->isModified())
    {
        return;
    }
    QWidget* editor = lineEdit->parentWidget();
    DVASSERT(editor != nullptr);
    const QString& text = lineEdit->text();
    if (!text.isEmpty() && !IsPathValid(text))
    {
        return;
    }
    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
    lineEdit->setModified(false);
}

void ResourceFilePropertyDelegate::OnTextChanged(const QString& text)
{
    QPalette palette(lineEdit->palette());
    QString textCopy(text);

    QColor globalTextColor = qApp->palette().color(QPalette::Text);
    QColor nextColor = IsPathValid(text) ? globalTextColor : Qt::red;
    palette.setColor(QPalette::Text, nextColor);
    lineEdit->setPalette(palette);
}

bool ResourceFilePropertyDelegate::IsPathValid(const QString& path)
{
    QString fullPath = path;
    if (!fullPath.isEmpty() && !fullPath.endsWith(resourceExtension))
    {
        fullPath.append(resourceExtension);
    }
    DAVA::FilePath filePath(QStringToString(fullPath));
    return FileSystem::Instance()->Exists(filePath);
}
