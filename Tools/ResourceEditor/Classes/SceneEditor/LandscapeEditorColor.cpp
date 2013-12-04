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



#include "LandscapeEditorColor.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelColor.h"
#include "PropertyControlCreator.h"
#include "EditorScene.h"

#include "HeightmapNode.h"

#include "../LandscapeEditor/EditorHeightmap.h"
#include "../LandscapeEditor/EditorLandscape.h"

#include "../Qt/Main/QtUtils.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/TilemapEditorCommands.h"

LandscapeEditorColor::LandscapeEditorColor(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;
    tileMaskEditorShader = new Shader();
	tileMaskEditorShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask-editor.shader");
	tileMaskEditorShader->Recompile();

    maskSprite = NULL;
	oldMaskSprite = NULL;
	toolSprite = NULL;
    savedTexture = NULL;
    settings = NULL;

    //init draw params
    srcBlendMode = BLEND_SRC_ALPHA;
    dstBlendMode = BLEND_ONE_MINUS_SRC_ALPHA;
    paintColor = Color(1.f, 1.f, 1.f, 1.0f);
    
    toolsPanel = new LandscapeToolsPanelColor(this, toolsRect);

    editingIsEnabled = false;

	originalImage = NULL;
}

LandscapeEditorColor::~LandscapeEditorColor()
{
    SafeRelease(tileMaskEditorShader);

    SafeRelease(savedTexture);
    
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
	SafeRelease(toolSprite);
	SafeRelease(originalImage);
}


void LandscapeEditorColor::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		UpdateTileMask();
        
		RenderManager::Instance()->SetRenderTarget(toolSprite);
		RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
		RenderManager::Instance()->RestoreRenderTarget();
		
		wasTileMaskToolUpdate = false;
	}
}

void LandscapeEditorColor::CreateMaskTexture()
{
    SafeRelease(savedTexture);
    savedTexture = SafeRetain(workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK));
    if(savedTexture)
    {
        savedPath = savedTexture->GetPathname();
    }
    else 
    {
        savedPath = FilePath();
    }
    
    CreateMaskFromTexture(savedTexture);
}

void LandscapeEditorColor::CreateMaskFromTexture(Texture *tex)
{
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
    SafeRelease(toolSprite);
    
    float32 texSize = (float32)settings->maskSize;
    if(tex)
    {
        texSize = (float32)tex->width;
    }
    
    maskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	oldMaskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
    toolSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
    
    if(tex)
    {
        RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
        
        Sprite *oldMask = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);
        
        RenderManager::Instance()->SetRenderTarget(oldMaskSprite);
        oldMask->SetPosition(0.f, 0.f);
        oldMask->Draw();
        RenderManager::Instance()->RestoreRenderTarget();
        
        RenderManager::Instance()->SetRenderTarget(maskSprite);
        oldMask->SetPosition(0.f, 0.f);
        oldMask->Draw();
        RenderManager::Instance()->RestoreRenderTarget();
        
        SafeRelease(oldMask);
    }
    
	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, oldMaskSprite->GetTexture());
}



void LandscapeEditorColor::UpdateTileMask()
{
	int32 colorType;
	if(settings->redMask)
	{
		colorType = 0;
	}
	else if(settings->greenMask)
	{
		colorType = 1;
	}
	else if(settings->blueMask)
	{
		colorType = 2;
	}
	else if(settings->alphaMask)
	{
		colorType = 3;
	}
	else
	{
		return; // no color selected
	}
    
	RenderManager::Instance()->SetRenderTarget(maskSprite);
    
	srcBlendMode = RenderManager::Instance()->GetSrcBlend();
	dstBlendMode = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
    
	RenderManager::Instance()->SetShader(tileMaskEditorShader);
	oldMaskSprite->PrepareSpriteRenderData(0);
	RenderManager::Instance()->SetRenderData(oldMaskSprite->spriteRenderObject);
	RenderManager::Instance()->SetTexture(oldMaskSprite->GetTexture(), 0);
	RenderManager::Instance()->SetTexture(toolSprite->GetTexture(), 1);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    
	int32 tex0 = tileMaskEditorShader->FindUniformIndexByName("texture0");
	tileMaskEditorShader->SetUniformValueByIndex(tex0, 0);
	int32 tex1 = tileMaskEditorShader->FindUniformIndexByName("texture1");
	tileMaskEditorShader->SetUniformValueByIndex(tex1, 1);
	int32 colorTypeUniform = tileMaskEditorShader->FindUniformIndexByName("colorType");
	tileMaskEditorShader->SetUniformValueByIndex(colorTypeUniform, colorType);
	int32 intensityUniform = tileMaskEditorShader->FindUniformIndexByName("intensity");
    
	tileMaskEditorShader->SetUniformValueByIndex(intensityUniform, currentTool->strength);
    
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    
	RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
	RenderManager::Instance()->RestoreRenderTarget();
    
	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, maskSprite->GetTexture());
	Sprite * temp = oldMaskSprite;
	oldMaskSprite = maskSprite;
	maskSprite = temp;
}

void LandscapeEditorColor::UpdateTileMaskTool()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scaleSize = currentTool->sprite->GetWidth() * (currentTool->size * currentTool->size);
        Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;
        if(pos != prevDrawPos)
        {
            wasTileMaskToolUpdate = true;
            
            RenderManager::Instance()->SetRenderTarget(toolSprite);
            currentTool->sprite->SetScaleSize(scaleSize, scaleSize);
            currentTool->sprite->SetPosition(pos);
            currentTool->sprite->Draw();
            RenderManager::Instance()->RestoreRenderTarget();
        }
	}
}


void LandscapeEditorColor::UpdateCursor()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scaleSize = currentTool->sprite->GetWidth() * (currentTool->size * currentTool->size);
		Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;

		workingLandscape->SetCursorTexture(cursorTexture);
		workingLandscape->SetBigTextureSize((float32)workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK)->GetWidth());
		workingLandscape->SetCursorPosition(pos);
		workingLandscape->SetCursorScale(scaleSize);
	}
}


void LandscapeEditorColor::InputAction(int32 phase, bool intersects)
{
    switch(phase)
    {
        case UIEvent::PHASE_BEGAN:
        {
            editingIsEnabled = true;

			StoreOriginalTexture();

            break;
        }
            
        case UIEvent::PHASE_DRAG:
        {
            if(editingIsEnabled && !intersects)
            {
                editingIsEnabled = false;

				CreateUndoPoint();
            }
            else if(!editingIsEnabled && intersects)
            {
                editingIsEnabled = true;

				StoreOriginalTexture();
            }
            break;
        }
            
        case UIEvent::PHASE_ENDED:
        {
            editingIsEnabled = false;

			CreateUndoPoint();

            break;
        }
            
        default:
            break;
    }
    
    
    
    Texture *tex = NULL;
    if(settings->redMask)
    {
        tex = workingLandscape->GetTexture(Landscape::TEXTURE_TILE0);
    }
    else if(settings->greenMask)
    {
        tex = workingLandscape->GetTexture(Landscape::TEXTURE_TILE1);
    }
    else if(settings->blueMask)
    {
        tex = workingLandscape->GetTexture(Landscape::TEXTURE_TILE2);
    }
    else if(settings->alphaMask)
    {
        tex = workingLandscape->GetTexture(Landscape::TEXTURE_TILE3);
    }
    
    if(tex)
    {
        UpdateTileMaskTool(); 
    }
}

Image* LandscapeEditorColor::StoreState()
{
	return maskSprite->GetTexture()->CreateImageFromMemory();
}

void LandscapeEditorColor::RestoreState(Texture* texture)
{
	if (texture)
	{
		CreateMaskFromTexture(texture);
		wasTileMaskToolUpdate = true;
	}
}

void LandscapeEditorColor::HideAction()
{
    workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, savedTexture);
    
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
	SafeRelease(toolSprite);
    
    SafeRelease(originalImage);
    SafeRelease(savedTexture);

	workingLandscape->CursorDisable();
}

void LandscapeEditorColor::ShowAction()
{
    CreateMaskTexture();
    landscapeSize = (int32)maskSprite->GetWidth();

	workingLandscape->CursorEnable();
}

void LandscapeEditorColor::SaveTextureAction(const FilePath &pathToFile)
{
    if(maskSprite)
    {
        Image *img = maskSprite->GetTexture()->CreateImageFromMemory();   
        if(img)
        {
            ImageLoader::Save(img, pathToFile.GetAbsolutePathname());
            SafeRelease(img);
            
            SafeRelease(savedTexture);
            
            FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pathToFile);
			Texture* texture = Texture::CreateFromFile(descriptorPathname);
			if (texture)
			{
				texture->Reload();
			}
			workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);

            savedTexture = SafeRetain(workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK));
            workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, maskSprite->GetTexture());
        }
    }
}

NodesPropertyControl *LandscapeEditorColor::GetPropertyControl(const Rect &rect)
{
    LandscapeEditorPropertyControl *propsControl =
		(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingScene, rect, LandscapeEditorPropertyControl::MASK_EDITOR_MODE);
    propsControl->SetDelegate(this);
    LandscapeEditorSettingsChanged(propsControl->Settings());
    return propsControl;
}

void LandscapeEditorColor::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

void LandscapeEditorColor::TextureWillChanged(const String &forKey)
{
    if("property.landscape.texture.tilemask" == forKey)
    {
        if(!savedPath.IsEmpty())
        {
            SaveTextureAction(savedPath);
        }
    }
}

void LandscapeEditorColor::TextureDidChanged(const String &forKey)
{
    if("property.landscape.texture.tilemask" == forKey)
    {
        CreateMaskTexture();
    }
}

void LandscapeEditorColor::RecreateHeightmapNode()
{
    if(workingScene && heightmapNode)
    {
        workingScene->RemoveNode(heightmapNode);
    }
        
    SafeRelease(heightmapNode);
    heightmapNode = new HeightmapNode(workingScene, workingLandscape);
    workingScene->AddNode(heightmapNode);
}

bool LandscapeEditorColor::SetScene(EditorScene *newScene)
{
    EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(FindLandscape(newScene));
    if(editorLandscape)
    {
        ShowErrorDialog(String("Cannot start tile mask editor. Remove EditorLandscape from scene"));
        return false;
    }
    
    return LandscapeEditorBase::SetScene(newScene);
}

void LandscapeEditorColor::UpdateLandscapeTilemap(Texture* texture)
{
	RestoreState(texture);
}

void LandscapeEditorColor::CreateUndoPoint()
{
	if (originalImage)
	{
		Image* newImage = StoreState();
		CommandsManager::Instance()->ExecuteAndRelease(new CommandDrawTilemap(originalImage,
																			  newImage,
																			  savedPath,
																			  workingLandscape),
													   workingScene);
		SafeRelease(originalImage);
		SafeRelease(newImage);
	}
}

void LandscapeEditorColor::StoreOriginalTexture()
{
	DVASSERT(originalImage == NULL);
	originalImage = StoreState();
}
