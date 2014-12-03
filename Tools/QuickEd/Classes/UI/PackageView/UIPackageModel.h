#ifndef __UI_EDITOR_UI_PACKAGE_MODEL_H__
#define __UI_EDITOR_UI_PACKAGE_MODEL_H__

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include "DAVAEngine.h"

class PackageDocument;
class PackageNode;
class ControlNode;
class PackageBaseNode;

class UIPackageModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    UIPackageModel(PackageDocument *document);
    virtual ~UIPackageModel();
    
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
    void MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void RemoveItem(const QModelIndex &srcItem);

    void InsertNode(ControlNode *node, const QModelIndex &parent, int row);
    void RemoveNode(ControlNode *node);
private:
    PackageNode *root;
    PackageDocument *document;
};

#endif // __UI_EDITOR_UI_PACKAGE_MODEL_H__
