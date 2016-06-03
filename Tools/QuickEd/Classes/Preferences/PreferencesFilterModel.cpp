#include "Preferences/PreferencesFilterModel.h"
#include "Preferences/PreferencesRootProperty.h"
#include "Model/ControlProperties/SubValueProperty.h"

namespace PreferencesModel_local
{
bool IsPropertyVisible(AbstractProperty* property)
{
    PreferencesIntrospectionProperty* inspProp = dynamic_cast<PreferencesIntrospectionProperty*>(property);
    if (nullptr != inspProp)
    {
        return (inspProp->GetMember()->Flags() & DAVA::I_VIEW) != 0;
    }

    PreferencesSectionProperty* sectionProp = dynamic_cast<PreferencesSectionProperty*>(property);
    if (nullptr != sectionProp)
    {
        bool isVisible = false;
        for (int i = sectionProp->GetCount() - 1; i >= 0 && !isVisible; --i)
        {
            isVisible |= IsPropertyVisible(sectionProp->GetProperty(i));
        }
        return isVisible;
    }
    else if (nullptr != dynamic_cast<SubValueProperty*>(property))
    {
        return false;
    }
    else
    {
        DVASSERT(false);
        return false;
    }
}
}

PreferencesFilterModel::PreferencesFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool PreferencesFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);
    AbstractProperty* property = static_cast<AbstractProperty*>(source.internalPointer());
    SubValueProperty* subValue = dynamic_cast<SubValueProperty*>(property);
    if (subValue != nullptr)
    {
        return true;
    }
    return PreferencesModel_local::IsPropertyVisible(property);
}