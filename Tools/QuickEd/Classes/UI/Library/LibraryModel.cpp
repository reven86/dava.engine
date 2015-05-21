#include "LibraryModel.h"

#include <QIcon>
#include <QMimeData>

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ClassProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"

#include "Utils/QtDavaConvertion.h"
#include "UI/IconHelper.h"

using namespace DAVA;

LibraryModel::LibraryModel(PackageNode *_root, QObject *parent) 
    : QAbstractItemModel(parent)
    , root(SafeRetain(_root))
{
    root->AddListener(this);
    defaultControls.push_back("UIControl");
    defaultControls.push_back("UIButton");
    defaultControls.push_back("UIStaticText");
    defaultControls.push_back("UITextField");
    defaultControls.push_back("UISlider");
    defaultControls.push_back("UIList");
    defaultControls.push_back("UIListCell");
    defaultControls.push_back("UIScrollBar");
    defaultControls.push_back("UIScrollView");
    defaultControls.push_back("UISpinner");
    defaultControls.push_back("UISwitch");
    defaultControls.push_back("UIParticles");
}

LibraryModel::~LibraryModel()
{
    root->RemoveListener(this);
    SafeRelease(root);
}

QModelIndex LibraryModel::indexByNode(PackageBaseNode *node) const
{
    PackageBaseNode *parent = node->GetParent();
    return nullptr == parent ? QModelIndex()
        : createIndex(parent->GetIndex(node) + defaultControls.size(), 0, node);
}

QModelIndex LibraryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    else if (IsDefault(row))
    {
        return createIndex(row, column);
    }
    else if (!parent.isValid())
    {
        return createIndex(row, column, root->GetPackageControlsNode()->Get(row - defaultControls.size()));
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row - defaultControls.size()));
}

QModelIndex LibraryModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || IsDefault(child.row()))
    {
        return QModelIndex();
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode *parent = node->GetParent();
    if (nullptr == parent || parent == root->GetPackageControlsNode())
    {
        return QModelIndex();
    }
    else
    {
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    }
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return nullptr != root->GetPackageControlsNode() ? root->GetPackageControlsNode()->GetCount() + defaultControls.size() : 0;
    }
    else if (IsDefault(parent.row()))
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

int LibraryModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    int row = index.row();
    switch (role)
    {
    case Qt::DisplayRole:
        return IsDefault(row) ? defaultControls.at(row)
            : QString::fromStdString(node->GetName());
    case Qt::DecorationRole:
        if (IsDefault(row))
        {
            QString className = defaultControls.at(row);
            return QIcon(IconHelper::GetIconPathForClassName(className));
        }
        else
        {
            if (nullptr == node->GetControl())
            {
                return QVariant();
            }
            ControlNode *controlNode = DynamicTypeCheck<ControlNode *>(node);
            DVASSERT(controlNode);
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
    default:
        return QVariant();
    }
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled;
}

QStringList LibraryModel::mimeTypes() const
{
    return QStringList() << "text/plain";
}

QMimeData *LibraryModel::mimeData(const QModelIndexList &indexes) const
{
    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            QMimeData *data = new QMimeData();
            data->setText(defaultControls.at(index.row()));
            return data;
        }
    }
    return nullptr;
}

bool LibraryModel::IsDefault(int row) const
{
    return row < defaultControls.size();
}

void LibraryModel::ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property)
{
    //just check
}

void LibraryModel::ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int row)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, row, row);
}

void LibraryModel::ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int row)
{
    endInsertRows();
}

void LibraryModel::ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void LibraryModel::ControlWasRemoved(ControlNode *node, ControlsContainerNode *from)
{
    endRemoveRows();
}

void LibraryModel::ImportedPackageWillBeAdded(PackageControlsNode *node, PackageNode *to, int index)
{
    QModelIndex destIndex = indexByNode(to);
    beginInsertRows(destIndex, index, index);
}

void LibraryModel::ImportedPackageWasAdded(PackageControlsNode *node, PackageNode *to, int index)
{
    endInsertRows();
}

void LibraryModel::ImportedPackageWillBeRemoved(PackageControlsNode *node, PackageNode *from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void LibraryModel::ImportedPackageWasRemoved(PackageControlsNode *node, PackageNode *from)
{
    endRemoveRows();
}
