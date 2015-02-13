#ifndef __QUICKED_PACKAGE_MODEL_H__
#define __QUICKED_PACKAGE_MODEL_H__

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include "DAVAEngine.h"

class Document;
class PackageNode;
class ControlNode;
class PackageBaseNode;
class PackageControlsNode;

class PackageModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    PackageModel(Document *document);
    virtual ~PackageModel();
    
    virtual QModelIndex indexByNode(PackageBaseNode *node) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual Qt::DropActions supportedDropActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    
    void InsertItem(const QString &name, int dstRow, const QModelIndex &dstParent);
    void InsertItem(ControlNode *node, int dstRow, const QModelIndex &dstParent);
    void InsertImportedPackage(PackageControlsNode *node, int dstRow, const QModelIndex &dstParent);
    void MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void RemoveItem(const QModelIndex &srcItem);

    void InsertNode(ControlNode *node, const QModelIndex &parent, int row);
    void RemoveNode(PackageBaseNode *node);
    void RemoveControlNode(ControlNode *node);
    void RemovePackageControlsNode(PackageControlsNode *node);
private:
    PackageNode *root;
    Document *document;
};

#endif // __QUICKED_PACKAGE_MODEL_H__
