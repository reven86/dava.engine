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


#include "HeightmapEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Commands2/HeightmapEditorCommands2.h"
#include "Main/QtUtils.h"
#include "HoodSystem.h"

#include <QApplication>

HeightmapEditorSystem::HeightmapEditorSystem(Scene* scene)
:	LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
,	editingIsEnabled(false)
,	curToolSize(0)
,	originalHeightmap(NULL)
,	toolImage(NULL)
,	strength(15)
,	averageStrength(0.5f)
,	inverseDrawingEnabled(false)
,	toolImagePath("")
,	drawingType(HEIGHTMAP_DRAW_ABSOLUTE)
,	copyPasteFrom(-1.f, -1.f)
,	copyPasteTo(-1.f, -1.f)
,   squareTexture(NULL)
,	toolImageIndex(0)
,	curHeight(0.f)
,	activeDrawingType(drawingType)
,	textureLevel(Landscape::TEXTURE_TILE_MASK)
{
    cursorSize = 30;
	
	noBlendDrawState = DAVA::RenderManager::Instance()->Subclass3DRenderState(DAVA::BLEND_ONE, DAVA::BLEND_ZERO);
}

HeightmapEditorSystem::~HeightmapEditorSystem()
{
	SafeRelease(squareTexture);
}

LandscapeEditorDrawSystem::eErrorType HeightmapEditorSystem::EnableLandscapeEditing()
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

	LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
	if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enableCustomDrawError;
	}

	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);

	landscapeSize = drawSystem->GetHeightmapProxy()->Size();
	copyPasteFrom = Vector2(-1.f, -1.f);

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);

	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool HeightmapEditorSystem::DisableLandscapeEdititing()
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
	
	enabled = false;
	return !enabled;
}

void HeightmapEditorSystem::Process(DAVA::float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		UpdateToolImage();
		UpdateBrushTool(timeElapsed);
	}
}

void HeightmapEditorSystem::Input(DAVA::UIEvent *event)
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
                if (drawingType == HEIGHTMAP_DRAW_ABSOLUTE_DROPPER ||
					drawingType == HEIGHTMAP_DROPPER)
				{
					curHeight = drawSystem->GetHeightAtPoint(GetHeightmapPositionFromCursor());
					
					SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
				}
				
				if (isIntersectsLandscape)
				{
					if (drawingType == HEIGHTMAP_COPY_PASTE)
					{
						int32 curKeyModifiers = QApplication::keyboardModifiers();
						if (curKeyModifiers & Qt::AltModifier)
						{
							copyPasteFrom = GetHeightmapPositionFromCursor();
							copyPasteTo = Vector2(-1.f, -1.f);
							return;
						}
						else
						{
							if (copyPasteFrom == Vector2(-1.f, -1.f))
							{
								return;
							}
							copyPasteTo = GetHeightmapPositionFromCursor();
							StoreOriginalHeightmap();
						}
					}
					else
					{
						if (drawingType != HEIGHTMAP_DROPPER)
						{
							StoreOriginalHeightmap();
						}
					}

					UpdateToolImage();
					editingIsEnabled = true;
				}

				activeDrawingType = drawingType;
				break;

        case UIEvent::Phase::DRAG:
                break;

        case UIEvent::Phase::ENDED:
                FinishEditing();
				break;
		}
	}
}

void HeightmapEditorSystem::FinishEditing()
{
	if (editingIsEnabled)
	{
		if (drawingType != HEIGHTMAP_DROPPER)
		{
			CreateHeightmapUndo();
		}
		editingIsEnabled = false;
	}
}


void HeightmapEditorSystem::UpdateToolImage(bool force)
{
	if (toolImage)
	{
		if (curToolSize != cursorSize || force)
		{
			SafeRelease(toolImage);
		}
	}
	
	if (!toolImage)
	{
		if (!toolImagePath.IsEmpty())
		{
			toolImage = CreateToolImage(cursorSize, toolImagePath);
			curToolSize = cursorSize;
		}
	}
}

Image* HeightmapEditorSystem::CreateToolImage(int32 sideSize, const FilePath& filePath)
{
	Texture *dstTex = Texture::CreateFBO((float32)sideSize, (float32)sideSize, FORMAT_RGBA8888, Texture::DEPTH_NONE);
	Texture *srcTex = Texture::CreateFromFile(filePath);
	
    RenderHelper::Instance()->Set2DRenderTarget(dstTex);
    RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	RenderManager::Instance()->SetColor(Color::White);
    RenderHelper::Instance()->DrawTexture(srcTex, RenderState::RENDERSTATE_2D_BLEND, Rect((dstTex->GetWidth() - sideSize) / 2.f, (dstTex->GetHeight() - sideSize) / 2.f, (float32)sideSize, (float32)sideSize));
    RenderManager::Instance()->SetRenderTarget(0);
	
    Image *retImage = dstTex->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
	
	SafeRelease(srcTex);
    SafeRelease(dstTex);
	
	return retImage;
}

void HeightmapEditorSystem::UpdateBrushTool(float32 timeElapsed)
{
	if (!toolImage)
	{
		DAVA::Logger::Error("Tool image is empty!");
		return;
	}
	
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	int32 scaleSize = toolImage->GetWidth();
	Vector2 pos = GetHeightmapPositionFromCursor() - Vector2((float32)scaleSize, (float32)scaleSize) / 2.0f;
	{
		switch (activeDrawingType)
		{
			case HEIGHTMAP_DRAW_RELATIVE:
			{
				float32 koef = (strength * timeElapsed);
				if(inverseDrawingEnabled)
				{
					koef = -koef;
				}

				if (IsKeyModificatorPressed(DVKEY_ALT))
				{
					koef = -koef;
				}

				editorHeightmap->DrawRelativeRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef);
				break;
			}
				
			case HEIGHTMAP_DRAW_AVERAGE:
			{
				float32 koef = (averageStrength * timeElapsed) * 2.0f;
				editorHeightmap->DrawAverageRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef);
				break;
			}

			case HEIGHTMAP_DRAW_ABSOLUTE:
			case HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
			{
				float32 maxHeight = drawSystem->GetLandscapeMaxHeight();
				float32 height = curHeight / maxHeight * Heightmap::MAX_VALUE;
				
				float32 koef = (averageStrength * timeElapsed) * 2.0f;
				editorHeightmap->DrawAbsoluteRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef, height);
				break;
			}

			case HEIGHTMAP_DROPPER:
			{
				float32 curHeight = drawSystem->GetHeightAtPoint(GetHeightmapPositionFromCursor());
				SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
				return;
			}

			case HEIGHTMAP_COPY_PASTE:
			{
				if (copyPasteFrom == Vector2(-1.f, -1.f) || copyPasteTo == Vector2(-1.f, -1.f))
				{
					return;
				}

				Vector2 posTo = pos;
				
				Vector2 deltaPos = GetHeightmapPositionFromCursor() - copyPasteTo;
				Vector2 posFrom = copyPasteFrom + deltaPos - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;
				
				float32 koef = (averageStrength * timeElapsed) * 2.0f;

				editorHeightmap->DrawCopypasteRGBA(toolImage, posFrom, posTo, scaleSize, scaleSize, koef);

				break;
			}
				
			default:
				DAVA::Logger::Error("Invalid drawing type!");
				return;
		}
		
		Rect rect(pos.x, pos.y, (float32)scaleSize, (float32)scaleSize);
		drawSystem->GetHeightmapProxy()->UpdateRect(rect);
		AddRectToAccumulator(heightmapUpdatedRect, rect);
	}
}

void HeightmapEditorSystem::ResetAccumulatorRect(Rect& accumulator)
{
	float32 inf = std::numeric_limits<float32>::infinity();
	accumulator = Rect(inf, inf, -inf, -inf);
}

void HeightmapEditorSystem::AddRectToAccumulator(Rect& accumulator, const Rect& rect)
{
	accumulator = accumulator.Combine(rect);
}

Rect HeightmapEditorSystem::GetHeightmapUpdatedRect()
{
	Rect r = heightmapUpdatedRect;
	drawSystem->ClampToHeightmap(r);
	return r;
}

void HeightmapEditorSystem::StoreOriginalHeightmap()
{
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	DVASSERT(originalHeightmap == NULL);
	originalHeightmap = editorHeightmap->Clone(NULL);
	ResetAccumulatorRect(heightmapUpdatedRect);
}

void HeightmapEditorSystem::CreateHeightmapUndo()
{
	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ModifyHeightmapCommand(drawSystem->GetHeightmapProxy(),
										   originalHeightmap,
										   GetHeightmapUpdatedRect()));

	SafeRelease(originalHeightmap);
}

void HeightmapEditorSystem::SetBrushSize(int32 brushSize)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;
		drawSystem->SetCursorSize(cursorSize);
	}
}

void HeightmapEditorSystem::SetStrength(float32 strength)
{
	float32 s = abs(strength);
	this->strength = s;
		
	inverseDrawingEnabled = false;
	if (strength < 0.f)
	{
		inverseDrawingEnabled = true;
	}
}

void HeightmapEditorSystem::SetAverageStrength(float32 averageStrength)
{
	if (averageStrength >= 0)
	{
		this->averageStrength = averageStrength;
	}
}

void HeightmapEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
	this->toolImagePath = toolImagePath;
	this->toolImageIndex = index;
	UpdateToolImage(true);
}

void HeightmapEditorSystem::SetDrawingType(eHeightmapDrawType type)
{
	copyPasteFrom = Vector2(-1.f, -1.f);
	drawingType = type;
}

int32 HeightmapEditorSystem::GetBrushSize()
{
	return cursorSize;
}

float32 HeightmapEditorSystem::GetStrength()
{
	float32 s = strength;
	if (inverseDrawingEnabled)
	{
		s = -s;
	}

	return s;
}

float32 HeightmapEditorSystem::GetAverageStrength()
{
	return averageStrength;
}

int32 HeightmapEditorSystem::GetToolImage()
{
	return toolImageIndex;
}

HeightmapEditorSystem::eHeightmapDrawType HeightmapEditorSystem::GetDrawingType()
{
	return drawingType;
}

void HeightmapEditorSystem::SetDropperHeight(float32 height)
{
	float32 maxHeight = drawSystem->GetLandscapeMaxHeight();

	if (height >= 0 && height <= maxHeight)
	{
		curHeight = height;
		SceneSignals::Instance()->EmitDropperHeightChanged(static_cast<SceneEditor2*>(GetScene()), curHeight);
	}
}

float32 HeightmapEditorSystem::GetDropperHeight()
{
	return curHeight;
}

Vector2 HeightmapEditorSystem::GetHeightmapPositionFromCursor() const
{
    return Vector2(cursorPosition.x, landscapeSize - 1 - cursorPosition.y);
}

