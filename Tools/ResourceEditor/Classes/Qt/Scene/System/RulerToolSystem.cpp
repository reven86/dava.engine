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


#include "RulerToolSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "../SceneEditor2.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/RulerToolProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "../Qt/Main/QtUtils.h"
#include "../SceneSignals.h"

RulerToolSystem::RulerToolSystem(Scene* scene)
:	LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
,	curToolSize(0)
,	lineWidth(1)
,	previewEnabled(true)
{
}

RulerToolSystem::~RulerToolSystem()
{
}

LandscapeEditorDrawSystem::eErrorType RulerToolSystem::EnableLandscapeEditing()
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

	Texture* rulerToolTexture = drawSystem->GetRulerToolProxy()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetRulerToolTexture(rulerToolTexture);
	drawSystem->GetLandscapeProxy()->SetRulerToolTextureEnabled(true);
	landscapeSize = drawSystem->GetHeightmapProxy()->Size();

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorSize(0);

	previewLength = -1.f;
	previewEnabled = true;

	Clear();
	DrawPoints();

	SendUpdatedLength();

	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool RulerToolSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);

	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();

	drawSystem->GetLandscapeProxy()->SetRulerToolTexture(NULL);
	drawSystem->GetLandscapeProxy()->SetRulerToolTextureEnabled(false);

	Clear();
	previewLength = -1.f;
	SendUpdatedLength();

	enabled = false;
	return !enabled;
}


void RulerToolSystem::Process(DAVA::float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
}

void RulerToolSystem::Input(DAVA::UIEvent *event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}

	UpdateCursorPosition();

	Vector3 point;
	collisionSystem->LandRayTestFromCamera(point);

    switch ( event->phase )
    {
    case UIEvent::Phase::KEY_DOWN:
        if ( DVKEY_BACKSPACE == event->tid )
        {
            RemoveLastPoint();
            previewEnabled = true;
            CalcPreviewPoint( point, true );
        }
        else if ( DVKEY_ESCAPE == event->tid )
        {
            previewEnabled = false;
        }
        DrawPoints();
        break;

    case UIEvent::Phase::MOVE:
        if ( previewEnabled )
        {
            CalcPreviewPoint( point );
            DrawPoints();
        }
        break;

    case UIEvent::Phase::ENDED:
        if ( event->tid == UIEvent::BUTTON_1 && isIntersectsLandscape )
        {
            if ( IsKeyModificatorPressed( DVKEY_SHIFT ) )
            {
                SetStartPoint( Vector3( point ) );
            }
            else
            {
                if ( previewEnabled )
                {
                    AddPoint( Vector3( point ) );
                }
            }

            previewEnabled = true;
            CalcPreviewPoint( point );
            DrawPoints();
        }
        break;

    default:
        break;
    }
}

void RulerToolSystem::SetStartPoint(const DAVA::Vector3 &point)
{
	Clear();

	previewPoint = point;
	linePoints.push_back(point);
	lengths.push_back(0.f);
	SendUpdatedLength();
}

void RulerToolSystem::AddPoint(const DAVA::Vector3 &point)
{
	if(0 < linePoints.size())
	{
		Vector3 prevPoint = *(linePoints.rbegin());
		float32 l = lengths.back();
		l += GetLength(prevPoint, point);

		linePoints.push_back(point);
		lengths.push_back(l);

		SendUpdatedLength();
	}
}

void RulerToolSystem::RemoveLastPoint()
{
	//remove points except start point
	if (linePoints.size() > 1)
	{
		List<Vector3>::iterator pointsIter = linePoints.end();
		--pointsIter;
		linePoints.erase(pointsIter);

		List<float32>::iterator lengthsIter = lengths.end();
		--lengthsIter;
		lengths.erase(lengthsIter);

		SendUpdatedLength();
	}
}

void RulerToolSystem::CalcPreviewPoint(const Vector3& point, bool force)
{
	if (!previewEnabled)
	{
		return;
	}

	if ((isIntersectsLandscape && linePoints.size() > 0) && (force || previewPoint != point))
	{
		Vector3 lastPoint = linePoints.back();
		float32 previewLen = GetLength(lastPoint, point);

		previewPoint = point;
		previewLength = lengths.back() + previewLen;
	}
	else if (!isIntersectsLandscape)
	{
		previewLength = -1.f;
	}
	SendUpdatedLength();
}

DAVA::float32 RulerToolSystem::GetLength(const DAVA::Vector3 &startPoint, const DAVA::Vector3 &endPoint)
{
	float32 lineSize = 0.f;

	Vector3 prevPoint = startPoint;
	Vector3 prevLandscapePoint = drawSystem->GetLandscapeProxy()->PlacePoint(prevPoint); //

	for(int32 i = 1; i <= APPROXIMATION_COUNT; ++i)
	{
		Vector3 point = startPoint + (endPoint - startPoint) * i / (float32)APPROXIMATION_COUNT;
		Vector3 landscapePoint = drawSystem->GetLandscapeProxy()->PlacePoint(point); //

		lineSize += (landscapePoint - prevLandscapePoint).Length();

		prevPoint = point;
		prevLandscapePoint = landscapePoint;
	}

	return lineSize;
}

void RulerToolSystem::DrawPoints()
{
	if (!drawSystem->GetRulerToolProxy())
	{
		return;
	}

    Texture* targetTexture = drawSystem->GetRulerToolProxy()->GetTexture();

    RenderHelper::Instance()->Set2DRenderTarget(targetTexture);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

	Vector<Vector3> points;
	points.reserve(linePoints.size() + 1);
	std::copy(linePoints.begin(), linePoints.end(), std::back_inserter(points));

	if (previewEnabled && isIntersectsLandscape)
	{
		points.push_back(previewPoint);
	}
    
    const uint32 pointsCount = points.size();
	if(pointsCount > 1)
	{
        for(uint32 i = 0; i < pointsCount; ++i)
        {
            points[i] = MirrorPoint(points[i]);
        }
        
		Color red(1.0f, 0.0f, 0.0f, 1.0f);
		Color blue(0.f, 0.f, 1.f, 1.f);
		RenderManager::Instance()->SetColor(red);

		const AABBox3 & boundingBox = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();
		const Vector3 landSize = boundingBox.max - boundingBox.min;
		Vector3 offsetPoint = boundingBox.min;

        float32 koef = (float32)targetTexture->GetWidth() / landSize.x;

		Vector3 startPoint = points[0];
		for (uint32 i = 1; i < pointsCount; ++i)
		{
			if (previewEnabled && isIntersectsLandscape && i == (points.size() - 1))
			{
				RenderManager::Instance()->SetColor(blue);
			}

			Vector3 endPoint = points[i];

			Vector3 startPosition = (startPoint - offsetPoint) * koef;
			Vector3 endPosition = (endPoint - offsetPoint) * koef;
            
			RenderHelper::Instance()->DrawLine(DAVA::Vector3(startPosition.x, startPosition.y, 0),
											   DAVA::Vector3(endPosition.x, endPosition.y, 0),
											   (float32)lineWidth,
                                               DAVA::RenderState::RENDERSTATE_2D_BLEND);

			startPoint = endPoint;
		}
	}

    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->SetRenderTarget(0);
    
	drawSystem->GetRulerToolProxy()->UpdateTexture();
}

void RulerToolSystem::Clear()
{
	linePoints.clear();
	lengths.clear();
}

void RulerToolSystem::DisablePreview()
{
	previewEnabled = false;
	previewLength = -1.f;

	SendUpdatedLength();
}

void RulerToolSystem::SendUpdatedLength()
{
	float32 length = GetLength();
	float32 previewLength = GetPreviewLength();

	SceneSignals::Instance()->EmitRulerToolLengthChanged(dynamic_cast<SceneEditor2*>(GetScene()),
														 length, previewLength);
}

void RulerToolSystem::SetLineWidth(int32 width)
{
	if (width > 0)
	{
		lineWidth = width;
	}

	DrawPoints();
}

int32 RulerToolSystem::GetLineWidth()
{
	return lineWidth;
}

float32 RulerToolSystem::GetLength()
{
	float32 length = -1.f;
	if (lengths.size() > 0)
	{
		length = lengths.back();
	}

	return length;
}

float32 RulerToolSystem::GetPreviewLength()
{
	float32 previewLength = -1.f;
	if (previewEnabled)
	{
		previewLength = this->previewLength;
	}

	return previewLength;
}

Vector3 RulerToolSystem::MirrorPoint(const Vector3 & point) const
{
    const AABBox3 & boundingBox = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

    Vector3 newPoint = point;
    newPoint.y = (boundingBox.max.y - point.y) + boundingBox.min.y;
    
    return newPoint;
}
