#ifndef __GENERICTREEMODEL_H__
#define __GENERICTREEMODEL_H__

#include <QAbstractItemModel>

class GenericTreeNode;
class GenericTreeModel : public QAbstractItemModel
{
public:
    GenericTreeModel(int columnCount, QObject* parent = nullptr);
    virtual ~GenericTreeModel();

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

protected:
    void SetRootNode(GenericTreeNode* root);

private:
    GenericTreeNode* rootNode;
    int ncolumns;
};

#endif  // __GENERICTREEMODEL_H__
