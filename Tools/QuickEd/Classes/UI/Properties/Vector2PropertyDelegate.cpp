#include "Vector2PropertyDelegate.h"
#include <QItemEditorFactory>
#include <QLineEdit>
#include <QApplication>
#include "QtControls/Vector2DEdit.h"
#include "Model/ControlProperties/AbstractProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"

Vector2PropertyDelegate::Vector2PropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

Vector2PropertyDelegate::~Vector2PropertyDelegate()
{
}

QWidget * Vector2PropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = new Vector2DEdit(parent);
    vectorEditor->setObjectName(QString::fromUtf8("vectorEditor"));
    connect(vectorEditor->lineEditX(), SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    connect(vectorEditor->lineEditY(), SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    return vectorEditor;
}

void Vector2PropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = editor->findChild<Vector2DEdit *>("vectorEditor");
    vectorEditor->setVector2D(Vector2ToQVector2D(index.data(Qt::EditRole).value<DAVA::VariantType>().AsVector2()));
}

bool Vector2PropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(editor, model, index))
        return true;

    Vector2DEdit *vectorEditor = editor->findChild<Vector2DEdit *>("vectorEditor");

    DAVA::VariantType vectorType( QVector2DToVector2(vectorEditor->vector2D()) );
    QVariant vectorVariant;
    vectorVariant.setValue<DAVA::VariantType>(vectorType);
    return model->setData(index, vectorVariant, Qt::EditRole);
}

void Vector2PropertyDelegate::OnEditingFinished()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget()->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}