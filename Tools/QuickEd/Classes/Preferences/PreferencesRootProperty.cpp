/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
