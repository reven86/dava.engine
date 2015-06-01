/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/NameProperty.h"
#include "Model/ControlProperties/ClassProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"
#include "Model/ControlProperties/PrototypeNameProperty.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/EditorUIPackageBuilder.h"

#include "PackageMimeData.h"

using namespace DAVA;

PackageModel::PackageModel(PackageNode *_root, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent)
    : QAbstractItemModel(parent)
    , root(SafeRetain(_root))
    , commandExecutor(SafeRetain(_commandExecutor))
{
    root->AddListener(this);
}

PackageModel::~PackageModel()
{
    root->RemoveListener(this);
    SafeRelease(root);
    SafeRelease(commandExecutor);
}

QModelIndex PackageModel::indexByNode(PackageBaseNode *node) const
{
    PackageBaseNode *parent = node->GetParent();
    return nullptr == parent ? QModelIndex() 
        : createIndex(parent->GetIndex(node), 0, node);
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    if (!parent.isValid())
    {
        return createIndex(row, column, root->Get(row));
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex PackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return QModelIndex();
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode *parent = node->GetParent();
    if (nullptr == parent || parent == root)
    {
        return QModelIndex();
    }
    if (nullptr != parent->GetParent())
    {
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    }
    else
    {
        return createIndex(0, 0, parent);
    }
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return nullptr != root ? root->GetCount() : 0;
    }
    return static_cast<PackageBaseNode*>(parent.internalPointer())->GetCount();
}

int PackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    
    int prototypeFlag = PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
    int controlFlag = PackageBaseNode::FLAG_CONTROL_CREATED_FROM_CLASS | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
    int flags = node->GetFlags();
    switch(role)
    {
        case Qt::DisplayRole:
            return StringToQString(node->GetName());
            
        case Qt::DecorationRole:
            if (node->GetControl())
            {
                ControlNode *controlNode = DynamicTypeCheck<ControlNode *>(node);
                if (controlNode)
                {
                    if (controlNode->GetRootProperty()->GetCustomClassProperty()->IsSet())
                    {
                        return QIcon(IconHelper::GetCustomIconPath());
                    }
                    else
                    {
                        const String &className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
                        return QIcon(IconHelper::GetIconPathForClassName(QString::fromStdString(className)));
                    }
                }
            }
            return QVariant();
            
        case Qt::CheckStateRole:
            if (node->GetControl())
                return node->GetControl()->GetVisibleForUIEditor() ? Qt::Checked : Qt::Unchecked;
            else
                return QVariant();
            
        case Qt::ToolTipRole:
            if (node->GetControl() != nullptr)
            {
                ControlNode *controlNode = DynamicTypeCheck<ControlNode *>(node);
                const String &prototype = controlNode->GetRootProperty()->GetPrototypeProperty()->GetPrototypeName();
                const String &className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
                const String &customClassName = controlNode->GetRootProperty()->GetCustomClassProperty()->GetCustomClassName();
                QString toolTip = QString("class: ") + className.c_str();
                if (!customClassName.empty())
                {
                    toolTip += QString("\ncustom class: ") + customClassName.c_str();
                }

                if (controlNode->GetPrototype())
                {
                    toolTip += QString("\nprototype: ") + prototype.c_str();
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
    //TODO: DF-6265, add insert import packages here
    
    return flags;
}

Qt::DropActions PackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction  | Qt::LinkAction;
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
        if (nullptr == controlMimeData)
            return false;

        const Vector<ControlNode *> &srcNodes = controlMimeData->GetControlNodes();
        if (srcNodes.empty())
            return false;
        switch (action)
        {
        default: return false;
        case Qt::CopyAction:
            commandExecutor->CopyControls(srcNodes, parentNode, rowIndex);
            break;
        case Qt::MoveAction:
            commandExecutor->MoveControls(srcNodes, parentNode, rowIndex);
            break;
        case Qt::LinkAction:
            return false;//TODO: DF-6471, d_belsky must implement linkControls
            break;
        }
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
                    //TODO: DF-6265, implement here
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

void PackageModel::ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property)
{
    QModelIndex index = indexByNode(node);
    emit dataChanged(index, index);
}

void PackageModel::ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int row)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, row, row);
}

void PackageModel::ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int row)
{
    endInsertRows();
}

void PackageModel::ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ControlWasRemoved(ControlNode *node, ControlsContainerNode *from)
{
    endRemoveRows();
}

void PackageModel::ImportedPackageWillBeAdded(PackageControlsNode *node, PackageNode *to, int index)
{
    QModelIndex destIndex = indexByNode(to);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::ImportedPackageWasAdded(PackageControlsNode *node, PackageNode *to, int index)
{
    endInsertRows();
}

void PackageModel::ImportedPackageWillBeRemoved(PackageControlsNode *node, PackageNode *from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ImportedPackageWasRemoved(PackageControlsNode *node, PackageNode *from)
{
    endRemoveRows();
}
