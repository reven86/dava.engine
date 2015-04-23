#include "BasePropertyDelegate.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>

#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

BasePropertyDelegate::BasePropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : AbstractPropertyDelegate(delegate)
{

}

BasePropertyDelegate::~BasePropertyDelegate()
{
    itemDelegate = NULL;
}

bool BasePropertyDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
    if (!BasePropertyDelegate::IsValueModified(editor))
        return true;

    BasePropertyDelegate::SetValueModified(editor, false);

    if (BasePropertyDelegate::IsValueReseted(editor))
    {
        BasePropertyDelegate::SetValueReseted(editor, false);
        if (model->setData(index, QVariant(), DAVA::ResetRole))
        {
            return true;
        }
    }

    return false;

}

void BasePropertyDelegate::enumEditorActions( QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    AbstractProperty *property = static_cast<AbstractProperty *>(index.internalPointer());
    if (property && property->GetEditFlag() & AbstractProperty::EF_CAN_RESET )
    {
        QAction *resetAction = new QAction(QIcon(":/Icons/edit_undo.png"), tr("reset"), parent);
        resetAction->setToolTip(tr("Reset property value to default"));
        actions.push_back(resetAction);
        connect(resetAction, SIGNAL(triggered(bool)), this, SLOT(resetClicked()));
    }
}

void BasePropertyDelegate::resetClicked()
{
    QAction *resetAction = qobject_cast<QAction *>(sender());
    if (!resetAction)
        return;

    QWidget *editor = resetAction->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueReseted(editor, true);
    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
