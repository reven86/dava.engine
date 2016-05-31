#include "Preferences/PreferencesRootProperty.h"
#include "Preferences/PreferencesSectionProperty.h"
#include "Preferences/PreferencesStorage.h"
#include <QString>
#include <QStringList>

using namespace DAVA;

namespace PreferencesRootProperty_local
{
struct PreferencesLocation
{
    DAVA::Map<DAVA::String, PreferencesLocation> locations; //locations inside current location
    DAVA::Vector<const DAVA::InspMember*> members; //leaf insp member
};

void CreatePropertiesTree(const PreferencesLocation& preferencesLocation, PreferencesSectionProperty* property)
{
    for (const auto& locationAndName : preferencesLocation.locations)
    {
        ScopedPtr<PreferencesSectionProperty> section(new PreferencesSectionProperty(locationAndName.first));
        property->AddSection(section);
        CreatePropertiesTree(locationAndName.second, section);
    }
    for (const DAVA::InspMember* member : preferencesLocation.members)
    {
        DAVA::ScopedPtr<PreferencesIntrospectionProperty> memberProperty(new PreferencesIntrospectionProperty(member));
        property->AddProperty(memberProperty);
    }
}
}

PreferencesRootProperty::PreferencesRootProperty()
    : PreferencesSectionProperty("PREFERENCES_ROOT_PROPERTY")
{
    const PreferencesStorage::RegisteredIntrospection& registeredInsp = PreferencesStorage::Instance()->GetRegisteredInsp();
    PreferencesRootProperty_local::PreferencesLocation rootPreferencesLocation;

    for (const InspInfo* info : registeredInsp)
    {
        //insp info do not store necessary information for this proprety
        for (int i = 0, count = info->MembersCount(); i < count; ++i)
        {
            const InspMember* member = info->Member(i);
            QString description(member->Desc().text);
            QStringList folders = description.split('/', QString::SkipEmptyParts);
            PreferencesRootProperty_local::PreferencesLocation* currentLocation = &rootPreferencesLocation;
            for (int index = 0, foldersCount = folders.size() - 1; index < foldersCount; ++index)
            {
                DAVA::String folder = folders.at(index).toStdString();
                currentLocation = &currentLocation->locations[folder];
            }
            currentLocation->members.push_back(member);
        }
    }

    PreferencesRootProperty_local::CreatePropertiesTree(rootPreferencesLocation, this);
}
