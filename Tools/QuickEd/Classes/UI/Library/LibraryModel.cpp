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
    libraryControls.push_back("UIControl");
    libraryControls.push_back("UIButton");
    libraryControls.push_back("UIStaticText");
    libraryControls.push_back("UITextField");
    libraryControls.push_back("UISlider");
    libraryControls.push_back("UIList");
    libraryControls.push_back("UIListCell");
    libraryControls.push_back("UIScrollBar");
    libraryControls.push_back("UIScrollView");
    libraryControls.push_back("UISpinner");
    libraryControls.push_back("UISwitch");
    libraryControls.push_back("UIParticles");
    defaultCountrolsCount = libraryControls.size();
   
    PackageControlsNode *packageControls = node->GetPackageControlsNode();
    for (int j = 0; j < packageControls->GetCount(); j++)
    {
        ControlNode *node = static_cast<ControlNode*>(packageControls->Get(j));
        libraryControls.push_back(node->GetName());
    }

    for (int i = 0; i < node->GetImportedPackagesNode()->GetCount(); i++)
    {
        ImportedPackagesNode *importedPackage = (ImportedPackagesNode*) node->GetImportedPackagesNode()->Get(i);
        for (int j = 0; j < importedPackage->GetCount(); j++)
        {
            libraryControls.push_back(importedPackage->GetName() + "/" + importedPackage->Get(j)->GetName());
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
        return libraryControls.size();
    }
    return 0;
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return StringToQString(libraryControls[index.row()]);
    }
    else if (role == Qt::DecorationRole)
    {
        QString className = StringToQString(libraryControls[index.internalId()]);
        return index.internalId() < defaultCountrolsCount ? QIcon(IconHelper::GetIconPathForClassName(className))
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
            data->setText(StringToQString(libraryControls[index.row()]));
            return data;
        }
    }
    return nullptr;
}
