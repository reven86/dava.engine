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

LibraryModel::LibraryModel(PackageNode *node, QObject *parent) : QAbstractListModel(parent)
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
    {
        return QModelIndex();
    }
    if (!parent.isValid())
    {
        return createIndex(row, column, row);
    }
    return QModelIndex();
    
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return defaultControls.size();
    }
    return 0;
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
        return index.internalId() < defaultControls.size() ? QIcon(IconHelper::GetIconPathForClassName(className))
                                                           : QIcon(IconHelper::GetCustomIconPath());
    }
    return QVariant();
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
            data->setText(StringToQString(defaultControls[index.row()]));
            return data;
        }
    }
}
