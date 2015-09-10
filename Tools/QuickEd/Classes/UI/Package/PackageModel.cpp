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
        return false;
    
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
        ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
        DVASSERT(controlNode);
        auto prop = controlNode->GetRootProperty()->GetNameProperty();
        const auto &newName = value.toString().toStdString();
        if (newName != node->GetName())
        {
            commandExecutor->ChangeProperty(controlNode, prop, DAVA::VariantType(newName));
            return true;
        }
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
    PackageMimeData *mimeData = new PackageMimeData();
    
    for (const QModelIndex &index : indices)
    {
        if (index.isValid())
        {
            PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->CanCopy())
            {
                ControlNode *controlNode = dynamic_cast<ControlNode*>(node);
                if (controlNode)
                {
                    mimeData->AddControl(controlNode);
                }
                else
                {
                    StyleSheetNode *style = dynamic_cast<StyleSheetNode*>(node);
                    if (style)
                        mimeData->AddStyle(style);
                }
                
            }
        }
    }
    
    YamlPackageSerializer serializer;
    serializer.SerializePackageNodes(root, mimeData->GetControls(), mimeData->GetStyles());
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

    
    PackageBaseNode *destNode = static_cast<PackageBaseNode*>(parent.internalPointer());
    
    ControlsContainerNode *destControlContainer = dynamic_cast<ControlsContainerNode*>(destNode);
    StyleSheetsNode *destStylesContainer = dynamic_cast<StyleSheetsNode*>(destNode);
    
    if (destControlContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData *controlMimeData = dynamic_cast<const PackageMimeData*>(data);
        if (!controlMimeData)
            return false;

        const Vector<ControlNode *> &srcControls = controlMimeData->GetControls();
        if (srcControls.empty())
            return false;
        
        if (action == Qt::CopyAction)
            commandExecutor->CopyControls(srcControls, destControlContainer, rowIndex);
        else if (action == Qt::MoveAction)
            commandExecutor->MoveControls(srcControls, destControlContainer, rowIndex);
        else if (action == Qt::LinkAction)
            commandExecutor->InsertInstances(srcControls, destControlContainer, rowIndex);
        else
            return false;

        return true;
    }
    else if (destStylesContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData *mimeData = dynamic_cast<const PackageMimeData*>(data);
        if (!mimeData)
            return false;
        
        const Vector<StyleSheetNode *> &srcStyles = mimeData->GetStyles();
        if (srcStyles.empty())
            return false;
        
        if (action == Qt::CopyAction)
            commandExecutor->CopyStyles(srcStyles, destStylesContainer, rowIndex);
        else if (action == Qt::MoveAction)
            commandExecutor->MoveStyles(srcStyles, destStylesContainer, rowIndex);
        else
            return false;
        
        return true;
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
            commandExecutor->AddImportedPackagesIntoPackage(packages, root);
        }
    }
    else if (destNode && data->hasFormat("text/plain") && data->hasText())
    {
        String string = data->text().toStdString();
        commandExecutor->Paste(root, destNode, rowIndex, string);
        return true;
    }

    return false;
}

void PackageModel::ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property)
{
    if (property->GetName() == "Name")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }
}

void PackageModel::StylePropertyWasChanged(StyleSheetNode *node, AbstractProperty *property)
{
    QModelIndex index = indexByNode(node);
    emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
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

void PackageModel::StyleWillBeAdded(StyleSheetNode *node, StyleSheetsNode *destination, int index)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::StyleWasAdded(StyleSheetNode *node, StyleSheetsNode *destination, int index)
{
    endInsertRows();
}

void PackageModel::StyleWillBeRemoved(StyleSheetNode *node, StyleSheetsNode *from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::StyleWasRemoved(StyleSheetNode *node, StyleSheetsNode *from)
{
    endRemoveRows();
}

void PackageModel::ImportedPackageWillBeAdded(PackageNode *node, ImportedPackagesNode *to, int index)
{
    QModelIndex destIndex = indexByNode(to);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::ImportedPackageWasAdded(PackageNode *node, ImportedPackagesNode *to, int index)
{
    endInsertRows();
}

void PackageModel::ImportedPackageWillBeRemoved(PackageNode *node, ImportedPackagesNode *from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ImportedPackageWasRemoved(PackageNode *node, ImportedPackagesNode *from)
{
    endRemoveRows();
}
