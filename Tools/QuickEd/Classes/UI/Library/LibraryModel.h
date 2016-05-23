#ifndef __UI_EDITOR_LIBRARY_MODEL_H__
#define __UI_EDITOR_LIBRARY_MODEL_H__

#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include <QStandardItemModel>

class PackageNode;
class PackageBaseNode;
class AbstractProperty;
class ControlNode;
class ControlsContainerNode;
class ImportedPackagesNode;

class LibraryModel : public QStandardItemModel, PackageListener
{
    Q_OBJECT
    enum
    {
        POINTER_DATA = Qt::UserRole + 1,
        INNER_NAME_DATA
    };

public:
    LibraryModel(QObject* parent = nullptr);
    ~LibraryModel() override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    void SetPackageNode(PackageNode* package);

private:
    QVariant data(const QModelIndex& index, int role) const override;

    QModelIndex indexByNode(const void* node, const QStandardItem* item) const;
    void BuildModel();
    void AddControl(ControlNode* node);
    void AddImportedControl(PackageNode* node);
    void CreateControlsRootItem();
    void CreateImportPackagesRootItem();

    //Package Signals
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;
    PackageNode* package = nullptr;
    QStandardItem *defaultControlsRootItem, *controlsRootItem, *importedPackageRootItem;
    DAVA::Vector<ControlNode*> defaultControls;
};

#endif // __UI_EDITOR_LIBRARY_MODEL_H__
