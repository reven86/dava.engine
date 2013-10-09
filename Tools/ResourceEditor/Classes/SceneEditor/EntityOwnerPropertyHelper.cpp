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



#include "EntityOwnerPropertyHelper.h"
#include "EditorSettings.h"

namespace DAVA {

const char* EntityOwnerPropertyHelper::SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME = "editor.designerName";
const char* EntityOwnerPropertyHelper::SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME = "editor.modificationData";

void EntityOwnerPropertyHelper::UpdateEntityOwner(KeyedArchive *customProperties)
{
	SetDesignerName(customProperties, EditorSettings::Instance()->GetDesignerName());
	UpdateModificationTime(customProperties);
}

void EntityOwnerPropertyHelper::SetDesignerName(KeyedArchive *customProperties, const String & name)
{
	customProperties->SetString(SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, name);
}

String EntityOwnerPropertyHelper::GetDesignerName(KeyedArchive *customProperties)
{
	return customProperties->GetString(SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, "nobody");
}

void EntityOwnerPropertyHelper::UpdateModificationTime(KeyedArchive *customProperties)
{
	time_t now = time(0);
    tm* utcTime = localtime(&now);
	
    String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",
							   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
							   utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);

	customProperties->SetString(SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, timeString);
}

String EntityOwnerPropertyHelper::GetModificationTime(KeyedArchive *customProperties)
{
	return customProperties->GetString(SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, "unknown");
}

};