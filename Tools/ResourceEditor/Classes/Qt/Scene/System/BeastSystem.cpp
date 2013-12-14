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


#include "BeastSystem.h"


BeastSystem::BeastSystem(Scene* scene):SceneSystem(scene)
{}

BeastSystem::~BeastSystem(){}

void BeastSystem::AddEntity(Entity * entity)
{
    SetDefaultPropertyValues(entity);
}

void BeastSystem::SetDefaultPropertyValues(Entity * entity)
{
	DAVA::KeyedArchive* propertyList = entity->GetCustomProperties();

	if(GetLight(entity))
	{
		SetBool(propertyList, "editor.staticlight.enable", true);

		SetFloat(propertyList, "editor.intensity", 1.f);

		SetFloat(propertyList, "editor.staticlight.shadowangle", 0.f);
		SetFloat(propertyList, "editor.staticlight.shadowradius", 0.f);
		SetInt32(propertyList, "editor.staticlight.shadowsamples", 1);
		SetFloat(propertyList, "editor.staticlight.falloffcutoff", 1000.f);
		SetFloat(propertyList, "editor.staticlight.falloffexponent", 1.f);
	}
}

void BeastSystem::SetBool(KeyedArchive* propertyList, const String & key, bool value)
{
	if(!propertyList->IsKeyExists(key))
	{
		propertyList->SetBool(key, value);
	}
}

void BeastSystem::SetFloat(KeyedArchive* propertyList, const String & key, float32 value)
{
	if(!propertyList->IsKeyExists(key))
	{
		propertyList->SetFloat(key, value);
	}
}

void BeastSystem::SetInt32( KeyedArchive* propertyList, const String & key, int32 value )
{
	if(!propertyList->IsKeyExists(key))
	{
		propertyList->SetInt32(key, value);
	}
}



