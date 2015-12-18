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
#include "Model/PackageHierarchy/ControlsContainerNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/NameProperty.h"
#include "Model/ControlProperties/ClassProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"
#include "Model/ControlProperties/PrototypeNameProperty.h"
#include "Model/YamlPackageSerializer.h"

#include "PackageMimeData.h"

using namespace DAVA;

PackageModel::PackageModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

void PackageModel::Reset(std::weak_ptr<PackageNode> package_, std::weak_ptr<QtModelPackageCommandExecutor> executor_)
{
    beginResetModel();
    {
        auto packagePtr = package.lock();
        if (nullptr != packagePtr)
        {
            packagePtr->ControlPropertyWasChanged.Disconnect(&connectionTracker);
            packagePtr->StylePropertyWasChanged.Disconnect(&connectionTracker);
            packagePtr->ControlWillBeAdded.Disconnect(&connectionTracker);
            packagePtr->ControlWasAdded.Disconnect(&connectionTracker);
            packagePtr->ControlWillBeRemoved.Disconnect(&connectionTracker);
            packagePtr->ControlWasRemoved.Disconnect(&connectionTracker);
            packagePtr->StyleWillBeAdded.Disconnect(&connectionTracker);
            packagePtr->StyleWasAdded.Disconnect(&connectionTracker);
            packagePtr->StyleWillBeRemoved.Disconnect(&connectionTracker);
            packagePtr->StyleWasRemoved.Disconnect(&connectionTracker);
            packagePtr->ImportedPackageWillBeAdded.Disconnect(&connectionTracker);
            packagePtr->ImportedPackageWasAdded.Disconnect(&connectionTracker);
            packagePtr->ImportedPackageWillBeRemoved.Disconnect(&connectionTracker);
            packagePtr->ImportedPackageWasRemoved.Disconnect(&connectionTracker);
        }
    }
    package = package_;
    commandExecutor = executor_;
    {
        auto packagePtr = package.lock();
        if (nullptr != packagePtr)
        {
            auto id = packagePtr->ControlPropertyWasChanged.Connect(this, &PackageModel::OnControlPropertyWasChanged);
            packagePtr->ControlPropertyWasChanged.Track(id, &connectionTracker);
            id = packagePtr->StylePropertyWasChanged.Connect(this, &PackageModel::OnStylePropertyWasChanged);
            packagePtr->StylePropertyWasChanged.Track(id, &connectionTracker);
            id = packagePtr->ControlWillBeAdded.Connect(this, &PackageModel::OnControlWillBeAdded);
            packagePtr->ControlWillBeAdded.Track(id, &connectionTracker);
            id = packagePtr->ControlWasAdded.Connect(this, &PackageModel::OnControlWasAdded);
            packagePtr->ControlWasAdded.Track(id, &connectionTracker);
            id = packagePtr->ControlWillBeRemoved.Connect(this, &PackageModel::OnControlWillBeRemoved);
            packagePtr->ControlWillBeRemoved.Track(id, &connectionTracker);
            id = packagePtr->ControlWasRemoved.Connect(this, &PackageModel::OnControlWasRemoved);
            packagePtr->ControlWasRemoved.Track(id, &connectionTracker);
            id = packagePtr->StyleWillBeAdded.Connect(this, &PackageModel::OnStyleWillBeAdded);
            packagePtr->StyleWillBeAdded.Track(id, &connectionTracker);
            id = packagePtr->StyleWasAdded.Connect(this, &PackageModel::OnStyleWasAdded);
            packagePtr->StyleWasAdded.Track(id, &connectionTracker);
            id = packagePtr->StyleWillBeRemoved.Connect(this, &PackageModel::OnStyleWillBeRemoved);
            packagePtr->StyleWillBeRemoved.Track(id, &connectionTracker);
            id = packagePtr->StyleWasRemoved.Connect(this, &PackageModel::OnStyleWasRemoved);
            packagePtr->StyleWasRemoved.Track(id, &connectionTracker);
            id = packagePtr->ImportedPackageWillBeAdded.Connect(this, &PackageModel::OnImportedPackageWillBeAdded);
            packagePtr->ImportedPackageWillBeAdded.Track(id, &connectionTracker);
            id = packagePtr->ImportedPackageWasAdded.Connect(this, &PackageModel::OnImportedPackageWasAdded);
            packagePtr->ImportedPackageWasAdded.Track(id, &connectionTracker);
            id = packagePtr->ImportedPackageWillBeRemoved.Connect(this, &PackageModel::OnImportedPackageWillBeRemoved);
            packagePtr->ImportedPackageWillBeRemoved.Track(id, &connectionTracker);
            id = packagePtr->ImportedPackageWasRemoved.Connect(this, &PackageModel::OnImportedPackageWasRemoved);
            packagePtr->ImportedPackageWasRemoved.Track(id, &connectionTracker);
        }
    }
    endResetModel();
}

QModelIndex PackageModel::indexByNode(PackageBaseNode *node) const
{
    PackageBaseNode *parent = node->GetParent();
    if (parent == nullptr)
    {
        return QModelIndex();
    }
    return createIndex(parent->GetIndex(node), 0, node);
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    if (!parent.isValid())
    {
        auto packagePtr = package.lock();
        DVASSERT(packagePtr != nullptr);
        if (packagePtr != nullptr)
        {
            return createIndex(row, column, packagePtr->Get(row));
        }
        return QModelIndex();
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex PackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        DVASSERT(false && "invalid child passed to parent function");
        return QModelIndex();
    }

    PackageBaseNode *node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode *parent = node->GetParent();
    auto packagePtr = package.lock();
    DVASSERT(packagePtr != nullptr);
    if (packagePtr != nullptr)
    {
        DVASSERT(nullptr != parent);
        if (nullptr == parent || parent == packagePtr.get())
        {
            return QModelIndex();
        }
        DVASSERT(nullptr != parent->GetParent());
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    }
    return QModelIndex();
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        auto packagePtr = package.lock();
        if (packagePtr != nullptr)
        {
            return packagePtr->GetCount();
        }
        return 0;
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
        DVASSERT(false && "invalid index passed to data function");
        return QVariant();
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    ControlNode *controlNode = dynamic_cast<ControlNode*>(node);

    if (controlNode)
    {
        switch(role)
        {
            case Qt::EditRole:
            case Qt::DisplayRole:
                return StringToQString(node->GetName());
                
            case Qt::DecorationRole:
                if (controlNode->GetRootProperty()->GetCustomClassProperty()->IsOverridden())
                {
                    return QIcon(IconHelper::GetCustomIconPath());
                }
                else
                {
                    const String &className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
                    return QIcon(IconHelper::GetIconPathForClassName(QString::fromStdString(className)));
                }
                
            case Qt::CheckStateRole:
                return controlNode->GetControl()->GetVisibleForUIEditor() ? Qt::Checked : Qt::Unchecked;
                
            case Qt::ToolTipRole:
            {
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
                
            case Qt::TextColorRole:
                return controlNode->GetPrototype() != nullptr ? QColor(Qt::blue) : QColor(Qt::black);
                
            case Qt::BackgroundRole:
                return QColor(Qt::white);
                
            case Qt::FontRole:
            {
                QFont myFont;
                if (controlNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
                    myFont.setBold(true);
                
                if (node->IsReadOnly())
                    myFont.setItalic(true);
                
                return myFont;
            }
        }
    }
    else
    {
        StyleSheetNode *styleSheet = dynamic_cast<StyleSheetNode*>(node);
        if (styleSheet)
        {
            switch(role)
            {
                case Qt::DisplayRole:
                    return StringToQString(node->GetName());
                    
                case Qt::TextColorRole:
                    return QColor(Qt::darkGreen);
                    
                case Qt::BackgroundRole:
                    return QColor(Qt::white);
                    
                case Qt::FontRole:
                {
                    QFont myFont;
                    if (node->IsReadOnly())
                        myFont.setItalic(true);
                    
                    return myFont;
                }
            }
        }
        else
        {
            switch(role)
            {
                case Qt::DisplayRole:
                    return StringToQString(node->GetName());
                    
                case Qt::TextColorRole:
                    return QColor(Qt::black);
                    
                case Qt::BackgroundRole:
                    return QColor(Qt::lightGray);
                    
                case Qt::FontRole:
                {
                    QFont myFont;
                    myFont.setBold(true);
                    
                    if (node->IsReadOnly())
                        myFont.setItalic(true);
                    
                    return myFont;
                }
            }
        }
    }

    return QVariant();
}

bool PackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        DVASSERT(false && "invalid index passed to setData");
        return false;
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    auto control = node->GetControl();
    if(nullptr == control)
    {
        return false;
    }
    if (role == Qt::CheckStateRole)
    {
        control->SetVisibleForUIEditor(value.toBool());
        return true;
    }
    if(role == Qt::EditRole)
    {
        auto commandExecutorPtr = commandExecutor.lock();
        DVASSERT(nullptr != commandExecutorPtr);
        if (nullptr != commandExecutorPtr)
        {
            ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
            DVASSERT(controlNode);
            auto prop = controlNode->GetRootProperty()->GetNameProperty();
            const auto& newName = value.toString().toStdString();
            if (newName != node->GetName())
            {
                commandExecutorPtr->ChangeProperty(controlNode, prop, DAVA::VariantType(newName));
                return true;
            }
        }
    }
    return false;
}

Qt::ItemFlags PackageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    
    const PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    if (node->CanCopy())
    {
        flags |= Qt::ItemIsDragEnabled;
    }
    if (node->IsInsertingControlsSupported() || node->IsInsertingPackagesSupported() || node->IsInsertingStylesSupported())
    {
        flags |= Qt::ItemIsDropEnabled;
    }
    if(node->IsEditingSupported())
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

Qt::DropActions PackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
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
    auto packagePtr = package.lock();
    DVASSERT(nullptr != packagePtr);
    if (nullptr == packagePtr)
    {
        return nullptr;
    }

    PackageMimeData *mimeData = new PackageMimeData();
    
    for (const QModelIndex &index : indices)
    {
        if (index.isValid())
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->CanCopy())
            {
                ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
                if (nullptr != controlNode)
                {
                    mimeData->AddControl(controlNode);
                }
                else
                {
                    StyleSheetNode *style = dynamic_cast<StyleSheetNode*>(node);
                    if (nullptr != style)
                    {
                        mimeData->AddStyle(style);
                    }
                }
                
            }
        }
    }
    
    YamlPackageSerializer serializer;
    serializer.SerializePackageNodes(packagePtr.get(), mimeData->GetControls(), mimeData->GetStyles());
    String str = serializer.WriteToString();
    mimeData->setText(QString::fromStdString(str));

    return mimeData;
}

int PackageModel::GetRowIndex(int row, const QModelIndex& parent) const
{
    if (row != -1)
    {
        return row;
    }
    return rowCount(parent);
}

bool PackageModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int /*column*/, const QModelIndex& parent)
{
    int rowIndex = GetRowIndex(row, parent);
    PackageBaseNode* destNode = static_cast<PackageBaseNode*>(parent.internalPointer());
    OnDropMimeData(data, action, destNode, rowIndex, Vector2(-1.0f, -1.0f));
    return true; //if we can drop - we must drop. Otherwise CanDropMimeData must return false;
}

void PackageModel::OnDropMimeData(const QMimeData* data, Qt::DropAction action, PackageBaseNode* destNode, int destIndex, const DAVA::Vector2& pos)
{
    auto commandExecutorPtr = commandExecutor.lock();
    DVASSERT(nullptr != commandExecutorPtr);

    auto packagePtr = package.lock();
    DVASSERT(nullptr != packagePtr);

    ControlsContainerNode *destControlContainer = dynamic_cast<ControlsContainerNode*>(destNode);
    StyleSheetsNode *destStylesContainer = dynamic_cast<StyleSheetsNode*>(destNode);

    if (destControlContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData *controlMimeData = dynamic_cast<const PackageMimeData*>(data);
        DVASSERT(nullptr != controlMimeData);

        const Vector<ControlNode *> &srcControls = controlMimeData->GetControls();
        DVASSERT(!srcControls.empty());

        switch (action)
        {
        case Qt::CopyAction:
            commandExecutorPtr->CopyControls(srcControls, destControlContainer, destIndex, pos);
            break;
        case Qt::MoveAction:
            emit BeforeNodesMoved(SelectedNodes(srcControls.begin(), srcControls.end()));
            commandExecutorPtr->MoveControls(srcControls, destControlContainer, destIndex, pos);
            emit NodesMoved(SelectedNodes(srcControls.begin(), srcControls.end()));
            break;
        case Qt::LinkAction:
            commandExecutorPtr->InsertInstances(srcControls, destControlContainer, destIndex, pos);
            break;
        default:
            DVASSERT(false && "unrecognised action!");
        }
    }
    else if (destStylesContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData *mimeData = dynamic_cast<const PackageMimeData*>(data);
        DVASSERT(nullptr != mimeData)

        const Vector<StyleSheetNode *> &srcStyles = mimeData->GetStyles();
        DVASSERT(!srcStyles.empty());

        switch (action)
        {
        case Qt::CopyAction:
            commandExecutorPtr->CopyStyles(srcStyles, destStylesContainer, destIndex);
            break;
        case Qt::MoveAction:
            emit BeforeNodesMoved(SelectedNodes(srcStyles.begin(), srcStyles.end()));
            commandExecutorPtr->MoveStyles(srcStyles, destStylesContainer, destIndex);
            emit NodesMoved(SelectedNodes(srcStyles.begin(), srcStyles.end()));
            break;
        default:
            DVASSERT(false && "unrecognised action!");
        }
    }
    else if (data->hasFormat("text/uri-list") && data->hasText())
    {
        QStringList list = data->text().split("\n");
        Vector<FilePath> packages;
        for (const QString &str : list)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                packages.push_back(FilePath(url.toLocalFile().toStdString()));
            }
        }
        if (!packages.empty())
        {
            commandExecutorPtr->AddImportedPackagesIntoPackage(packages, packagePtr.get());
        }
    }
    else if (destNode && data->hasFormat("text/plain") && data->hasText())
    {
        String string = data->text().toStdString();
        commandExecutorPtr->Paste(packagePtr.get(), destNode, destIndex, string, pos);
    }
}

void PackageModel::OnControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }

    else if (property->GetName() == "Custom Class")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DecorationRole);
    }
}

void PackageModel::OnStylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }
}

void PackageModel::OnControlWillBeAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, row, row);
}

void PackageModel::OnControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    endInsertRows();
}

void PackageModel::OnControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::OnControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    endRemoveRows();
}

void PackageModel::OnStyleWillBeAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::OnStyleWasAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index)
{
    endInsertRows();
}

void PackageModel::OnStyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::OnStyleWasRemoved(StyleSheetNode* node, StyleSheetsNode* from)
{
    endRemoveRows();
}

void PackageModel::OnImportedPackageWillBeAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    QModelIndex destIndex = indexByNode(to);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::OnImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    endInsertRows();
}

void PackageModel::OnImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::OnImportedPackageWasRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    endRemoveRows();
}
