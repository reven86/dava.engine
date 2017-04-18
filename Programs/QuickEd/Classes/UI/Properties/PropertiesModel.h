#pragma once

#include "Model/ControlProperties/PropertyListener.h"

#include <UI/Styles/UIStyleSheetSystem.h>
#include <QtTools/Updaters/ContinuousUpdater.h>

#include <Base/RefPtr.h>
#include <FileSystem/VariantType.h>

#include <QAbstractItemModel>
#include <QSet>

namespace DAVA
{
class InspInfo;
class Any;
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

class AbstractProperty;
class PackageBaseNode;
class ControlNode;
class StyleSheetNode;
class ComponentPropertiesSection;

class PropertiesModel : public QAbstractItemModel, private PropertyListener, private DAVA::UIStyleSheetSystemListener
{
    Q_OBJECT

public:
    enum
    {
        ResetRole = Qt::UserRole + 1
    };

    PropertiesModel(QObject* parent = nullptr);
    ~PropertiesModel() override;

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);

    void Reset(PackageBaseNode* node_);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    const AbstractProperty* GetRootProperty() const;
    QModelIndex indexByProperty(const AbstractProperty* property, int column = 0);

signals:
    void ComponentAdded(const QModelIndex& index);

protected:
    void UpdateAllChangedProperties();
    void ResetInternal();
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

    // UIStyleSheetSystemListener
    void OnStylePropertyChanged(DAVA::UIControl* control, DAVA::UIComponent* component, DAVA::uint32 propertyIndex) override;

    virtual void ChangeProperty(AbstractProperty* property, const DAVA::Any& value);
    virtual void ResetProperty(AbstractProperty* property);

    QString makeQVariant(const AbstractProperty* property) const;
    void initAny(DAVA::Any& var, const QVariant& val) const;
    void CleanUp();

    void OnPackageChanged(const DAVA::Any& package);
    void BindFields();

    PackageBaseNode* nodeToReset = nullptr;
    ControlNode* controlNode = nullptr;
    StyleSheetNode* styleSheet = nullptr;
    AbstractProperty* rootProperty = nullptr;
    DAVA::Set<DAVA::RefPtr<AbstractProperty>> changedProperties;
    ContinuousUpdater propertiesUpdater;
    ContinuousUpdater nodeUpdater;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
};
