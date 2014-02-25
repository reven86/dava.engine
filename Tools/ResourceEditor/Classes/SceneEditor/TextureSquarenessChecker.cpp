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



#include "TextureSquarenessChecker.h"
#include "Qt/Scene/SceneHelper.h"
#include "SceneEditor/SceneEditorScreenMain.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Main/QtUtils.h"
#include "Main/mainwindow.h"

TextureSquarenessChecker::TextureSquarenessChecker()
{

}

TextureSquarenessChecker::~TextureSquarenessChecker()
{
}

void TextureSquarenessChecker::CheckSceneForTextureSquarenessAndResave(Scene *scene)
{
    if(scene) 
    {
        Vector<Material *> allMaterials;
        SceneHelper::EnumerateMaterials(scene, allMaterials);

        for(size_t i = 0; i < allMaterials.size(); i++)
        {
            Texture *texture = allMaterials[i]->GetTexture(Material::TEXTURE_DIFFUSE);
            if(texture)
            {
                if(!CheckTexureSquareness(texture))
                {
                    Vector2 oldTextureSize;
                    MakeTexureSquare(texture, oldTextureSize);
                    if(squaredTextures.count(texture) == 0)
                    {
                        float32 newSize = texture->GetWidth();
                        Vector2 newScale = oldTextureSize / newSize;

                        squaredTextures[texture] = newScale;
                    }
                }
            }
        }

        if(squaredTextures.size())
        {
            ValidateTextureCoordsOfNodeGeometry(scene);

            QtMainWindow::Instance()->GetCurrentScene()->Save();
        }
    }

    squaredTextures.clear();
    parsedPG.clear();
}

void TextureSquarenessChecker::ValidateTextureCoordsOfNodeGeometry(Entity *sceneNode)
{
    if(!sceneNode)
        return;

    int32 count = sceneNode->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        Entity *node = sceneNode->GetChild(i);

        ValidateTextureCoordsOfNodeGeometry(node);

        RenderObject *ro = GetRenderObject(node);
        if(!ro) continue;

        uint32 count = ro->GetRenderBatchCount();
        for(uint32 b = 0; b < count; ++b)
        {
            RenderBatch *rb = ro->GetRenderBatch(b);

            PolygonGroup *polygonGroup = rb->GetPolygonGroup();
            if(!polygonGroup) continue;

            Material *material = rb->GetMaterial();
            if(!material) continue;

            Texture * texture = material->GetTexture(Material::TEXTURE_DIFFUSE);
            if(squaredTextures.count(texture) && std::find(parsedPG.begin(), parsedPG.end(), polygonGroup) == parsedPG.end())
            {
                Vector2 newScale = squaredTextures[texture];

                int32 vertexCount = polygonGroup->GetVertexCount();
                DAVA::Vector2 texCoord;
                for(DAVA::int32 i = 0; i < vertexCount; ++i)
                {
                    polygonGroup->GetTexcoord(0, i, texCoord);
                    texCoord.x *= newScale.x;
                    texCoord.y *= newScale.y;
                    polygonGroup->SetTexcoord(0, i, texCoord);

                }

                polygonGroup->BuildBuffers();

                parsedPG.push_back(polygonGroup);
            }

        }
    }
}

bool TextureSquarenessChecker::CheckTexureSquareness(Texture *texure)
{
    if(texure->GetWidth() == texure->GetHeight())
    {
        return true;
    }

    return false;
}

void TextureSquarenessChecker::MakeTexureSquare(Texture *texure, Vector2 &oldSize)
{
    FilePath texturePath = texure->GetPathname();
    texturePath = FilePath::CreateWithNewExtension(texturePath, ".png");

    Image * img = CreateTopLevelImage(texturePath);

    oldSize = Vector2(img->GetWidth(), img->GetHeight());
    img->ResizeToSquare();
    float32 newSize = img->GetWidth();
    ImageLoader::Save(img, texturePath);

    texure->Reload();
}