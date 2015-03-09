#include "BasePropertyDelegate.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>

#include "PropertiesTreeModel.h"
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

    if (BasePropertyDelegate::IsValueReseted(editor))
    {
        if (model->setData(index, QVariant(), DAVA::ResetRole))
        {
            BasePropertyDelegate::SetValueModified(editor, false);
            BasePropertyDelegate::SetValueReseted(editor, false);
            return true;
        }
    }

    return false;

}

void BasePropertyDelegate::enumEditorActions( QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    if (property && property->GetEditFlag() & BaseProperty::EF_CAN_RESET )
    {
        QAction *resetAction = new QAction(QIcon(":/Icons/editclear.png"), tr("reset"), parent);
        resetAction->setToolTip(tr("reset property value to default"));
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
