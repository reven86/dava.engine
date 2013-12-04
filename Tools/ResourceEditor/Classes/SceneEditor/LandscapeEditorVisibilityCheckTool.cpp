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



#include "LandscapeEditorVisibilityCheckTool.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelCustomColors.h"
#include "PropertyControlCreator.h"
#include "EditorScene.h"
#include "EditorConfig.h"

#include "HeightmapNode.h"

#include "../LandscapeEditor/EditorHeightmap.h"
#include "../LandscapeEditor/EditorLandscape.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Main/QtUtils.h"

#include "EditorBodyControl.h"
#include "cmath"

#include "../Commands/CommandsManager.h"
#include "../Commands/VisibilityCheckToolCommands.h"

#define VISIBILITY_POINT_CURSOR_SCALE 0.2f

LandscapeEditorVisibilityCheckTool::LandscapeEditorVisibilityCheckTool(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl),
		state(VCT_STATE_NORMAL),
		isVisibilityPointSet(false),
		visibilityAreaSprite(0)
{
	wasTileMaskToolUpdate = false;

    settings = NULL;

    toolsPanel = new LandscapeToolsPanelCustomColors(this, toolsRect);

    editingIsEnabled = false;

	texSurf = NULL;
	
	isCursorTransparent = false;

	areaCursorTexture = cursorTexture;
	pointCursorTexture = NULL;

	isFogEnabled = false;
}

LandscapeEditorVisibilityCheckTool::~LandscapeEditorVisibilityCheckTool()
{
	SafeRelease(texSurf);
	SafeRelease(pointCursorTexture);
	SafeRelease(visibilityAreaSprite);
}

void LandscapeEditorVisibilityCheckTool::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		PerformLandscapeDraw();
		wasTileMaskToolUpdate = false;
	}
}

void LandscapeEditorVisibilityCheckTool::RecreateVisibilityAreaSprite()
{
	uint32 width = 0;
	uint32 height = 0;

	if (visibilityAreaSprite)
	{
		width = visibilityAreaSprite->GetWidth();
		height = visibilityAreaSprite->GetHeight();
	}
	else if (texSurf)
	{
		width = texSurf->GetWidth();
		height = texSurf->GetHeight();
	}

	SafeRelease(visibilityAreaSprite);
	visibilityAreaSprite = Sprite::CreateAsRenderTarget(width, height, FORMAT_RGBA8888);
}

void LandscapeEditorVisibilityCheckTool::PerformLandscapeDraw()
{
	Sprite* visibilityPointSprite = 0;
	if(isVisibilityPointSet)
	{
		visibilityPointSprite = Sprite::CreateFromTexture(pointCursorTexture,
														  0,
														  0,
														  pointCursorTexture->GetWidth(),
														  pointCursorTexture->GetHeight());
	}

	Sprite* sprLandscape = Sprite::CreateFromTexture(texSurf, 0, 0, texSurf->GetWidth(), texSurf->GetHeight());
	Sprite* sprTargetSurf = Sprite::CreateAsRenderTarget(texSurf->GetWidth(), texSurf->GetHeight(), FORMAT_RGBA8888);

    RenderManager::Instance()->SetRenderTarget(sprTargetSurf);
	sprLandscape->Draw();
	
	if(visibilityAreaSprite != 0)
	{
		visibilityAreaSprite->Draw();
	}

	if(visibilityPointSprite != 0)
	{
		float32 scale = VISIBILITY_POINT_CURSOR_SCALE * 2;
		Vector2 scaledSize = visibilityPointSprite->GetSize() * scale;
		visibilityPointSprite->SetPosition(visibilityPoint - scaledSize / 2);
		visibilityPointSprite->SetScale(scale, scale);
		visibilityPointSprite->Draw();
	}
    RenderManager::Instance()->RestoreRenderTarget();

	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, sprTargetSurf->GetTexture());
	
	SafeRelease(visibilityPointSprite);
	SafeRelease(sprTargetSurf);
	SafeRelease(sprLandscape);
	
}

void LandscapeEditorVisibilityCheckTool::UpdateCursor()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scale;
		
		switch(state)
		{
			case VCT_STATE_SET_POINT:
				cursorTexture = pointCursorTexture;
				scale = VISIBILITY_POINT_CURSOR_SCALE *2;
				break;
				
			case VCT_STATE_SET_AREA:
				cursorTexture = areaCursorTexture;
				scale = 2 * (float32)visibilityAreaSize / areaCursorTexture->GetWidth();
				break;

			default:
				scale = 0.f;
				break;
		}

		float32 scaledSize = cursorTexture->GetWidth() * scale;
		Vector2 pos = landscapePoint - Vector2(scaledSize, scaledSize) / 2;

		workingLandscape->SetCursorTexture(cursorTexture);
		workingLandscape->SetBigTextureSize((float32)workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth());
		workingLandscape->SetCursorPosition(pos);
		workingLandscape->SetCursorScale(scaledSize);
	}
}

void LandscapeEditorVisibilityCheckTool::SetVisibilityAreaSize(uint32 size)
{
	visibilityAreaSize = size;
}

void LandscapeEditorVisibilityCheckTool::SetState(eVisibilityCheckToolState newState)
{
	if(state == newState)
		state = VCT_STATE_NORMAL;
	else
		state = newState;

	// TODO: mainwindow
	/*
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	switch(state)
	{
		case VCT_STATE_SET_POINT:
			handler->SetPointButtonStateVisibilityTool(true);
			handler->SetAreaButtonStateVisibilityTool(false);
			break;
			
		case VCT_STATE_SET_AREA:
			handler->SetPointButtonStateVisibilityTool(false);
			handler->SetAreaButtonStateVisibilityTool(true);
			break;
			
		default:
			handler->SetPointButtonStateVisibilityTool(false);
			handler->SetAreaButtonStateVisibilityTool(false);
			break;
	}
	*/
}

void LandscapeEditorVisibilityCheckTool::SaveColorLayer(const FilePath &pathName)
{
	SaveTextureAction(pathName);
}

void LandscapeEditorVisibilityCheckTool::InputAction(int32 phase, bool intersects)
{
	switch(state)
	{
		case VCT_STATE_SET_POINT:
			wasTileMaskToolUpdate = SetPointInputAction(phase);
			break;
			
		case VCT_STATE_SET_AREA:
			wasTileMaskToolUpdate = SetAreaInputAction(phase);
			break;
			
		default:
			break;
	}
}

bool LandscapeEditorVisibilityCheckTool::SetPointInputAction(int32 phase)
{
	bool res = false;
	
	if(phase == UIEvent::PHASE_ENDED)
	{
		Rect landRect(0, 0, visibilityAreaSprite->GetWidth(), visibilityAreaSprite->GetHeight());
		if (landRect.PointInside(landscapePoint))
		{
			CreatePointUndoAction();

			SetState(VCT_STATE_NORMAL);
		}
	}
	
	return res;
}

void LandscapeEditorVisibilityCheckTool::StorePointState(Vector2* point, bool* pointIsSet, Image** image)
{
	*point = visibilityPoint;
	*pointIsSet = isVisibilityPointSet;
	*image = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
}

void LandscapeEditorVisibilityCheckTool::RestorePointState(const DAVA::Vector2 &point, bool pointIsSet, DAVA::Image *image)
{
	visibilityPoint = point;
	isVisibilityPointSet  = pointIsSet;

	RestoreAreaState(image);
}

void LandscapeEditorVisibilityCheckTool::SetVisibilityPoint(const DAVA::Vector2 &point)
{
	Rect landRect(0, 0, visibilityAreaSprite->GetWidth(), visibilityAreaSprite->GetHeight());
	if (landRect.PointInside(visibilityPoint))
	{
		visibilityPoint = point;
		isVisibilityPointSet = true;
		RecreateVisibilityAreaSprite();

		if (IsActive())
			PerformLandscapeDraw();
	}
}

bool LandscapeEditorVisibilityCheckTool::SetAreaInputAction(int32 phase)
{
	bool res = false;
	
	if(phase == UIEvent::PHASE_ENDED)
	{
		if(isVisibilityPointSet)
		{
			Rect landRect(0, 0, visibilityAreaSprite->GetWidth(), visibilityAreaSprite->GetHeight());
			Vector2 visibilityAreaCenter(landscapePoint);

			if(landRect.PointInside(visibilityAreaCenter))
			{
				CreateAreaUndoAction();
			}
		}
		else
		{
			ShowErrorDialog(String("Cannot check visibility without visibility point is set. Please set visibility point"));
		}
	}
	
	return res;
}

Image* LandscapeEditorVisibilityCheckTool::StoreAreaState()
{
	return visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
}

void LandscapeEditorVisibilityCheckTool::RestoreAreaState(Image* image)
{
	if (image)
	{
		Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(),false);
		Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());

		RecreateVisibilityAreaSprite();
		RenderManager::Instance()->SetRenderTarget(visibilityAreaSprite);
		sprite->Draw();
		RenderManager::Instance()->RestoreRenderTarget();

		SafeRelease(sprite);
		SafeRelease(texture);

		if (IsActive())
			PerformLandscapeDraw();
	}
}

void LandscapeEditorVisibilityCheckTool::SetVisibilityArea(const Vector2& visibilityAreaCenter, uint32 visibilityAreaSize)
{
	if (isVisibilityPointSet)
	{
		Rect landRect(0, 0, visibilityAreaSprite->GetWidth(), visibilityAreaSprite->GetHeight());
		if (landRect.PointInside(visibilityAreaCenter))
		{
			Vector3 point(visibilityPoint);
			point.z = GetLandscapeHeightFromCursorPos(visibilityPoint);
			point.z += visibilityPointHeight;

			Vector<Vector3> resP;
			PerformHightTest(point, visibilityAreaCenter, visibilityAreaSize, pointsDensity, areaPointHeights, &resP);
			DrawVisibilityAreaPoints(resP);

			PerformLandscapeDraw();
		}
	}
}

void LandscapeEditorVisibilityCheckTool::DrawVisibilityAreaPoints(const Vector<DAVA::Vector3> &points)
{
	DVASSERT(visibilityAreaSprite != 0);

	RenderManager* manager = RenderManager::Instance();
	RenderHelper* helper = RenderHelper::Instance();

	manager->SetRenderTarget(visibilityAreaSprite);

	for(uint32 i = 0; i < points.size(); ++i)
	{
		uint32 colorIndex = (uint32)points[i].z;
		Vector2 pos(points[i].x, points[i].y);

		Color color(1.f, 0.f, 0.f, 1.f);
		if(areaPointColors.size() > colorIndex)
			color = areaPointColors[colorIndex];

		manager->SetColor(areaPointColors[colorIndex]);
		helper->DrawPoint(pos);
	}

	manager->ResetColor();
	manager->RestoreRenderTarget();
}

void LandscapeEditorVisibilityCheckTool::PrepareConfig()
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

void LandscapeEditorVisibilityCheckTool::PerformHightTest(Vector3 spectatorCoords, Vector2 circleCentr, float circleRadius,
	float density, const Vector<float>& hightValues, Vector<Vector3>* colorizedPoints)
{
	DVASSERT(colorizedPoints);
	if(hightValues.size() == 0 )
	{
		return;
	}
	Vector2 startOfCounting(circleCentr.x - circleRadius, circleCentr.y - circleRadius);
	Vector2 SpectatorCoords2D(spectatorCoords.x, spectatorCoords.y);

	
	// get source point in propper coords system
	Vector3 sourcePoint(ConvertToLanscapeSystem(SpectatorCoords2D)) ;
	
	sourcePoint.z = spectatorCoords.z;
	
	uint32	hight = hightValues.size();
	uint32	sideLength = (circleRadius * 2) / density;
	
	Vector< Vector< Vector< Vector3 > > > points;
	
	for(uint32 layerIndex = 0; layerIndex < hight; ++layerIndex)
	{
		Vector<Vector<Vector3> > xLine;
		for(uint32 x = 0; x < sideLength; ++x)
		{
			float xOfPoint = startOfCounting.x + density * x;
			Vector<Vector3> yLine;
			for(uint32 y = 0; y < sideLength; ++y)
			{
				float yOfPoint = startOfCounting.y + density * y;
				float32 zOfPoint = GetLandscapeHeightFromCursorPos(Vector2(xOfPoint, yOfPoint)) + hightValues[layerIndex];
				Vector3 pointToInsert(xOfPoint, yOfPoint, zOfPoint);
				yLine.push_back(pointToInsert);
			}
			xLine.push_back(yLine);
		}
		points.push_back(xLine);
	}
	
	colorizedPoints->clear();
	parent->GetScene()->JuncCollWorldToLandscapeCollWorld();
	for(uint32 x = 0; x < sideLength; ++x)
	{
		for(uint32 y = 0; y < sideLength; ++y)
		{
			Vector3 target(ConvertToLanscapeSystem( Vector2(points[0][x][y].x,points[0][x][y].y) )) ;

			bool prevRes = false;
			for(int32 layerIndex = hight - 1; layerIndex >= 0; --layerIndex)
			{
				Vector3 targetTmp = points[layerIndex][x][y];
				if(!CheckIsInCircle(circleCentr, circleRadius, Vector2(targetTmp.x, targetTmp.y)))
				{
					continue;
				}
				
				target.z = targetTmp.z;
				bool res = parent->GetScene()->TryIsTargetAccesible(sourcePoint, target);
				float colorIndex =0;
				if(prevRes)
				{
					if(!res)
					{
						colorIndex = (float)layerIndex+2; // +1 - because need layer from prev loop, and +1 - 'cause the first color reserved for  "death zone" 
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
					if(layerIndex == hight - 1 && !res)
					{
						colorIndex = 0;
						Vector3 exportData(targetTmp.x, targetTmp.y, colorIndex );
						colorizedPoints->push_back(exportData);
						break;
					}
				}
				prevRes = res;
			}
		}
	}
	parent->GetScene()->SeparateCollWorldFromLandscapeCollWorld();
}

Vector2 LandscapeEditorVisibilityCheckTool::TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect)
{
	DVASSERT(fromRect.dx != 0 && fromRect.dy != 0);
	
	Vector2 origRectSize = fromRect.GetSize();
	Vector2 destRectSize = toRect.GetSize();
	
	Vector2 scale(destRectSize.x / origRectSize.x,
	     destRectSize.y / origRectSize.y);
	
	Vector2 relPos = point - fromRect.GetPosition();
	Vector2 newRelPos(relPos.x * scale.x,
	      relPos.y * scale.y);
	
	Vector2 newPos = newRelPos + toRect.GetPosition();
	
	return newPos;
}

float32 LandscapeEditorVisibilityCheckTool::GetLandscapeHeightFromCursorPos(const Vector2& point)
{
	Vector3 landscapePoint(ConvertToLanscapeSystem(point));
	
	workingLandscape->PlacePoint(landscapePoint, landscapePoint);

	return landscapePoint.z;
}

Vector2 LandscapeEditorVisibilityCheckTool::ConvertToLanscapeSystem(const Vector2& point)
{
	AABBox3 boundingBox = workingLandscape->GetBoundingBox();
	Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
	Vector2 landSize((boundingBox.max - boundingBox.min).x,
	     (boundingBox.max - boundingBox.min).y);
	
	Rect landRect(landPos, landSize);
	Rect textRect(Vector2(0, 0), visibilityAreaSprite->GetSize());
	
	return TranslatePoint(point, textRect, landRect);
}

bool LandscapeEditorVisibilityCheckTool::GetIntersectionPoint(const DAVA::Vector2 &touchPoint, DAVA::Vector3 &pointOnLandscape)
{
    DVASSERT(parent);
	
    Vector3 from, dir;
    parent->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * 1000;
	
    bool isIntersect = workingScene->LandscapeIntersection(from, to, pointOnLandscape);
    return isIntersect;
}

bool LandscapeEditorVisibilityCheckTool::CheckIsInCircle(Vector2 circleCentre, float radius, Vector2 targetCoord)
{
	return (targetCoord - circleCentre).Length() < radius;
}

void LandscapeEditorVisibilityCheckTool::HideAction()
{
	if(!IsActive())
	{
		return;
	}
	workingLandscape->CursorDisable();
	workingLandscape->SetFog(isFogEnabled);

	SafeRelease(texSurf);
	
	SetState(VCT_STATE_NORMAL);
	// TODO: mainwindow
	//QtMainWindowHandler::Instance()->SetWidgetsStateVisibilityTool(false);
}

void LandscapeEditorVisibilityCheckTool::ShowAction()
{
	if(pointCursorTexture == NULL)
	{
		pointCursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/setPointCursor.png");
		pointCursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
	}
	SetState(VCT_STATE_NORMAL);

	PrepareConfig();

    landscapeSize = GetLandscape()->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	workingLandscape->CursorEnable();

	//save fog status and disable it for more convenience
	isFogEnabled = workingLandscape->IsFogEnabled();
	workingLandscape->SetFog(false);

	texSurf = SafeRetain( workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));

	PerformLandscapeDraw();
	if(visibilityAreaSprite == 0)
		RecreateVisibilityAreaSprite();

	// TODO: mainwindow
	//QtMainWindowHandler::Instance()->SetWidgetsStateVisibilityTool(true);
}

void LandscapeEditorVisibilityCheckTool::SaveTextureAction(const FilePath &pathToFile)
{
    if(visibilityAreaSprite)
    {
		Sprite* saveSprite = Sprite::CreateAsRenderTarget(visibilityAreaSprite->GetWidth(),
														  visibilityAreaSprite->GetHeight(),
														  FORMAT_RGBA8888);

		if(!saveSprite)
			return;

		Sprite* cursorSprite = Sprite::CreateFromTexture(pointCursorTexture,
														 0,
														 0,
														 pointCursorTexture->GetWidth(),
														 pointCursorTexture->GetHeight());

		RenderManager::Instance()->SetRenderTarget(saveSprite);
		visibilityAreaSprite->Draw();
		if(cursorSprite != 0)
		{
			float32 scale = VISIBILITY_POINT_CURSOR_SCALE * 2;
			Vector2 scaledSize = cursorSprite->GetSize() * scale;
			cursorSprite->SetPosition(visibilityPoint - scaledSize / 2);
			cursorSprite->SetScale(scale, scale);
			cursorSprite->Draw();
		}
		RenderManager::Instance()->RestoreRenderTarget();

        Image *img = saveSprite->GetTexture()->CreateImageFromMemory();
        if(img)
        {
			ImageLoader::Save(img, pathToFile);
            SafeRelease(img);
        }
		
		SafeRelease(cursorSprite);
		SafeRelease(saveSprite);
    }
}

NodesPropertyControl *LandscapeEditorVisibilityCheckTool::GetPropertyControl(const Rect &rect)
{
    LandscapeEditorPropertyControl *propsControl =
		(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingScene, rect, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);
	workingLandscape->SetTiledShaderMode(Landscape::TILED_MODE_TEXTURE);
    propsControl->SetDelegate(this);
    LandscapeEditorSettingsChanged(propsControl->Settings());
    return propsControl;
}

void LandscapeEditorVisibilityCheckTool::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

void LandscapeEditorVisibilityCheckTool::TextureWillChanged(const String &forKey)
{}

void LandscapeEditorVisibilityCheckTool::TextureDidChanged(const String &forKey)
{}

void LandscapeEditorVisibilityCheckTool::RecreateHeightmapNode()
{
    if(workingScene && heightmapNode)
    {
        workingScene->RemoveNode(heightmapNode);
    }
        
    SafeRelease(heightmapNode);
    heightmapNode = new HeightmapNode(workingScene, workingLandscape);
    workingScene->AddNode(heightmapNode);
}

bool LandscapeEditorVisibilityCheckTool::SetScene(EditorScene *newScene)
{
    EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(FindLandscape(newScene));
    if(editorLandscape)
    {
        ShowErrorDialog(String("Cannot start Visibility Check Tool. Remove EditorLandscape from scene"));
        return false;
    }
    
    return LandscapeEditorBase::SetScene(newScene);
}

Rect2i LandscapeEditorVisibilityCheckTool::FitRectToImage(DAVA::Image *image, const Rect2i& rect)
{
	Point2i rectPos(Max(rect.x, 0),
					Max(rect.y, 0));
	Point2i rectBottomRight(Min((uint32)rect.x + (uint32)rect.dx, image->GetWidth()),
							Min((uint32)rect.y + (uint32)rect.dy, image->GetHeight()));
	Point2i rectSize = rectBottomRight - rectPos;

	Rect2i newRect;
	newRect.SetSize(Size2i(rectSize.x, rectSize.y));
	newRect.SetPosition(rectPos);
	
	return newRect;
}

void LandscapeEditorVisibilityCheckTool::CopyImageRectToImage(Image* imageFrom, const Rect2i& rectFrom, Image* imageTo, const Point2i& pos)
{
	DVASSERT(imageFrom && imageTo && imageTo->GetPixelFormat() == imageFrom->GetPixelFormat());

	uint32 fromWidth = rectFrom.dx;
	uint32 fromHeight = rectFrom.dy;
	uint32 fromX = rectFrom.x;
	uint32 fromY = rectFrom.y;
	uint32 toX = pos.x;
	uint32 toY = pos.y;
	uint32 pixelSize = Texture::GetPixelFormatSizeInBytes(imageFrom->GetPixelFormat());

	uint8* imageFromData = imageFrom->GetData();
	uint8* imageToData = imageTo->GetData();

	for(uint32 i = 0; i < fromHeight; ++i)
	{
		uint8* imageFromRow = &(imageFromData[imageFrom->GetWidth() * pixelSize * (fromY + i) + fromX * pixelSize]);
		uint8* imageToRow = &(imageToData[imageTo->GetWidth() * pixelSize * (toY + i) + toX * pixelSize]);

		memcpy(imageToRow, imageFromRow, fromWidth * pixelSize);
	}
}

void LandscapeEditorVisibilityCheckTool::ClearSceneResources()
{
	LandscapeEditorBase::ClearSceneResources();
	SafeRelease(visibilityAreaSprite);
	SafeRelease(pointCursorTexture);
	visibilityPoint.x = 0;
	visibilityPoint.y = 0;
}

void LandscapeEditorVisibilityCheckTool::UpdateLandscapeTilemap(Texture* texture)
{
	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, texSurf);
	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
	workingLandscape->UpdateFullTiledTexture();

	Image* image = workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory();
	ImageLoader::Save(image, texture->GetPathname());
	SafeRelease(image);

	SafeRelease(texSurf);

	texSurf = SafeRetain(workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));
	wasTileMaskToolUpdate = true;
}

void LandscapeEditorVisibilityCheckTool::CreatePointUndoAction()
{
	Image* areaImage = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
	CommandsManager::Instance()->ExecuteAndRelease(new CommandPlacePointVisibilityTool(landscapePoint,
																					   visibilityPoint,
																					   isVisibilityPointSet,
																					   areaImage),
												   workingScene);
	SafeRelease(areaImage);
}

void LandscapeEditorVisibilityCheckTool::CreateAreaUndoAction()
{
	Image* areaImage = visibilityAreaSprite->GetTexture()->CreateImageFromMemory();
	CommandsManager::Instance()->ExecuteAndRelease(new CommandPlaceAreaVisibilityTool(landscapePoint,
																					  visibilityAreaSize,
																					  areaImage),
												   workingScene);
	SafeRelease(areaImage);
}
