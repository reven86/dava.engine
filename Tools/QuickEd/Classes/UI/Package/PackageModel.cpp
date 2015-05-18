#include "PackageModel.h"

#include <QIcon>
#include <QAction>
#include <QUrl>

#include "DAVAEngine.h"
#include "Base/ObjectFactory.h"

#include "UI/IconHelper.h"
#include "UI/QtModelPackageCommandExecutor.h"
#include "Utils/QtDavaConvertion.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/ControlPrototype.h"
#include "Model/PackageHierarchy/ControlsContainerNode.h"
#include "Model/YamlPackageSerializer.h"

#include "PackageMimeData.h"

using namespace DAVA;

PackageModel::PackageModel(PackageNode *_root, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent)
    : QAbstractItemModel(parent)
    , root(SafeRetain(_root))
    , commandExecutor(SafeRetain(_commandExecutor))
{
}

PackageModel::~PackageModel()
{
    SafeRelease(root);
    SafeRelease(commandExecutor);
}

void PackageModel::emitNodeChanged(PackageBaseNode *node)
{
    QModelIndex index = indexByNode(node);
    emit dataChanged(index, index);
}

QModelIndex PackageModel::indexByNode(PackageBaseNode *node) const
{
    PackageBaseNode *parent = node->GetParent();
    if (parent == nullptr)
        return QModelIndex();
    
    if (parent)
        return createIndex(parent->GetIndex(node), 0, node);
    else
        return createIndex(0, 0, parent);
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, root->Get(row));

    PackageBaseNode *node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex PackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    PackageBaseNode *node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode *parent = node->GetParent();
    if (nullptr == parent || parent == root)
        return QModelIndex();
    
    if (parent->GetParent())
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    else
        return createIndex(0, 0, parent);
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return root ? root->GetCount() : 0;
    
    return static_cast<PackageBaseNode*>(parent.internalPointer())->GetCount();
}

int PackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
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
            return node->GetControl() != nullptr ? QIcon(IconHelper::GetIconPathForUIControl(node->GetControl())) : QVariant();
            
        case Qt::CheckStateRole:
            if (node->GetControl())
                return node->GetControl()->GetVisibleForUIEditor() ? Qt::Checked : Qt::Unchecked;
            else
                return QVariant();
            
        case Qt::ToolTipRole:
            if (node->GetControl() != nullptr)
            {
                ControlNode *controlNode = DynamicTypeCheck<ControlNode *>(node);
                QString toolTip = QString("class: ") + controlNode->GetControl()->GetControlClassName().c_str();
                if (!controlNode->GetControl()->GetCustomControlClassName().empty())
                {
                    toolTip += QString("\ncustom class: ") + controlNode->GetControl()->GetCustomControlClassName().c_str();
                }

                if (controlNode->GetPrototype())
                {
                    bool withPackage = true; // TODO fix for currentPackage
                    toolTip += QString("\nprototype: ") + controlNode->GetPrototype()->GetName(withPackage).c_str();
                }
                return toolTip;
            }
            break;
            
        case Qt::TextColorRole:
            return (flags & prototypeFlag) != 0 ? QColor(Qt::blue) : QColor(Qt::black);
            
        case Qt::BackgroundRole:
            return (flags & controlFlag) == 0 ? QColor(Qt::lightGray) : QColor(Qt::white);
            
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

bool PackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

Qt::ItemFlags PackageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    
    const PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    if (node->CanCopy())
        flags |= Qt::ItemIsDragEnabled;
    if (node->IsInsertingSupported())
        flags |= Qt::ItemIsDropEnabled;
    
    return flags;
}

Qt::DropActions PackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList PackageModel::mimeTypes() const
{
    QStringList types;
    types << PackageMimeData::MIME_TYPE;
    types << "text/plain";
    types << "text/uri-list";
    return types;
}

QMimeData *PackageModel::mimeData(const QModelIndexList &indices) const
{
    PackageMimeData *mimeData = new PackageMimeData();
    
    for (const QModelIndex &index : indices)
    {
        if (index.isValid())
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
            if (controlNode && controlNode->CanCopy())
            {
                mimeData->AddControlNode(controlNode);
            }
        }
    }
    
    YamlPackageSerializer serializer;
    root->Serialize(&serializer, mimeData->GetControlNodes());
    String str = serializer.WriteToString();
    mimeData->setText(QString::fromStdString(str));

    return mimeData;
}

bool PackageModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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

    ControlsContainerNode *parentNode = dynamic_cast<ControlsContainerNode*>(static_cast<PackageBaseNode*>(parent.internalPointer()));
    
    if (parentNode && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData *controlMimeData = dynamic_cast<const PackageMimeData*>(data);
        if (!controlMimeData)
            return false;

        const Vector<ControlNode *> &srcNodes = controlMimeData->GetControlNodes();
        if (srcNodes.empty())
            return false;
        
        if (action == Qt::CopyAction)
            commandExecutor->CopyControls(srcNodes, parentNode, rowIndex);
        else if (action == Qt::MoveAction)
            commandExecutor->MoveControls(srcNodes, parentNode, rowIndex);
        else
            return false;
        
        return true;
    }
    else if (data->hasFormat("text/uri-list") && data->hasText())
    {
        QStringList list = data->text().split("\n");
        for (const QString &str : list)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                FilePath path(url.toLocalFile().toStdString());
                if (root->FindImportedPackage(path) == nullptr)
                {
                }
            }
        }
    }
    else if (parentNode && data->hasFormat("text/plain") && data->hasText())
    {
        String string = data->text().toStdString();
        
        if (!commandExecutor->Paste(root, parentNode, rowIndex, string))
        {
            String controlName = QStringToString(data->text());
            size_t slashIndex = controlName.find("/");
            ControlNode *node = nullptr;
            
            if (slashIndex != String::npos)
            {
                String packName = controlName.substr(0, slashIndex);
                controlName = controlName.substr(slashIndex + 1, controlName.size() - slashIndex - 1);
                PackageControlsNode *packageControls = root->GetImportedPackagesNode()->FindPackageControlsNodeByName(packName);
                if (packageControls)
                {
                    ControlNode *prototypeControl = packageControls->FindControlNodeByName(controlName);
                    if (prototypeControl)
                    {
                        node = ControlNode::CreateFromPrototype(prototypeControl, packageControls->GetPackageRef());
                    }
                }
            }
            else
            {
                UIControl *control = ObjectFactory::Instance()->New<UIControl>(controlName);
                if (control)
                {
                    node = ControlNode::CreateFromControl(control);
                    SafeRelease(control);
                }
                else
                {
                    ControlNode *prototypeControl = root->GetPackageControlsNode()->FindControlNodeByName(controlName);
                    if (prototypeControl)
                    {
                        node = ControlNode::CreateFromPrototype(prototypeControl, root->GetPackageControlsNode()->GetPackageRef());
                    }
                }
            }
            
            if (node)
            {
                commandExecutor->InsertControl(node, parentNode, rowIndex);
                SafeRelease(node);
            }
        }
        return true;
    }

    return false;
}

void PackageModel::InsertControlNode(ControlNode *node, ControlsContainerNode *dest, int row)
{
    QModelIndex destIndex = indexByNode(dest);
    beginInsertRows(destIndex, row, row);
    dest->InsertAtIndex(row, node);
    endInsertRows();
}

void PackageModel::RemoveControlNode(ControlNode *node, ControlsContainerNode *parent)
{
    QModelIndex parentIndex = indexByNode(parent);
    int index = parent->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
    parent->Remove(node);
    endRemoveRows();
}

void PackageModel::InsertImportedPackage(PackageControlsNode *node, PackageNode *dest, int destRow)
{
    QModelIndex destIndex = indexByNode(dest);
    beginInsertRows(destIndex, destRow, destRow);
    dest->GetImportedPackagesNode()->InsertAtIndex(destRow, node);
    endInsertRows();
}

void PackageModel::RemoveImportedPackage(PackageControlsNode *node, PackageNode *parent)
{
    QModelIndex parentIndex = indexByNode(parent);
    int index = parent->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
    parent->GetImportedPackagesNode()->Remove(node);
    endRemoveRows();
}

