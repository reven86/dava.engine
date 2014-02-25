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


#include "EditorLandscape.h"
#include "LandscapeRenderer.h"
#include "EditorHeightmap.h"

using namespace DAVA;

EditorLandscape::EditorLandscape()
    : Landscape()
{
    // RETURN TO THIS CODE LATER
    //SetName(String("Landscape_EditorNode"));
    
    landscapeRenderer = NULL;
    nestedLandscape = NULL;
    parentLandscape = NULL;
}

EditorLandscape::~EditorLandscape()
{
    SafeRelease(landscapeRenderer);
    SafeRelease(nestedLandscape);
    SafeRelease(parentLandscape);
}

void EditorLandscape::SetNestedLandscape(DAVA::Landscape *landscapeNode)
{
    SafeRelease(nestedLandscape);
    nestedLandscape = SafeRetain(landscapeNode);
    
    EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(landscapeNode);
    if(editorLandscape)
    {
        editorLandscape->SetParentLandscape(this);
    }
    
    // RETURN TO THIS CODE LATER
    // SetDebugFlags(nestedLandscape->GetDebugFlags());
    
    SetHeightmap(nestedLandscape->GetHeightmap());
    heightmapPath = nestedLandscape->GetHeightmapPathname();

    SetTexture(TEXTURE_TILE_FULL, nestedLandscape->GetTexture(TEXTURE_TILE_FULL));
    
    bbox = nestedLandscape->GetBoundingBox();
    CopyCursorData(nestedLandscape, this);
    
    SetDisplayedTexture();
}


void EditorLandscape::SetHeightmap(DAVA::Heightmap *height)
{
    SafeRelease(heightmap);
    heightmap = SafeRetain(height);
    
    if(IsPointerToExactClass<EditorHeightmap>(height))
    {
        EditorHeightmap *editorHeightmap = (EditorHeightmap *)height;
        HeihghtmapUpdated(Rect(0, 0, (float32)editorHeightmap->Size() - 1.f, (float32)editorHeightmap->Size() - 1.f));
    }
}


void EditorLandscape::SetRenderer(LandscapeRenderer *renderer)
{
    SafeRelease(landscapeRenderer);
    landscapeRenderer = SafeRetain(renderer);
}


void EditorLandscape::Draw(Camera * camera)
{
    //if (!(flags & NODE_VISIBLE)) return;
    if(!landscapeRenderer) return;
    
    
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, camera->GetMatrix());
	
    landscapeRenderer->BindMaterial(GetTexture(Landscape::TEXTURE_TILE_FULL));
    
    landscapeRenderer->DrawLandscape();
    
#if defined(__DAVAENGINE_OPENGL__)
//    if (debugFlags & DEBUG_DRAW_GRID)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        RenderManager::Instance()->SetColor(1.0f, 1.f, 1.f, 1.f);
//        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
//        RenderManager::Instance()->SetShader(0);
//        RenderManager::Instance()->FlushState();
//
//        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, landscapeRenderer->Indicies());
//
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    }
#endif //#if defined(__DAVAENGINE_OPENGL__)

    
	if(cursor)
	{
		RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		cursor->Prepare();
        
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, landscapeRenderer->Indicies());
        
		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderState::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}

    landscapeRenderer->UnbindMaterial();
}
    

void EditorLandscape::HeihghtmapUpdated(const DAVA::Rect &forRect)
{
    EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(nestedLandscape);
    if(editorLandscape)
    {
        editorLandscape->HeihghtmapUpdated(forRect);
    }
}

void EditorLandscape::SetDisplayedTexture()
{
    
}

DAVA::Landscape *EditorLandscape::GetNestedLandscape()
{
    return nestedLandscape;
}


void EditorLandscape::SetParentLandscape(EditorLandscape *landscapeNode)
{
    SafeRelease(parentLandscape);
    parentLandscape = SafeRetain(landscapeNode);
}

EditorLandscape *EditorLandscape::GetParentLandscape()
{
    return parentLandscape;
}

LandscapeRenderer *EditorLandscape::GetRenderer()
{
    return landscapeRenderer;
}

void EditorLandscape::CopyCursorData(DAVA::Landscape *sourceLandscape, DAVA::Landscape *destinationLandscape)
{
    if(!sourceLandscape || !destinationLandscape)
        return;
    
    LandscapeCursor *sourceCursor = sourceLandscape->GetCursor();
    LandscapeCursor *destinationCursor = destinationLandscape->GetCursor();
    
    if(!sourceCursor && destinationCursor)
    {
        destinationLandscape->CursorDisable();
    }
    else if(sourceCursor)
    {
        if(!destinationCursor)
        {
            destinationLandscape->CursorEnable();
        }
        
        destinationLandscape->SetCursorTexture(sourceCursor->GetCursorTexture());
        destinationLandscape->SetBigTextureSize(sourceCursor->GetBigTextureSize());
		destinationLandscape->SetCursorPosition(sourceCursor->GetCursorPosition());
		destinationLandscape->SetCursorScale(sourceCursor->GetCursorScale());
    }
}

void EditorLandscape::FlushChanges()
{
    if(parentLandscape)
    {
        parentLandscape->FlushChanges();
    }
    
    if(nestedLandscape)
    {
        CopyCursorData(this, nestedLandscape);
        // RETURN TO THIS CODE LATER
        //nestedLandscape->SetDebugFlags(GetDebugFlags());
    }
}

void EditorLandscape::DrawFullTiledTexture(DAVA::Texture *renderTarget, const DAVA::Rect &drawRect)
{
    Texture *fullTiledTexture = nestedLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL);
    Sprite *background = Sprite::CreateFromTexture(fullTiledTexture, 0, 0, (float32)fullTiledTexture->GetWidth(), (float32)fullTiledTexture->GetHeight());

    background->SetPosition(0.f, 0.f);
    background->SetScaleSize((float32)renderTarget->GetWidth(), (float32)renderTarget->GetHeight());
    background->Draw();
    
    SafeRelease(background);
}

void EditorLandscape::UpdateFullTiledTexture()
{
    nestedLandscape->UpdateFullTiledTexture();
}

void EditorLandscape::BuildLandscapeFromHeightmapImage(const DAVA::FilePath & heightmapPathname, const DAVA::AABBox3 & landscapeBox)
{
    nestedLandscape->BuildLandscapeFromHeightmapImage(heightmapPathname, landscapeBox);
}

Texture * EditorLandscape::GetTexture(eTextureLevel level)
{
    if(level == TEXTURE_TILE_FULL)
    {
        return GetDisplayedTexture();
    }

    return nestedLandscape->GetTexture(level);
}

Texture * EditorLandscape::GetDisplayedTexture()
{
    return nestedLandscape->GetTexture(TEXTURE_TILE_FULL);
}

DAVA::RenderObject * EditorLandscape::Clone( DAVA::RenderObject *newObject )
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<EditorLandscape>(this), "Can clone only EditorLandscape");
		newObject = new EditorLandscape();
	}

	return Landscape::Clone(newObject);
}


