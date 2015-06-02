#include "FilePathPropertyDelegate.h"
#include <QLineEdit>
#include "DAVAEngine.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesTreeItemDelegate.h"

FilePathPropertyDelegate::FilePathPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{

}

FilePathPropertyDelegate::~FilePathPropertyDelegate()
{

}

QWidget * FilePathPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));

    return lineEdit;
}

void FilePathPropertyDelegate::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    QString stringValue = StringToQString(variant.AsFilePath().GetStringValue());
    editor->setText(stringValue);
}

bool FilePathPropertyDelegate::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DAVA::String str = QStringToString(editor->text());
    DAVA::FilePath filePath = str;
    variantType.SetFilePath(filePath);

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void FilePathPropertyDelegate::OnEditingFinished()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}
