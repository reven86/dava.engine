//
//  PropertiesTreeModel.h
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#ifndef __UI_EDITOR_PROPERTIES_TREE_MODEL_H__
#define __UI_EDITOR_PROPERTIES_TREE_MODEL_H__

#include <QAbstractItemModel>
#include "DAVAEngine.h"

namespace DAVA {
    class InspInfo;
    enum ItemDataRole
    {
        ResetRole = Qt::UserRole +1,
    };
}

class BaseProperty;
class PropertiesViewContext;

class PropertiesTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    PropertiesTreeModel(BaseProperty *propertiesRoot, PropertiesViewContext *context, QObject *parent = NULL);
    virtual ~PropertiesTreeModel();
    
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

private:
    QVariant makeQVariant(const BaseProperty *property) const;
    void initVariantType(DAVA::VariantType &var, const QVariant &val) const;
    
private:
    BaseProperty *root;
    PropertiesViewContext *propertiesViewContext;
};

#endif // __UI_EDITOR_PROPERTIES_TREE_MODEL_H__
