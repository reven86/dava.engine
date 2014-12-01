#include "UIPackageModel.h"

#include <qicon.h>
#include <QAction>

#include "DAVAEngine.h"
#include "Base/ObjectFactory.h"

#include "UI/IconHelper.h"
#include "UI/PackageDocument.h"
#include "Utils/QtDavaConvertion.h"
#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"
#include "UIControls/PackageHierarchy/ImportedPackagesNode.h"
#include "PackageModelCommands.h"

#include "UIPackageMimeData.h"

using namespace DAVA;

UIPackageModel::UIPackageModel(PackageDocument *document)
    : QAbstractItemModel(document)
    , root(NULL)
    , document(document)
{
    root = SafeRetain(document->Package());
}

UIPackageModel::~UIPackageModel()
{
    document = NULL;
    SafeRelease(root);
}

QModelIndex UIPackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, root->Get(row));

    PackageBaseNode *node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex UIPackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    PackageBaseNode *node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode *parent = node->GetParent();
    if (parent == NULL || parent == root)
        return QModelIndex();
    
    if (parent->GetParent())
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    else
        return createIndex(0, 0, parent);
}

int UIPackageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return root ? root->GetCount() : 0;
    
    return static_cast<PackageBaseNode*>(parent.internalPointer())->GetCount();
}

int UIPackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant UIPackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    
    int prototypeFlag = PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
    int controlFlag = PackageBaseNode::FLAG_CONTROL_CREATED_FROM_CLASS | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
    int flags = node->GetFlags();
    switch(role)
    {
        case Qt::DisplayRole:
            return StringToQString(node->GetName());
            
        case Qt::DecorationRole:
            return node->GetControl() != NULL ? QIcon(IconHelper::GetIconPathForUIControl(node->GetControl())) : QVariant();
            
        case Qt::CheckStateRole:
            if (node->GetControl())
                return node->GetControl()->GetVisibleForUIEditor() ? Qt::Checked : Qt::Unchecked;
            else
                return QVariant();
            
        case Qt::ToolTipRole:
            if (node->GetControl() != NULL)
            {
                ControlNode *controlNode = DynamicTypeCheck<ControlNode *>(node);
                QString toolTip = QString("class: ") + controlNode->GetControl()->GetControlClassName().c_str();
                if (!controlNode->GetControl()->GetCustomControlClassName().empty())
                {
                    toolTip += QString("\ncustom class: ") + controlNode->GetControl()->GetCustomControlClassName().c_str();
                }

                if (!controlNode->GetPrototypeName().empty())
                {
                    toolTip += QString("\nprototype: ") + controlNode->GetPrototypeName().c_str();
                }
                return toolTip;
            }
            break;
            
        case Qt::TextColorRole:
            return (flags & prototypeFlag) != 0 ? Qt::blue : Qt::black;
            
        case Qt::BackgroundRole:
            return (flags & controlFlag) == 0 ? Qt::lightGray : Qt::white;
            
        case Qt::FontRole:
        {
            QFont myFont;
            if ((flags & PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE) != 0 || (flags & controlFlag) == 0)
                myFont.setBold(true);
            if ((flags & PackageBaseNode::FLAG_READ_ONLY) != 0)
                myFont.setItalic(true);
            
            return myFont;
        }
            
        default:
            return QVariant();
    }

    return QVariant();
}

bool UIPackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    
    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    
    if (role == Qt::CheckStateRole)
    {
        if (node->GetControl())
            node->GetControl()->SetVisibleForUIEditor(value.toBool());
        return true;
    }
    return false;
}

Qt::ItemFlags UIPackageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    
    const PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    if ((node->GetFlags() & PackageBaseNode::FLAGS_CONTROL) != 0)
        flags |= Qt::ItemIsDragEnabled;
    if ((node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        flags |= Qt::ItemIsDropEnabled;
    
    return flags;
}

Qt::DropActions UIPackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList UIPackageModel::mimeTypes() const
{
    QStringList types;
    types << "application/packageModel";
    types << "text/plain";
    return types;
}

QMimeData *UIPackageModel::mimeData(const QModelIndexList &indexes) const
{
    UIPackageMimeData *mimeData = new UIPackageMimeData();
    
    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
            mimeData->SetIndex(index);
    }

    return mimeData;
}

bool UIPackageModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;
    
    int rowIndex;
    if (row != -1)
        rowIndex = row;
    else if (parent.isValid())
        rowIndex = rowCount(parent);
    else
        rowIndex = rowCount(QModelIndex());

    if (data->hasFormat("application/packageModel"))
    {
        const UIPackageMimeData *controlMimeData = dynamic_cast<const UIPackageMimeData*>(data);
        if (!controlMimeData)
            return false;
        
        if (column > 0)
            return false;

        QModelIndex srcIndex = controlMimeData->GetIndex();
        QModelIndex srcParent = srcIndex.parent();
        QModelIndex dstParent = parent;
        int dstRow = rowIndex;
        
        if (action == Qt::CopyAction)
        {
            QUndoCommand *changeCommand = new CopyItemModelCommand(this, srcIndex, dstRow, dstParent);
            document->UndoStack()->push(changeCommand);
            return true;
        }
        else if (action == Qt::MoveAction)
        {
            int srcRow = srcIndex.row();
            
            if (srcIndex == dstParent)
            {
                return false;
            }
            
            if (srcParent == dstParent && (dstRow == srcRow || dstRow == srcRow + 1))
            {
                return false;
            }

            ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(index(srcRow, 0, srcParent).internalPointer()));
            if (!sourceNode || (sourceNode->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) != 0)
            {
                return false;
            }

            document->UndoStack()->push(new MoveItemModelCommand(this, srcIndex, dstRow, dstParent));
            return true;
        }
        else if (action == Qt::LinkAction)
        {
            DVASSERT(false);
            return true;
        }
    }
    else if (data->hasFormat("text/plain") && data->hasText())
    {
        QModelIndex dstParent = parent;
        int dstRow = rowIndex;
        QUndoCommand *changeCommand = new InsertControlNodeCommand(this, data->text(), dstRow, dstParent);
        document->UndoStack()->push(changeCommand);
        return true;
    }

    return false;
}

void UIPackageModel::InsertItem(const QString &name, int dstRow, const QModelIndex &dstParent)
{
    String controlName = QStringToString(name);
    size_t slashIndex = controlName.find("/");
    ControlNode *node = NULL;
    if (slashIndex != String::npos)
    {
        String packName = controlName.substr(0, slashIndex);
        controlName = controlName.substr(slashIndex + 1, controlName.size() - slashIndex - 1);
        PackageControlsNode *packageControls = root->GetImportedPackagesNode()->FindPackageControlsNodeByName(packName);
        if (packageControls)
        {
            ControlNode *prototype = packageControls->FindControlNodeByName(controlName);
            if (prototype)
                node = new ControlNode(prototype, packageControls->GetPackage());
        }
    }
    else
    {
        UIControl *control = ObjectFactory::Instance()->New<UIControl>(controlName);
        if (control)
        {
            node = new ControlNode(control);
            SafeRelease(control);
        }
        else
        {
            ControlNode *prototype = root->GetPackageControlsNode()->FindControlNodeByName(controlName);
            if (prototype)
                node = new ControlNode(prototype, NULL);
        }
    }
    
    if (node)
    {
        beginInsertRows(dstParent, dstRow, dstRow);
        InsertNode(node, dstParent, dstRow);
        endInsertRows();
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::InsertItem(ControlNode *node, int dstRow, const QModelIndex &dstParent)
{
    beginInsertRows(dstParent, dstRow, dstRow);
    InsertNode(node, dstParent, dstRow);
    endInsertRows();
}

void UIPackageModel::MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent)
{
    ControlNode *sourceNode(dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcItem.internalPointer())));
    if (sourceNode)
    {
        beginMoveRows(srcItem.parent(), srcItem.row(), srcItem.row(), dstParent, dstRow);
        {
            SafeRetain(sourceNode);
            RemoveNode(sourceNode);
            InsertNode(sourceNode, dstParent, dstRow);
            SafeRelease(sourceNode);
        }
        endMoveRows();
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent)
{
    ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcItem.internalPointer()));
    if (sourceNode)
    {
        beginInsertRows(dstParent, dstRow, dstRow);
        {
            ControlNode* node = new ControlNode(sourceNode, NULL, ControlNode::CREATED_FROM_CLASS);
            InsertNode(node, dstParent, dstRow);
            SafeRelease(node);
        }
        endInsertRows();
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::RemoveItem(const QModelIndex &srcItem)
{
    ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcItem.internalPointer()));
    if (sourceNode)
    {
        beginRemoveRows(srcItem.parent(), srcItem.row(), srcItem.row());
        RemoveNode(sourceNode);
        endRemoveRows();
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::InsertNode(ControlNode *node, const QModelIndex &parent, int dstRow)
{
    if (parent.isValid())
    {
        PackageBaseNode *parentNode = static_cast<PackageBaseNode*>(parent.internalPointer());
        if ((parentNode->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        {
            const QModelIndex &insertBelowIndex = index(dstRow, 0, parent);
            if ((parentNode->GetFlags() & PackageBaseNode::FLAGS_CONTROL) != 0)
            {
                ControlNode *parentControl = dynamic_cast<ControlNode*>(parentNode);
                if (parentControl)
                {
                    ControlNode *belowThis = static_cast<ControlNode*>(insertBelowIndex.internalPointer());
                    if (belowThis)
                        parentControl->InsertBelow(node, belowThis);
                    else
                        parentControl->Add(node);
                }
                else
                {
                    DVASSERT(false);
                }
            }
            else
            {
                PackageControlsNode *parentControls = dynamic_cast<PackageControlsNode*>(parentNode);
                if (parentControls)
                {
                    ControlNode *belowThis = static_cast<ControlNode*>(insertBelowIndex.internalPointer());
                    if (belowThis)
                        parentControls->InsertBelow(node, belowThis);
                    else
                        parentControls->Add(node);
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::RemoveNode(ControlNode *node)
{
    PackageBaseNode *parentNode = node->GetParent();
    if ((parentNode->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
    {
        PackageControlsNode *controls = dynamic_cast<PackageControlsNode*>(parentNode);
        if (controls)
        {
            controls->Remove(node);
        }
        else
        {
            ControlNode *parentControl = dynamic_cast<ControlNode*>(parentNode);
            if (parentControl)
            {
                parentControl->Remove(node);
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}
