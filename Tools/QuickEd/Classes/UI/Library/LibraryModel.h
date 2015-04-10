#ifndef __UI_EDITOR_LIBRARY_MODEL_H__
#define __UI_EDITOR_LIBRARY_MODEL_H__

#include <QAbstractItemModel>
#include "Base/BaseTypes.h"

class PackageNode;

class LibraryModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    LibraryModel(PackageNode *node, QObject *parent = NULL);
    virtual ~LibraryModel();
    
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;

private:
    DAVA::Vector<DAVA::String> defaultControls;
};

#endif // __UI_EDITOR_LIBRARY_MODEL_H__
