#include "ColorPropertyDelegate.h"
#include <QToolButton>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QColorDialog>

#include "PropertiesTreeItemDelegate.h"
#include "PropertiesModel.h"
#include "Utils/QtDavaConvertion.h"


ColorPropertyDelegate::ColorPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

ColorPropertyDelegate::~ColorPropertyDelegate()
{

}

QWidget *ColorPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    //QRegExpValidator *validator = new QRegExpValidator();
    //validator->setRegExp(QRegExp("#{0,1}[A-F0-9]{8}", Qt::CaseInsensitive));
    //lineEdit->setValidator(validator);

    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    return lineEdit;
}

void ColorPropertyDelegate::enumEditorActions( QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    BasePropertyDelegate::enumEditorActions(parent, index, actions);

    QAction *chooseColor = new QAction(tr("..."), parent);
    connect(chooseColor, SIGNAL(triggered(bool)), this, SLOT(OnChooseColorClicked()));
    actions.push_front(chooseColor);
}

void ColorPropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");
    QColor color = ColorToQColor(index.data(Qt::EditRole).value<DAVA::VariantType>().AsColor());
    lineEdit->setText(QColorToHex(color));
    lineEdit->setProperty("color", color);
}

bool ColorPropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(editor, model, index))
        return true;

    QLineEdit *lineEdit = editor->findChild<QLineEdit *>("lineEdit");

    QColor newColor = HexToQColor(lineEdit->text());
    //DAVA::VariantType color( QColorToColor(lineEdit->property("color").value<QColor>()) );
    DAVA::VariantType color( QColorToColor(newColor) );
    QVariant colorVariant;
    colorVariant.setValue<DAVA::VariantType>(color);
    return model->setData(index, colorVariant, Qt::EditRole);
}

void ColorPropertyDelegate::OnChooseColorClicked()
{
    QAction *chooseAction = qobject_cast<QAction *>(sender());
    if (!chooseAction)
        return;

    QWidget *editor = chooseAction->parentWidget();
    if (!editor)
        return;

    QLineEdit *lineEdit = editor->findChild<QLineEdit *>("lineEdit");

    QColorDialog dlg(editor);

    dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    dlg.setCurrentColor(lineEdit->property("color").value<QColor>());

    if (dlg.exec() == QDialog::Accepted)
    {
        lineEdit->setText(QColorToHex(dlg.selectedColor()));
        lineEdit->setProperty("color", dlg.selectedColor());
        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void ColorPropertyDelegate::OnEditingFinished()
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
