#ifndef __UI_EDITOR_LIBRARY_MODEL_H__
#define __UI_EDITOR_LIBRARY_MODEL_H__

#include <QStandardItemModel>
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageListener.h"

class PackageNode;
class PackageBaseNode;

class LibraryModel : public QStandardItemModel, private PackageListener
{
    Q_OBJECT
    
public:
    LibraryModel(PackageNode *root, QObject *parent = nullptr);
    virtual ~LibraryModel();
   
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
private:
    QModelIndex indexByNode(const void *node, const QStandardItem *item) const;
    void BuildModel();

private:
    PackageNode *root;
    QStandardItem *controlsRootItem, *importedPackageRootItem;
    QStringList defaultControls;
private: // PackageListener
    void ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property) override;

    void ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int row) override;
    void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int row) override;

    void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) override;
    void ControlWasRemoved(ControlNode *node, ControlsContainerNode *from) override;

    void ImportedPackageWillBeAdded(PackageControlsNode *node, PackageNode *to, int index) override;
    void ImportedPackageWasAdded(PackageControlsNode *node, PackageNode *to, int index) override;

    void ImportedPackageWillBeRemoved(PackageControlsNode *node, PackageNode *from) override;
    void ImportedPackageWasRemoved(PackageControlsNode *node, PackageNode *from) override;

};

#endif // __UI_EDITOR_LIBRARY_MODEL_H__
