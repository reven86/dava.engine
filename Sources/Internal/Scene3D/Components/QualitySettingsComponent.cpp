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



#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA 
{
    

QualitySettingsComponent::QualitySettingsComponent()
    : Component()   
    , filterByType(true)
{
}

QualitySettingsComponent::~QualitySettingsComponent()
{
}
   
    
Component * QualitySettingsComponent::Clone(Entity * toEntity)
{
    QualitySettingsComponent * component = new QualitySettingsComponent();
	component->SetEntity(toEntity);
    
    component->filterByType = filterByType;
    component->modelType = modelType;    
    component->requiredGroup = requiredGroup;
    component->requiredQuality = requiredQuality;

    return component;
}

void QualitySettingsComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);
    if (modelType.IsValid())
        archive->SetString("modelType", modelType.c_str());
    if (requiredGroup.IsValid())
        archive->SetString("requiredGroup", requiredGroup.c_str());
    if (requiredQuality.IsValid())
        archive->SetString("requiredQuality", requiredQuality.c_str());    
    archive->SetBool("filterByType", filterByType);
}

void QualitySettingsComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
    if (archive->IsKeyExists("modelType"))
        modelType = FastName(archive->GetString("modelType"));
    if (archive->IsKeyExists("requiredGroup"))
        requiredGroup = FastName(archive->GetString("requiredGroup"));
    if (archive->IsKeyExists("requiredQuality"))
        requiredQuality = FastName(archive->GetString("requiredQuality"));
    filterByType = archive->GetBool("filterByType", filterByType);

	Component::Deserialize(archive, serializationContext);
}

void QualitySettingsComponent::SetFilterByType(bool filter)
{
    filterByType = filter;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}
bool QualitySettingsComponent::GetFilterByType() const
{
    return filterByType;
}

void QualitySettingsComponent::SetModelType(const FastName & type)
{
    modelType = type;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}

const FastName & QualitySettingsComponent::GetModelType() const
{
    return modelType;
}

void QualitySettingsComponent::SetRequiredGroup(const FastName & group)
{
    requiredGroup = group;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}
const FastName & QualitySettingsComponent::GetRequiredGroup() const
{
    return requiredGroup;
}

void QualitySettingsComponent::SetRequiredQuality(const FastName & quality)
{
    requiredQuality = quality;
    QualitySettingsSystem::Instance()->UpdateEntityVisibility(GetEntity());
}
const FastName & QualitySettingsComponent::GetRequiredQuality() const
{
    return requiredQuality;
}

};
