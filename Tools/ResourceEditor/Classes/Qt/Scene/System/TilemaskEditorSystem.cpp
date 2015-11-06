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


#include "TilemaskEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "../SceneEditor2.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Commands2/TilemaskEditorCommands.h"

#include "Render/Image/ImageConvert.h"

#include <QApplication>

static std::array<FastName, 4> TILECOLOR_PARAM_NAMES;

static const FastName TILEMASK_EDITOR_FLAG_DRAW_TYPE("DRAW_TYPE");
static const FastName TILEMASK_EDITOR_PARAM_INTENSITY("intensity");
static const FastName TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET("copypasteOffset");

static const FastName TILEMASK_EDTIOR_TEXTURE_SOURCE("sourceTexture");
static const FastName TILEMASK_EDTIOR_TEXTURE_TOOL("toolTexture");

static const FastName TILEMASK_EDITOR_MATERIAL_PASS("2d");

TilemaskEditorSystem::TilemaskEditorSystem(Scene* scene)
    : LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
    , curToolSize(0)
    , toolImageTexture(nullptr)
    , landscapeTilemaskTexture(nullptr)
    , tileTextureNum(0)
    , drawingType(TILEMASK_DRAW_NORMAL)
    , strength(0.25f)
    , toolImagePath("")
    , toolImageIndex(0)
    , copyPasteFrom(-1.f, -1.f)
    , editingIsEnabled(false)
    , toolTexture(NULL)
    , toolSpriteUpdated(false)
    , needCreateUndo(false)
    , textureLevel(Landscape::TEXTURE_TILEMASK)
{
    curToolSize = 120;

    editorMaterial = new NMaterial();
    editorMaterial->SetFXName(FastName("~res:/Materials/Landscape.Tilemask.Editor.material"));
    editorMaterial->AddFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, 0);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_INTENSITY, &strength, rhi::ShaderProp::TYPE_FLOAT1);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET, copyPasteOffset.data, rhi::ShaderProp::TYPE_FLOAT2);

    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);

    std::array<float32, 6 * (3 + 2)> buffer = // 6 vertecies by 5 floats: vec3 position, vec2 tex coord
    { { -1.f, -1.f, 0.f, 0.f, 0.f,
        -1.f, 1.f, 0.f, 0.f, 1.f,
        1.f, 1.f, 0.f, 1.f, 1.f,
        -1.f, -1.f, 0.f, 0.f, 0.f,
        1.f, 1.f, 0.f, 1.f, 1.f,
        1.f, -1.f, 0.f, 1.f, 0.f } };

    quadBuffer = rhi::CreateVertexBuffer(buffer.size() * sizeof(float32));
    rhi::UpdateVertexBuffer(quadBuffer, buffer.data(), 0, buffer.size() * sizeof(float32));

    quadPacket.vertexStreamCount = 1;
    quadPacket.vertexStream[0] = quadBuffer;
    quadPacket.vertexCount = 6;
    quadPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    quadPacket.primitiveCount = 2;

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);

    quadPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);

    TILECOLOR_PARAM_NAMES[0] = Landscape::PARAM_TILE_COLOR0;
    TILECOLOR_PARAM_NAMES[1] = Landscape::PARAM_TILE_COLOR1;
    TILECOLOR_PARAM_NAMES[2] = Landscape::PARAM_TILE_COLOR2;
    TILECOLOR_PARAM_NAMES[3] = Landscape::PARAM_TILE_COLOR3;
}

TilemaskEditorSystem::~TilemaskEditorSystem()
{
    rhi::DeleteVertexBuffer(quadBuffer);

    SafeRelease(editorMaterial);

    SafeRelease(toolImageTexture);
    SafeRelease(toolTexture);
}

LandscapeEditorDrawSystem::eErrorType TilemaskEditorSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}
	
	LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
	if ( canBeEnabledError!= LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return canBeEnabledError;
	}
	
	LandscapeEditorDrawSystem::eErrorType enablingError = drawSystem->EnableTilemaskEditing();
	if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enablingError;
	}

    drawSystem->GetLandscapeProxy()->UpdateTileMaskPathname();

    selectionSystem->SetLocked(true);
    modifSystem->SetLocked(true);

    landscapeSize = drawSystem->GetTextureSize(textureLevel);
    copyPasteFrom = Vector2(-1.f, -1.f);

    drawSystem->EnableCursor();
    drawSystem->EnableCustomDraw();
    drawSystem->SetCursorTexture(cursorTexture);
    drawSystem->SetCursorSize(cursorSize);
    SetBrushSize(curToolSize);

    InitSprites();

    drawSystem->GetLandscapeProxy()->InitTilemaskImageCopy();

    Texture* srcSprite = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
    Texture* dstSprite = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_DESTINATION);

    srcSprite->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
    dstSprite->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    editorMaterial->AddTexture(TILEMASK_EDTIOR_TEXTURE_TOOL, toolTexture);
    editorMaterial->AddTexture(TILEMASK_EDTIOR_TEXTURE_SOURCE, srcSprite);

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool TilemaskEditorSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	FinishEditing();

	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();
    drawSystem->DisableTilemaskEditing();

    editorMaterial->RemoveTexture(TILEMASK_EDTIOR_TEXTURE_TOOL);
    editorMaterial->RemoveTexture(TILEMASK_EDTIOR_TEXTURE_SOURCE);

    SafeRelease(landscapeTilemaskTexture);

    enabled = false;
    return !enabled;
}

void TilemaskEditorSystem::Process(float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		if (prevCursorPos != cursorPosition)
		{
			prevCursorPos = cursorPosition;

            Vector2 toolSize = Vector2((float32)curToolSize, (float32)curToolSize);
            Vector2 toolPos = cursorPosition * landscapeSize - toolSize / 2.f;
            Rect toolRect(toolPos, toolSize);

            RenderSystem2D::Instance()->BeginRenderTargetPass(toolTexture);
            RenderSystem2D::Instance()->DrawTexture(toolImageTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, toolRect);
            RenderSystem2D::Instance()->EndRenderTargetPass();

            toolSpriteUpdated = true;

            if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
            {
                editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET, copyPasteOffset.data);
            }

            AddRectToAccumulator(toolRect);
        }
    }
}

void TilemaskEditorSystem::Input(UIEvent* event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	UpdateCursorPosition();
	
	if (event->tid == UIEvent::BUTTON_1)
	{
		Vector3 point;

		switch(event->phase)
		{
        case UIEvent::Phase::BEGAN:
            if (isIntersectsLandscape && !needCreateUndo)
            {
                    if (drawingType == TILEMASK_DRAW_COPY_PASTE)
					{
						int32 curKeyModifiers = QApplication::keyboardModifiers();
						if (curKeyModifiers & Qt::AltModifier)
						{
							copyPasteFrom = cursorPosition;
                            copyPasteOffset = Vector2();
                            return;
                        }
                        else
						{
							if (copyPasteFrom == Vector2(-1.f, -1.f))
							{
								return;
							}
                            copyPasteOffset = copyPasteFrom - cursorPosition;
                        }
                    }

                    ResetAccumulatorRect();
					editingIsEnabled = true;
					activeDrawingType = drawingType;
				}
				break;

        case UIEvent::Phase::DRAG:
            break;

        case UIEvent::Phase::ENDED:
            FinishEditing();
            break;
        }
	}
}

void TilemaskEditorSystem::FinishEditing()
{
	if (editingIsEnabled)
	{
		needCreateUndo = true;
		editingIsEnabled = false;
	}
	prevCursorPos = Vector2(-1.f, -1.f);
}

void TilemaskEditorSystem::SetBrushSize(int32 brushSize)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = (float32)brushSize / landscapeSize;
        drawSystem->SetCursorSize(cursorSize);

        UpdateToolImage();
    }
}

void TilemaskEditorSystem::SetStrength(float32 _strength)
{
    if (_strength >= 0)
    {
        strength = _strength;
        editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_INTENSITY, &strength);
    }
}

void TilemaskEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
	this->toolImagePath = toolImagePath;
	this->toolImageIndex = index;
    UpdateToolImage();
}

void TilemaskEditorSystem::SetTileTexture(uint32 tileTexture)
{
	if (tileTexture >= GetTileTextureCount())
	{
		return;
	}
	
	tileTextureNum = tileTexture;

    editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, tileTextureNum);
    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
}

void TilemaskEditorSystem::UpdateBrushTool()
{
    if (drawingType == TILEMASK_DRAW_COPY_PASTE && (copyPasteFrom == Vector2(-1.f, -1.f)))
        return;

    Texture* srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
    Texture* dstTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_DESTINATION);

    editorMaterial->SetTexture(TILEMASK_EDTIOR_TEXTURE_SOURCE, srcTexture);

    rhi::RenderPassConfig passConf;
    passConf.colorBuffer[0].texture = dstTexture->handle;
    passConf.priority = PRIORITY_SERVICE_2D;
    passConf.viewport.width = dstTexture->GetWidth();
    passConf.viewport.height = dstTexture->GetHeight();
    passConf.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    Memset(passConf.colorBuffer[0].clearColor, 0, 4 * sizeof(float32));

    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
    editorMaterial->BindParams(quadPacket);

    rhi::HPacketList pList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConf, 1, &pList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(pList);

    rhi::AddPacket(pList, quadPacket);

    rhi::EndPacketList(pList);
    rhi::EndRenderPass(pass);

    drawSystem->SetTileMaskTexture(dstTexture);
    drawSystem->GetLandscapeProxy()->SwapTilemaskDrawTextures();
}

void TilemaskEditorSystem::UpdateToolImage()
{
    SafeRelease(toolImageTexture);

    Vector<Image*> images;
    ImageSystem::Instance()->Load(toolImagePath, images);
    if (images.size())
    {
        DVASSERT(images.size() == 1);
        DVASSERT(images[0]->GetPixelFormat() == FORMAT_RGBA8888);

        Image* toolImage = Image::Create(curToolSize, curToolSize, FORMAT_RGBA8888);
        ImageConvert::ResizeRGBA8Billinear((uint32*)images[0]->data, images[0]->GetWidth(), images[0]->GetHeight(),
                                           (uint32*)toolImage->data, curToolSize, curToolSize);

        SafeRelease(images[0]);

        toolImageTexture = Texture::CreateFromData(toolImage, false);
    }
}

void TilemaskEditorSystem::AddRectToAccumulator(const Rect& rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

void TilemaskEditorSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

Rect TilemaskEditorSystem::GetUpdatedRect()
{
	Rect r = updatedRectAccumulator;
	drawSystem->ClampToTexture(textureLevel, r);

	return r;
}

uint32 TilemaskEditorSystem::GetTileTextureCount() const
{
	return 4;
}

Texture* TilemaskEditorSystem::GetTileTexture()
{
    return drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE);
}

Color TilemaskEditorSystem::GetTileColor(int32 index)
{
	if (index < 0 || index >= (int32)GetTileTextureCount())
	{
		return Color::Black;
	}

    return drawSystem->GetLandscapeProxy()->GetLandscapeTileColor(TILECOLOR_PARAM_NAMES[index]);
}

void TilemaskEditorSystem::SetTileColor(int32 index, const Color& color)
{
	if (index < 0 || index >= (int32)GetTileTextureCount())
	{
		return;
	}

    Color curColor = drawSystem->GetLandscapeProxy()->GetLandscapeTileColor(TILECOLOR_PARAM_NAMES[index]);

    if (curColor != color)
    {
        SceneEditor2* scene = (SceneEditor2*)(GetScene());
        scene->Exec(new SetTileColorCommand(drawSystem->GetLandscapeProxy(), TILECOLOR_PARAM_NAMES[index], color));
    }
}

void TilemaskEditorSystem::CreateMaskTexture()
{
    Texture* tilemask = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK);
    Texture* srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE);

    if (tilemask != srcTexture)
    {
        landscapeTilemaskTexture = SafeRetain(tilemask);

        RenderSystem2D::Instance()->BeginRenderTargetPass(srcTexture);
        RenderSystem2D::Instance()->DrawTexture(landscapeTilemaskTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White);
        RenderSystem2D::Instance()->EndRenderTargetPass();

        drawSystem->SetTileMaskTexture(srcTexture);
    }
}

void TilemaskEditorSystem::Draw()
{
    if (!IsLandscapeEditingEnabled())
	{
		return;
	}

	if (toolSpriteUpdated)
	{
		UpdateBrushTool();
		toolSpriteUpdated = false;
	}

	if (needCreateUndo)
	{
		CreateUndoPoint();
		needCreateUndo = false;
	}
}

void TilemaskEditorSystem::CreateUndoPoint()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ModifyTilemaskCommand(drawSystem->GetLandscapeProxy(), GetUpdatedRect()));
}

int32 TilemaskEditorSystem::GetBrushSize()
{
    return curToolSize;
}

float32 TilemaskEditorSystem::GetStrength()
{
	return strength;
}

int32 TilemaskEditorSystem::GetToolImage()
{
	return toolImageIndex;
}

uint32 TilemaskEditorSystem::GetTileTextureIndex()
{
	return tileTextureNum;
}

void TilemaskEditorSystem::InitSprites()
{
	float32 texSize = drawSystem->GetTextureSize(textureLevel);

	if (toolTexture == NULL)
	{
        toolTexture = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888 /*, Texture::DEPTH_NONE*/);
    }

    drawSystem->GetLandscapeProxy()->InitTilemaskDrawTextures();
    CreateMaskTexture();
}

void TilemaskEditorSystem::SetDrawingType(eTilemaskDrawType type)
{
	if (type >= TILEMASK_DRAW_NORMAL && type < TILEMASK_DRAW_TYPES_COUNT)
	{
		drawingType = type;

        if (type == TILEMASK_DRAW_COPY_PASTE)
        {
            editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, 4);
        }
        else
        {
            editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, tileTextureNum);
        }

        editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
    }
}

TilemaskEditorSystem::eTilemaskDrawType TilemaskEditorSystem::GetDrawingType()
{
	return drawingType;
}
