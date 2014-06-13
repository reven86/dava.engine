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



#include "VisibilityToolSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/VisibilityToolProxy.h"
#include "Deprecated/EditorConfig.h"
#include "Scene/SceneSignals.h"
#include "Commands2/VisibilityToolActions.h"

VisibilityToolSystem::VisibilityToolSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
,	editingIsEnabled(false)
,	curToolSize(0)
,	cursorSize(120)
,	prevCursorPos(Vector2(-1.f, -1.f))
,	originalImage(NULL)
,	state(VT_STATE_NORMAL)
,	textureLevel(Landscape::TEXTURE_TILE_FULL)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.tex");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	crossTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/setPointCursor.tex");
	crossTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
}

VisibilityToolSystem::~VisibilityToolSystem()
{
	SafeRelease(cursorTexture);
	SafeRelease(crossTexture);
}

bool VisibilityToolSystem::IsLandscapeEditingEnabled() const
{
	return enabled;
}

LandscapeEditorDrawSystem::eErrorType VisibilityToolSystem::IsCanBeEnabled()
{
	return drawSystem->VerifyLandscape();
}

LandscapeEditorDrawSystem::eErrorType VisibilityToolSystem::EnableLandscapeEditing()
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

	SetState(VT_STATE_NORMAL);
	
	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);

	Texture* visibilityToolTexture = drawSystem->GetVisibilityToolProxy()->GetSprite()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTexture(visibilityToolTexture);
	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTextureEnabled(true);
	landscapeSize = visibilityToolTexture->GetWidth();

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorSize(0);

	PrepareConfig();

	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool VisibilityToolSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	SetState(VT_STATE_NORMAL);

	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);

	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();

	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTexture(NULL);
	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTextureEnabled(false);

	enabled = false;
	return !enabled;
}

void VisibilityToolSystem::Process(DAVA::float32 timeElapsed)
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
		}
	}
}

void VisibilityToolSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}

	UpdateCursorPosition(landscapeSize);

	if (state != VT_STATE_SET_AREA && state != VT_STATE_SET_POINT)
	{
		return;
	}

	if (UIEvent::PHASE_KEYCHAR == event->phase)
	{
		if (event->tid == DVKEY_ESCAPE)
		{
			SetState(VT_STATE_NORMAL);
		}
	}
	else if (event->tid == UIEvent::BUTTON_1)
	{
		Vector3 point;

		switch(event->phase)
		{
			case UIEvent::PHASE_BEGAN:
				if (isIntersectsLandscape)
				{
					StoreOriginalState();
					editingIsEnabled = true;
				}
				break;

			case UIEvent::PHASE_DRAG:
				break;

			case UIEvent::PHASE_ENDED:
				if (editingIsEnabled)
				{
					if (state == VT_STATE_SET_POINT)
					{
						SetVisibilityPointInternal(cursorPosition);
					}
					else if (state == VT_STATE_SET_AREA)
					{
						SetVisibilityAreaInternal();
					}

					CreateUndoPoint();
					editingIsEnabled = false;
				}
				break;
		}
	}
}

void VisibilityToolSystem::UpdateCursorPosition(int32 landscapeSize)
{
	Vector3 landPos;
	isIntersectsLandscape = false;
	if (collisionSystem->LandRayTestFromCamera(landPos))
	{
		isIntersectsLandscape = true;
		Vector2 point(landPos.x, landPos.y);

		point.x = (float32)((int32)point.x);
		point.y = (float32)((int32)point.y);

		AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

		cursorPosition.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
		cursorPosition.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
		cursorPosition.x = (int32)cursorPosition.x;
		cursorPosition.y = (int32)cursorPosition.y;

		drawSystem->SetCursorPosition(cursorPosition);
	}
	else
	{
		// hide cursor
		drawSystem->SetCursorPosition(DAVA::Vector2(-100, -100));
	}
}

void VisibilityToolSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

void VisibilityToolSystem::AddRectToAccumulator(const Rect &rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

Rect VisibilityToolSystem::GetUpdatedRect()
{
	Rect r = updatedRectAccumulator;
	drawSystem->ClampToTexture(textureLevel, r);

	return r;
}

void VisibilityToolSystem::SetBrushSize(int32 brushSize)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;

		if (state == VT_STATE_SET_AREA)
		{
			drawSystem->SetCursorSize(cursorSize);
		}
	}
}

void VisibilityToolSystem::StoreOriginalState()
{
	DVASSERT(originalImage == NULL);
	originalImage = drawSystem->GetVisibilityToolProxy()->GetSprite()->GetTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
	ResetAccumulatorRect();
}

void VisibilityToolSystem::CreateUndoPoint()
{
	SafeRelease(originalImage);
}

void VisibilityToolSystem::PrepareConfig()
{
	EditorConfig* config = EditorConfig::Instance();

	pointsDensity = 10.f;
	VariantType* value = config->GetPropertyDefaultValue("LevelVisibilityDensity");
	if(value && config->GetPropertyValueType("LevelVisibilityDensity") == VariantType::TYPE_FLOAT)
		pointsDensity = value->AsFloat();

	visibilityPointHeight = 2.f;
	areaPointHeights.clear();
	const Vector<String>& heights = config->GetComboPropertyValues("LevelVisibilityPointHeights");
	if(heights.size() != 0)
	{
		if(heights.front() != "none")
		{
			std::sscanf(heights[0].c_str(), "%f", &visibilityPointHeight);

            areaPointHeights.reserve(heights.size() - 1);
			for(uint32 i = 1; i < heights.size(); ++i)
			{
				float32 val;
				std::sscanf(heights[i].c_str(), "%f", &val);
				areaPointHeights.push_back(val);
			}
		}
	}

	areaPointColors = config->GetColorPropertyValues("LevelVisibilityColors");
}

void VisibilityToolSystem::SetState(eVisibilityToolState newState)
{
	if(state == newState)
		state = VT_STATE_NORMAL;
	else
		state = newState;

	switch(state)
	{
		case VT_STATE_SET_POINT:
			drawSystem->SetCursorTexture(crossTexture);
			drawSystem->SetCursorSize(CROSS_TEXTURE_SIZE);
			break;

		case VT_STATE_SET_AREA:
			drawSystem->SetCursorTexture(cursorTexture);
			drawSystem->SetCursorSize(cursorSize);
			break;

		default:
			if (IsLandscapeEditingEnabled())
			{
				drawSystem->SetCursorSize(0);
			}
			break;
	}
	SceneSignals::Instance()->EmitVisibilityToolStateChanged(dynamic_cast<SceneEditor2*>(GetScene()), state);
}

void VisibilityToolSystem::SetVisibilityPoint()
{
	SetState(VT_STATE_SET_POINT);
}

void VisibilityToolSystem::SetVisibilityArea()
{
	SetState(VT_STATE_SET_AREA);
}

void VisibilityToolSystem::SetVisibilityPointInternal(const Vector2& point)
{
	Sprite* sprite = Sprite::CreateAsRenderTarget(CROSS_TEXTURE_SIZE, CROSS_TEXTURE_SIZE, FORMAT_RGBA8888);

	Sprite* cursorSprite = Sprite::CreateFromTexture(crossTexture, 0, 0,
													 crossTexture->GetWidth(), crossTexture->GetHeight());

	RenderManager::Instance()->SetRenderTarget(sprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

    Sprite::DrawState drawState;
    drawState.SetPosition(0.f, 0.f);
	drawState.SetScaleSize(sprite->GetWidth(), sprite->GetHeight(),
                           cursorSprite->GetWidth(), cursorSprite->GetHeight());
	cursorSprite->Draw(&drawState);

	RenderManager::Instance()->RestoreRenderTarget();

	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ActionSetVisibilityPoint(originalImage, sprite,
											 drawSystem->GetVisibilityToolProxy(), cursorPosition));

	SafeRelease(originalImage);
	SafeRelease(sprite);
	SafeRelease(cursorSprite);

	SetState(VT_STATE_NORMAL);
}

void VisibilityToolSystem::SetVisibilityAreaInternal()
{
	if (drawSystem->GetVisibilityToolProxy()->IsVisibilityPointSet())
	{
		Vector2 areaSize = Vector2(cursorSize, cursorSize);
		Vector2 areaPos = cursorPosition;// - areaSize / 2;

		Rect updatedRect;
		updatedRect.SetPosition(areaPos - areaSize / 2.f);
		updatedRect.SetSize(areaSize);
		AddRectToAccumulator(updatedRect);

		Vector2 visibilityPoint = drawSystem->GetVisibilityToolProxy()->GetVisibilityPoint();

		Vector3 point(visibilityPoint);
		point.z = drawSystem->GetHeightAtPoint(drawSystem->TexturePointToHeightmapPoint(textureLevel, visibilityPoint));
		point.z += visibilityPointHeight;

		Vector<Vector3> resP;
		PerformHeightTest(point, areaPos, cursorSize / 2.f, pointsDensity, areaPointHeights, &resP);
		DrawVisibilityAreaPoints(resP);

		SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
		DVASSERT(scene);
		scene->Exec(new ActionSetVisibilityArea(originalImage, drawSystem->GetVisibilityToolProxy(), GetUpdatedRect()));

		SafeRelease(originalImage);
	}
	else
	{
		// show "could not check visibility without visibility point" error message
	}
}

void VisibilityToolSystem::PerformHeightTest(Vector3 spectatorCoords,
											 Vector2 circleCenter,
											 float32 circleRadius,
											 float32 density,
											 const Vector<float32>& heightValues,
											 Vector<Vector3>* colorizedPoints)
{
	DVASSERT(colorizedPoints);
	if(heightValues.size() == 0 )
	{
		return;
	}

	Vector2 startOfCounting(circleCenter.x - circleRadius, circleCenter.y - circleRadius);
	Vector2 SpectatorCoords2D(spectatorCoords.x, spectatorCoords.y);

	// get source point in propper coords system
	Vector3 sourcePoint(drawSystem->TexturePointToLandscapePoint(textureLevel, SpectatorCoords2D));

	sourcePoint.z = spectatorCoords.z;

	uint32	hight = heightValues.size();
	uint32	sideLength = (circleRadius * 2) / density;

	Vector< Vector< Vector< Vector3 > > > points;
    points.reserve(hight);

	for(uint32 layerIndex = 0; layerIndex < hight; ++layerIndex)
	{
		Vector<Vector<Vector3> > xLine;
        xLine.reserve(sideLength);
		for(uint32 x = 0; x < sideLength; ++x)
		{
			float xOfPoint = startOfCounting.x + density * x;
			Vector<Vector3> yLine;
            yLine.reserve(sideLength);
            
			for(uint32 y = 0; y < sideLength; ++y)
			{
				float yOfPoint = startOfCounting.y + density * y;
				float32 zOfPoint = drawSystem->GetHeightAtTexturePoint(textureLevel, Vector2(xOfPoint, yOfPoint));
				zOfPoint += heightValues[layerIndex];
				Vector3 pointToInsert(xOfPoint, yOfPoint, zOfPoint);
				yLine.push_back(pointToInsert);
			}
			xLine.push_back(yLine);
		}
		points.push_back(xLine);
	}

	colorizedPoints->clear();

	float32 textureSize = drawSystem->GetTextureSize(textureLevel);
	Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));
	for(uint32 x = 0; x < sideLength; ++x)
	{
		for(uint32 y = 0; y < sideLength; ++y)
		{
			Vector3 target(drawSystem->TexturePointToLandscapePoint(textureLevel, Vector2(points[0][x][y].x, points[0][x][y].y)));

			bool prevWasIntersection = true;
			for(int32 layerIndex = hight - 1; layerIndex >= 0; --layerIndex)
			{
				Vector3 targetTmp = points[layerIndex][x][y];
				if (!IsCircleContainsPoint(circleCenter, circleRadius, Vector2(targetTmp.x, targetTmp.y)) ||
					!textureRect.PointInside(Vector2(targetTmp.x, targetTmp.y)))
				{
					break;
				}

				target.z = targetTmp.z;

				const EntityGroup* entityGroup = collisionSystem->ObjectsRayTest(sourcePoint, target);
				bool wasIntersection = (entityGroup->Size() == 0) ? false : true;

				if (!wasIntersection)
				{
					Vector3 p;
					wasIntersection = collisionSystem->LandRayTest(sourcePoint, target, p);
				}

				float colorIndex = 0;
				if(prevWasIntersection == false)
				{
					if(wasIntersection)
					{
						colorIndex = (float)layerIndex + 2; // +1 - because need layer from prev loop, and +1 - 'cause the first color reserved for  "death zone"
						Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex);
						colorizedPoints->push_back(exportData);
						break;
					}
					else
					{
						if(layerIndex == 0)
						{
							colorIndex = 1;
							Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex);
							colorizedPoints->push_back(exportData);
						}
					}
				}
				else
				{
					if(layerIndex == hight - 1 && wasIntersection)
					{
						colorIndex = 0;
						Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex );
						colorizedPoints->push_back(exportData);
						break;
					}
				}
				prevWasIntersection = wasIntersection;
			}
		}
	}
}

bool VisibilityToolSystem::IsCircleContainsPoint(const Vector2& circleCenter, float32 circleRadius, const Vector2& point)
{
	return (point - circleCenter).Length() < (circleRadius * 0.9f);
}

void VisibilityToolSystem::DrawVisibilityAreaPoints(const Vector<DAVA::Vector3> &points)
{
	VisibilityToolProxy* visibilityToolProxy = drawSystem->GetVisibilityToolProxy();
	Sprite* visibilityAreaSprite = visibilityToolProxy->GetSprite();

	RenderManager* manager = RenderManager::Instance();
	RenderHelper* helper = RenderHelper::Instance();

	manager->SetRenderTarget(visibilityAreaSprite);

	for(uint32 i = 0; i < points.size(); ++i)
	{
		uint32 colorIndex = (uint32)points[i].z;
		Vector2 pos(points[i].x, points[i].y);

		manager->SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
		manager->SetColor(areaPointColors[colorIndex]);
		helper->DrawPoint(pos, 5.f, DAVA::RenderState::RENDERSTATE_2D_BLEND);
	}

	manager->ResetColor();
	manager->RestoreRenderTarget();
}

void VisibilityToolSystem::SaveTexture(const FilePath& filePath)
{
	if (filePath.IsEmpty())
	{
		return;
	}

	Sprite* visibilityToolSprite = drawSystem->GetVisibilityToolProxy()->GetSprite();
	Texture* visibilityToolTexture = visibilityToolSprite->GetTexture();

	Image* image = visibilityToolTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
    ImageSystem::Instance()->Save(filePath, image);
	SafeRelease(image);
}

VisibilityToolSystem::eVisibilityToolState VisibilityToolSystem::GetState()
{
	return state;
}

int32 VisibilityToolSystem::GetBrushSize()
{
	return cursorSize;
}
