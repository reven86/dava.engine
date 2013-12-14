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


#include "SceneHelper.h"
#include "Deprecated/SceneValidator.h"
#include "CubemapEditor/MaterialHelper.h"

#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/MaterialSystem.h"

void SceneHelper::EnumerateTextures(DAVA::Entity *forNode, DAVA::TexturesMap &textureCollection)
{
	if(!forNode) return;

    DAVA::RenderObject *ro = GetRenderObject(forNode);
    if(ro)
    {
        DAVA::uint32 count = ro->GetRenderBatchCount();
        for(DAVA::uint32 b = 0; b < count; ++b)
        {
            DAVA::RenderBatch *renderBatch = ro->GetRenderBatch(b);
            DAVA::NMaterial *material = renderBatch->GetMaterial();
            
            CollectTextures(material, textureCollection);
        }
    }
    
    
    DAVA::int32 count = forNode->GetChildrenCount();
	for(DAVA::int32 c = 0; c < count; ++c)
	{
        EnumerateTextures(forNode->GetChild(c), textureCollection);
	}
}


void SceneHelper::EnumerateTextures(DAVA::Scene *forScene, DAVA::TexturesMap &textureCollection)
{
    if(!forScene) return;
    
    DAVA::Vector<DAVA::NMaterial *> materials;
    
    DAVA::RenderSystem *rSystem = forScene->GetRenderSystem();
    DAVA::MaterialSystem *matSystem = rSystem->GetMaterialSystem();
    
    matSystem->BuildMaterialList(NULL, materials);

    DAVA::uint32 count = (DAVA::uint32)materials.size();
    for(DAVA::uint32 m = 0; m < count; ++m)
    {
        CollectTextures(materials[m], textureCollection);
    }
}

void SceneHelper::CollectTextures(const DAVA::NMaterial *material, DAVA::TexturesMap &textures)
{
    DAVA::uint32 texCount = material->GetTextureCount();
    for(DAVA::uint32 t = 0; t < texCount; ++t)
    {
        DAVA::Texture *texture = material->GetTexture(t);
        if(texture)
        {
            const DAVA::FilePath & path = texture->relativePathname;
            if(!path.IsEmpty() && SceneValidator::Instance()->IsPathCorrectForProject(path))
            {
                textures[FILEPATH_MAP_KEY(path)] = texture;
            }
        }
    }
}
