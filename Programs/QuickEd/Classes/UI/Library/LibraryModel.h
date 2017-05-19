#ifndef __UI_EDITOR_LIBRARY_MODEL_H__
#define __UI_EDITOR_LIBRARY_MODEL_H__

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include <QStandardItemModel>

class PackageNode;
class PackageBaseNode;
class AbstractProperty;
class ControlNode;
class ControlsContainerNode;
class ImportedPackagesNode;

namespace DAVA
{
namespace TArc
{
class UI;
}
}

class LibraryModel : public QStandardItemModel, PackageListener
{
    Q_OBJECT
    enum
    {
        POINTER_DATA = Qt::UserRole + 1,
        PROTOTYPE
    };

public:
    LibraryModel(QObject* parent = nullptr);
    ~LibraryModel() override;

    void SetUI(DAVA::TArc::UI* ui);
    void SetProjectLibraries(const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>& prototypes, const DAVA::Vector<DAVA::FilePath>& libraryPackages);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    void SetPackageNode(PackageNode* package);

    QModelIndex GetDefaultControlsModelIndex() const;

private:
    QVariant data(const QModelIndex& index, int role) const override;

    QModelIndex indexByNode(const void* node, const QStandardItem* item) const;
    void AddControl(ControlNode* node, QStandardItem* rootItem, bool makePrototype);
    void AddPackageControls(PackageControlsNode* packageControls, QStandardItem* rootItem, bool makePrototype);
    QStandardItem* CreatePackageControlsItem(PackageNode* package, bool makePrototype);

    //Package Signals
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;
    DAVA::RefPtr<PackageNode> package;

    DAVA::Vector<PackageNode*> libraryPackages;
    DAVA::Vector<QStandardItem*> libraryRootItems;

    QStandardItem* defaultControlsRootItem = nullptr;
    QStandardItem* controlsRootItem = nullptr;
    QStandardItem* importedPackageRootItem = nullptr;

    DAVA::Vector<ControlNode*> defaultControls;
    DAVA::Vector<DAVA::FilePath> libraryPackagePaths;
    DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>> prototypes;

    DAVA::TArc::UI* ui = nullptr;
};

#endif // __UI_EDITOR_LIBRARY_MODEL_H__
