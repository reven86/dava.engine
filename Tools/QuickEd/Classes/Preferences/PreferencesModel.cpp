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


#include "Preferences/PreferencesModel.h"
#include "Preferences/PreferencesRootProperty.h"

PreferencesFilterModel::PreferencesFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool PreferencesFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);
    AbstractProperty* prop = static_cast<AbstractProperty*>(source.internalPointer());
    PreferencesIntrospectionProperty* inspProp = dynamic_cast<PreferencesIntrospectionProperty*>(prop);
    if (nullptr == inspProp)
    {
        SectionProperty<PreferencesIntrospectionProperty>* sectionProp = dynamic_cast<SectionProperty<PreferencesIntrospectionProperty>*>(prop);
        if (nullptr != sectionProp)
        {
            bool isVisible = false;
            for (int i = 0, count = sectionProp->GetCount(); i < count && !isVisible; ++i)
            {
                isVisible |= ((sectionProp->GetProperty(i)->GetMember()->Flags() & DAVA::I_VIEW) != 0);
            }
            return isVisible;
        }
        else
        {
            return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
        }
    }
    return (inspProp->GetMember()->Flags() & DAVA::I_VIEW) != 0;
}

PreferencesModel::PreferencesModel(QObject* parent)
    : PropertiesModel(parent)
{
    rootProperty = new PreferencesRootProperty();
}

void PreferencesModel::ChangeProperty(AbstractProperty* property, const DAVA::VariantType& value)
{
    property->SetValue(value);
}

void PreferencesModel::ResetProperty(AbstractProperty* property)
{
    property->ResetValue();
}
