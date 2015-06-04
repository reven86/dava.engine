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


#include "ParticlesEditorNodeNameHelper.h"
#include "../StringConstants.h"

namespace DAVA {

String  ParticlesEditorNodeNameHelper::GetBaseName(const String& name)
{
	String baseName = name;
	String numberName;
	// Get numbers string at the end of entity name
	const char* cName = baseName.c_str();
	for (int i = baseName.length() - 1; i >= 0; --i)
	{
		char a = cName[i];
		if (a >= '0' && a <= '9')
			numberName = a + numberName;
		else
			break;
	}

	baseName = baseName.substr(0, name.length() - numberName.length());
	
	return baseName;
}

String ParticlesEditorNodeNameHelper::GetNewNodeName(const String &name, Entity *parentNode)
{
	// Don't change name for "root" nodes
    if(String::npos !=  name.find(ResourceEditor::EDITOR_BASE))
		return name;
		
	// Keep unique node name
	if (!IsNodeNameExist(name, parentNode))
	{
		return name;
	}

	// Increase node counter until we get unique name
	int i = 0;
	while (true)
	{
		String newName = String(Format("%s%i", name.c_str(), ++i));
		
		if (!IsNodeNameExist(newName, parentNode))
			return newName;
	}
}
			
bool ParticlesEditorNodeNameHelper::IsNodeNameExist(const String &name, Entity *parentNode)
{
	if (!parentNode)
		return false;
		
	int32 childrenCount = parentNode->GetChildrenCount();
    FastName fastName = FastName(name);

	for (int32 i = 0; i < childrenCount; ++i)
	{
		Entity * childNode = parentNode->GetChild(i);
		
		if (!childNode)
			continue;
		
		if (childNode->GetName() == fastName)
		{
			return true;
		}
	}
	
	return false;
}

String ParticlesEditorNodeNameHelper::GetNewLayerName(const String& name, ParticleEmitter *emitter)
{
	// Keep unique node name
	if (!IsLayerNameExist(name, emitter))
	{
		return name;
	}	

	int i = 0;
	while (true)
	{
		String newName = String(Format("%s%i", ResourceEditor::LAYER_NODE_NAME.c_str(), ++i));
		
		if (!IsLayerNameExist(newName, emitter))
			return newName;
	}
}

bool ParticlesEditorNodeNameHelper::IsLayerNameExist(const String &name, ParticleEmitter *emitter)
{
	if (!emitter)
		return false;	
	
	for (std::vector<ParticleLayer*>::const_iterator t = emitter->layers.begin(); t != emitter->layers.end(); ++t)
	{
        ParticleLayer *layer = *t;
		
		if (!layer)
			continue;
		
		if (layer->layerName == name)
		{
			return true;
		}
	}
	
	return false;
}

};