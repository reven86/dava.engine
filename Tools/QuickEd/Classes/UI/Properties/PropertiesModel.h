#ifndef __QUICKED_PROPERTIES_MODEL_H__
#define __QUICKED_PROPERTIES_MODEL_H__

#include <QAbstractItemModel>

#include "FileSystem/VariantType.h"
#include "Model/ControlProperties/PropertyListener.h"

namespace DAVA {
    class InspInfo;
    enum ItemDataRole
    {
        ResetRole = Qt::UserRole +1,
    };
}

class BaseProperty;
class ControlNode;
class QtModelPackageCommandExecutor;
class ComponentPropertiesSection;

class PropertiesModel : public QAbstractItemModel, private PropertyListener
{
    Q_OBJECT
    
public:
    PropertiesModel(ControlNode *controlNode, QtModelPackageCommandExecutor *_commandExecutor, QObject *parent = nullptr);
    virtual ~PropertiesModel();
    
    ControlNode *GetControlNode() const {return controlNode; }
    
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    void AddComponentSection(ComponentPropertiesSection *section);
    void RemoveComponentSection(ComponentPropertiesSection *section);

private: // PropertyListener
    virtual void PropertyChanged(BaseProperty *property) override;
    
private:
    QModelIndex indexByProperty(BaseProperty *property, int column = 0);
    QVariant makeQVariant(const BaseProperty *property) const;
    void initVariantType(DAVA::VariantType &var, const QVariant &val) const;
    
private:
    ControlNode *controlNode;
    QtModelPackageCommandExecutor *commandExecutor;
};

#endif // __QUICKED_PROPERTIES_MODEL_H__
