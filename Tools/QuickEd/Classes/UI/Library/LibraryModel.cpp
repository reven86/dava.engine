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
    : QStandardItemModel(parent)
    , root(SafeRetain(_root))
{
    root->AddListener(this);
    defaultControls
        << "UIControl"
        << "UIButton"
        << "UIStaticText"
        << "UITextField"
        << "UISlider"
        << "UIList"
        << "UIListCell"
        << "UIScrollBar"
        << "UIScrollView"
        << "UISpinner"
        << "UISwitch"
        << "UIParticles";
    BuildModel();
}

LibraryModel::~LibraryModel()
{
    root->RemoveListener(this);
    SafeRelease(root);
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

QModelIndex LibraryModel::indexByNode(const void *node, const QStandardItem *item) const
{
    void* ptr = item->data().value<void*>();
    if (ptr == node)
    {
        return item->index();
    }
    for (auto row = item->rowCount() - 1; row >= 0; --row)
    {
        QModelIndex index = indexByNode(node, item->child(row));
        if (index.isValid())
        {
            return index;
        }
    }
    return QModelIndex();
}

void LibraryModel::BuildModel()
{
    clear();
    auto defaultControlsRoot = new QStandardItem(tr("Default controls"));
    defaultControlsRoot->setBackground(QBrush(Qt::lightGray));
    invisibleRootItem()->appendRow(defaultControlsRoot);
    for (const auto &defaultControl : defaultControls)
    {
        auto item = new QStandardItem(
            QIcon(IconHelper::GetIconPathForClassName(defaultControl)),
            defaultControl
            );
        defaultControlsRoot->appendRow(item);
    }
    const auto packageControls = root->GetPackageControlsNode();
    if (packageControls->GetCount())
    {
        controlsRootItem = new QStandardItem(tr("Package controls"));
        controlsRootItem->setBackground(QBrush(Qt::lightGray));
        invisibleRootItem()->appendRow(controlsRootItem);
        for (int i = 0; i < packageControls->GetCount(); i++)
        {
            const auto node = packageControls->Get(i);
            auto item = new QStandardItem(
                QIcon(IconHelper::GetCustomIconPath()),
                QString::fromStdString(node->GetName())
                );
            item->setData(QVariant::fromValue(static_cast<void*>(node)));
            controlsRootItem->appendRow(item);
        }
    }
    const auto importedControls = root->GetImportedPackagesNode();
    if (importedControls->GetCount())
    {
        importedPackageRootItem = new QStandardItem(tr("Importred controls"));
        importedPackageRootItem->setBackground(QBrush(Qt::lightGray));
        invisibleRootItem()->appendRow(importedPackageRootItem);
        for (int i = 0; i < importedControls->GetCount(); ++i)
        {
            const auto importedPackage = importedControls->Get(i);
            auto importedPackageItem = new QStandardItem(QString::fromStdString(importedPackage->GetName()));
            importedPackageItem->appendRow(importedPackageItem);
            for (int j = 0; j < importedPackage->GetCount(); ++j)
            {
                const auto node = importedPackage->Get(j);
                auto item = new QStandardItem(
                    QIcon(IconHelper::GetCustomIconPath()),
                    QString::fromStdString(node->GetName())
                    );
                item->setData(QVariant::fromValue(static_cast<void*>(node)));
                importedPackageItem->appendRow(item);
            }
        }
    }
}

void LibraryModel::ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property)
{
    QModelIndex index = indexByNode(node, invisibleRootItem());
    if (index.isValid())
    {
        emit dataChanged(index, index);
    }
}

void LibraryModel::ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int row)
{
}

void LibraryModel::ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int row)
{
    //TODO: check that the parent is "controls"
    const QModelIndex destIndex = indexByNode(node, controlsRootItem); //check that we already do not have this item 
    if (!destIndex.isValid())
    {
        auto item = new QStandardItem(
            QIcon(IconHelper::GetCustomIconPath()),
            QString::fromStdString(node->GetName())
            );
        item->setData(QVariant::fromValue(static_cast<void*>(node)));
        controlsRootItem->appendRow(item);
    }
}

void LibraryModel::ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from)
{
    QModelIndex index = indexByNode(node, controlsRootItem);
    if (index.isValid())
    {
        removeRow(index.row(), index.parent());
    }
}

void LibraryModel::ControlWasRemoved(ControlNode *node, ControlsContainerNode *from)
{

}

void LibraryModel::ImportedPackageWillBeAdded(PackageControlsNode *node, PackageNode *to, int index)
{
}

void LibraryModel::ImportedPackageWasAdded(PackageControlsNode *node, PackageNode *to, int index)
{
    //TODO: check that the parent is "imported controls"
    const QModelIndex destIndex = indexByNode(node, importedPackageRootItem()); //check that we already do not have this item 
    if (!destIndex.isValid())
    {
        //TODO: we must add control to subsection
        auto item = new QStandardItem(
            QIcon(IconHelper::GetCustomIconPath()),
            QString::fromStdString(node->GetName())
            );
        item->setData(QVariant::fromValue(static_cast<void*>(node)));
        controlsRootItem->appendRow(item);
    }
}

void LibraryModel::ImportedPackageWillBeRemoved(PackageControlsNode *node, PackageNode *from)
{
    QModelIndex parentIndex = indexByNode(node, importedPackageRootItem);
    if (parentIndex.isValid())
    {
        removeRow(parentIndex.row(), parentIndex.parent());
    }
}

void LibraryModel::ImportedPackageWasRemoved(PackageControlsNode *node, PackageNode *from)
{

}
