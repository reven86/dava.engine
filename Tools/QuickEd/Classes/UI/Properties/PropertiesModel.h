#ifndef __QUICKED_PROPERTIES_MODEL_H__
#define __QUICKED_PROPERTIES_MODEL_H__

#include "Base/RefPtr.h"
#include "FileSystem/VariantType.h"

#include "Model/ControlProperties/PropertyListener.h"

#include <QAbstractItemModel>
#include <QSet>

namespace DAVA
{
class InspInfo;
}

class AbstractProperty;
class PackageBaseNode;
class ControlNode;
class StyleSheetNode;
class QtModelPackageCommandExecutor;
class ComponentPropertiesSection;
class ContinuousUpdater;

class PropertiesModel : public QAbstractItemModel, private PropertyListener
{
    Q_OBJECT

public:
    enum
    {
        ResetRole = Qt::UserRole + 1
    };
    PropertiesModel(QObject* parent = nullptr);
    virtual ~PropertiesModel();
    void Reset(PackageBaseNode* node_, QtModelPackageCommandExecutor* commandExecutor_);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

protected:
    void UpdateAllChangedProperties();

    // PropertyListener
    void PropertyChanged(AbstractProperty* property) override;
    void UpdateProperty(AbstractProperty* property);

    void ComponentPropertiesWillBeAdded(RootProperty* root, ComponentPropertiesSection* section, int index) override;
    void ComponentPropertiesWasAdded(RootProperty* root, ComponentPropertiesSection* section, int index) override;

    void ComponentPropertiesWillBeRemoved(RootProperty* root, ComponentPropertiesSection* section, int index) override;
    void ComponentPropertiesWasRemoved(RootProperty* root, ComponentPropertiesSection* section, int index) override;

    void StylePropertyWillBeAdded(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index) override;
    void StylePropertyWasAdded(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index) override;

    void StylePropertyWillBeRemoved(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index) override;
    void StylePropertyWasRemoved(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index) override;

    void StyleSelectorWillBeAdded(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index) override;
    void StyleSelectorWasAdded(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index) override;

    void StyleSelectorWillBeRemoved(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index) override;
    void StyleSelectorWasRemoved(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index) override;

    virtual void ChangeProperty(AbstractProperty* property, const DAVA::VariantType& value);
    virtual void ResetProperty(AbstractProperty* property);

    QModelIndex indexByProperty(AbstractProperty* property, int column = 0);
    QString makeQVariant(const AbstractProperty* property) const;
    void initVariantType(DAVA::VariantType& var, const QVariant& val) const;
    void CleanUp();

protected:
    ControlNode* controlNode = nullptr;
    StyleSheetNode* styleSheet = nullptr;
    AbstractProperty* rootProperty = nullptr;
    QtModelPackageCommandExecutor* commandExecutor = nullptr;
    DAVA::Set<DAVA::RefPtr<AbstractProperty>> changedProperties;
    ContinuousUpdater* continuousUpdater = nullptr;
};

#endif // __QUICKED_PROPERTIES_MODEL_H__
