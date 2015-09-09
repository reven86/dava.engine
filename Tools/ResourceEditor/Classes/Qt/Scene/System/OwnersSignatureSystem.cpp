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


#include "OwnersSignatureSystem.h"
#include "Classes/StringConstants.h"
#include "Qt/Settings/SettingsManager.h"
#include "Scene/SceneSignals.h"

const DAVA::int32 OwnersSignatureSystem::validIDs[] =
{
	CMDID_ENTITY_ADD,
	CMDID_ENTITY_CHANGE_PARENT,
	CMDID_TRANSFORM
};

OwnersSignatureSystem::OwnersSignatureSystem(DAVA::Scene* scene):SceneSystem(scene)
{}

OwnersSignatureSystem::~OwnersSignatureSystem()
{}

void OwnersSignatureSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(IsCommandIdValid(command->GetId()))
	{
		KeyedArchive* properties = GetCustomPropertiesArchieve(command->GetEntity());
		if(NULL != properties)
		{
			UpdateEntityOwner(properties);
		}
	}
}

bool OwnersSignatureSystem::IsCommandIdValid(int _id)
{
	for (size_t i = 0; i < COUNT_OF(validIDs); ++i)
	{
		if(validIDs[i] == _id)
		{
			return true;
		}
	}
	return false;
}

DAVA::String OwnersSignatureSystem::GetCurrentTime()
{
	time_t now = time(0);
    tm* utcTime = localtime(&now);
	
	DAVA::String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",
							   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
							   utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);
	
	return timeString;
}

void OwnersSignatureSystem::UpdateEntityOwner(DAVA::KeyedArchive *customProperties)
{
	customProperties->SetString(ResourceEditor::SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, SettingsManager::GetValue(Settings::General_DesinerName).AsString());
	customProperties->SetString(ResourceEditor::SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, GetCurrentTime());
}
