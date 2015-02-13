#include "LibraryModel.h"

#include <QIcon>
#include <QMimeData>

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Utils/QtDavaConvertion.h"
#include "UI/IconHelper.h"

LibraryModel::LibraryModel(PackageNode *node, QObject *parent) : QAbstractItemModel(parent)
{
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

    defaultControlsCount = defaultControls.size();
    
    PackageControlsNode *packageControls = node->GetPackageControlsNode();
    for (int j = 0; j < packageControls->GetCount(); j++)
    {
        ControlNode *node = static_cast<ControlNode*>(packageControls->Get(j));
        defaultControls.push_back(node->GetName());
    }

    for (int i = 0; i < node->GetImportedPackagesNode()->GetCount(); i++)
    {
        ImportedPackagesNode *importedPackage = (ImportedPackagesNode*) node->GetImportedPackagesNode()->Get(i);
        for (int j = 0; j < importedPackage->GetCount(); j++)
        {
            defaultControls.push_back(importedPackage->GetName() + "/" + importedPackage->Get(j)->GetName());
        }
    }
}

LibraryModel::~LibraryModel()
{
    
}

QModelIndex LibraryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    
    if (!parent.isValid())
        return createIndex(row, column, row);
    
    return QModelIndex();
    
}

QModelIndex LibraryModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return (int) defaultControls.size();
    
    return 0;
}

int LibraryModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return StringToQString(defaultControls[index.row()]);
    }
    else if (role == Qt::DecorationRole)
    {
        QString className = StringToQString(defaultControls[index.internalId()]);
        QIcon icon;
        if (index.internalId() < defaultControlsCount)
            icon = QIcon(IconHelper::GetIconPathForClassName(className));
        else
            icon = QIcon(IconHelper::GetCustomIconPath());
        return icon;
    }
    return QVariant();
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    flags |= Qt::ItemIsDragEnabled;
    
//    const PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
//    if ((node->GetFlags() & PackageBaseNode::FLAGS_CONTROL) != 0)
    
    return flags;
}

QStringList LibraryModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

QMimeData *LibraryModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            data->setText(StringToQString(defaultControls[index.row()]));
            break;
        }
    }
    return data;
}
