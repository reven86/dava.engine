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


#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/DataNode.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Material/NMaterial.h"

#include "Utils/StringFormat.h"

#include "Render/Material/NMaterialNames.h"
#include "Render/Texture.h"

namespace DAVA
{
    SerializationContext::SerializationContext() :
        globalMaterialKey(0)
	{ }

	SerializationContext::~SerializationContext()
	{
		for(Map<uint64, DataNode*>::iterator it = dataBlocks.begin();
			it != dataBlocks.end();
			++it)
		{
			SafeRelease(it->second);
		}
		
		for(Map<uint64, NMaterial*>::iterator it = importedMaterials.begin();
			it != importedMaterials.end();
			++it)
		{
			SafeRelease(it->second);
		}

		DVASSERT(materialBindings.size() == 0 && "Serialization context destroyed without resolving material bindings!");
		materialBindings.clear();
	}
	
	void SerializationContext::ResolveMaterialBindings()
	{
		size_t instanceCount = materialBindings.size();
		for(size_t i = 0; i < instanceCount; ++i)
		{
			MaterialBinding& binding = materialBindings[i];
            uint64 parentKey = (binding.parentKey) ? binding.parentKey : globalMaterialKey;
            if (!parentKey)
                continue;

            NMaterial* parentMat = static_cast<NMaterial*>(GetDataBlock(parentKey));
			
			DVASSERT(parentMat);
			if(parentMat != binding.childMaterial) //global material case
			{
				binding.childMaterial->SetParent(parentMat);
			}
		}
		
		materialBindings.clear();
	}
			
Texture* SerializationContext::PrepareTexture(rhi::TextureType textureTypeHint, Texture* tx)
{

	if(tx)
	{
		if(tx->isPink)
		{
			tx->Retain();
		}

		return tx;
	}

	return Texture::CreatePink(textureTypeHint);

}


void SerializationContext::AddLoadedPolygonGroup(PolygonGroup *group, uint32 dataFilePos)
{
    DVASSERT(loadedPolygonGroups.find(group)==loadedPolygonGroups.end());
    PolygonGroupLoadInfo loadInfo;
    loadInfo.filePos = dataFilePos;
    loadedPolygonGroups[group] = loadInfo;
}
void SerializationContext::AddRequestedPolygonGroupFormat(PolygonGroup *group, int32 format)
{
    auto foundGroup = loadedPolygonGroups.find(group);
    DVASSERT(foundGroup!=loadedPolygonGroups.end());
    foundGroup->second.requestedFormat |= format;
    foundGroup->second.onScene = true;
}

void SerializationContext::LoadPolygonGroupData(File *file)
{
    bool cutUnusedStreams = QualitySettingsSystem::Instance()->GetAllowCutUnusedVertexStreams();
    for (Map<PolygonGroup*, PolygonGroupLoadInfo>::iterator it = loadedPolygonGroups.begin(), e = loadedPolygonGroups.end(); it!=e; ++it)
    {
        if (it->second.onScene || !cutUnusedStreams)
        {
            file->Seek(it->second.filePos, File::SEEK_FROM_START);
            KeyedArchive * archive = new KeyedArchive();
            archive->Load(file);
            it->first->LoadPolygonData(archive, this, it->second.requestedFormat, cutUnusedStreams);
            SafeRelease(archive);
        }
    }
}
}