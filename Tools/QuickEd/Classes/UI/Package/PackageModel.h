#ifndef __QUICKED_PACKAGE_MODEL_H__
#define __QUICKED_PACKAGE_MODEL_H__

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include <QItemSelection>

#include "Model/PackageHierarchy/PackageListener.h"

class PackageNode;
class ControlNode;
class PackageBaseNode;
class PackageControlsNode;
class ControlsContainerNode;
class QtModelPackageCommandExecutor;

class PackageModel : public QAbstractItemModel, private PackageListener
{
    Q_OBJECT

public:
    PackageModel(PackageNode *root, QtModelPackageCommandExecutor *commandExecutor, QObject *parent = nullptr);
    virtual ~PackageModel();
    
    QModelIndex indexByNode(PackageBaseNode *node) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    
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
    
private:
    PackageNode *root;
    QtModelPackageCommandExecutor *commandExecutor;


};

#endif // __QUICKED_PACKAGE_MODEL_H__
