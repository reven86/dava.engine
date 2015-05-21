#ifndef __UI_EDITOR_LIBRARY_MODEL_H__
#define __UI_EDITOR_LIBRARY_MODEL_H__

#include <QAbstractItemModel>
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageListener.h"

class PackageNode;
class PackageBaseNode;

class LibraryModel : public QAbstractItemModel, private PackageListener
{
    Q_OBJECT
    
public:
    LibraryModel(PackageNode *root, QObject *parent = nullptr);
    virtual ~LibraryModel();
    
    QModelIndex indexByNode(PackageBaseNode *node) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

private:
    bool IsDefault(int row) const;
    PackageNode *root;
    QList<QString> defaultControls;

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
