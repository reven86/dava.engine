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
    drawSystem->GetLandscapeProxy()->SetToolTexture(rulerToolTexture, false);
    landscapeSize = drawSystem->GetHeightmapProxy()->Size();

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

	drawSystem->DisableCustomDraw();

    drawSystem->GetLandscapeProxy()->SetToolTexture(nullptr, false);

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

    Vector3 point3;
    collisionSystem->LandRayTestFromCamera(point3);
    Vector2 point(point3.x, point3.y);

    switch ( event->phase )
    {
    case UIEvent::PHASE_KEYCHAR:
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

    case UIEvent::PHASE_MOVE:
        if ( previewEnabled )
        {
            CalcPreviewPoint( point );
            DrawPoints();
        }
        break;

    case UIEvent::PHASE_ENDED:
        if ( event->tid == UIEvent::BUTTON_1 && isIntersectsLandscape )
        {
            if ( IsKeyModificatorPressed( DVKEY_SHIFT ) )
            {
                SetStartPoint(point);
            }
            else
            {
                if ( previewEnabled )
                {
                    AddPoint(point);
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

void RulerToolSystem::SetStartPoint(const DAVA::Vector2& point)
{
	Clear();

	previewPoint = point;
	linePoints.push_back(point);
	lengths.push_back(0.f);
	SendUpdatedLength();
}

void RulerToolSystem::AddPoint(const DAVA::Vector2& point)
{
	if(0 < linePoints.size())
	{
        Vector2 prevPoint = *(linePoints.rbegin());
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
        List<Vector2>::iterator pointsIter = linePoints.end();
        --pointsIter;
		linePoints.erase(pointsIter);

		List<float32>::iterator lengthsIter = lengths.end();
		--lengthsIter;
		lengths.erase(lengthsIter);

		SendUpdatedLength();
	}
}

void RulerToolSystem::CalcPreviewPoint(const Vector2& point, bool force)
{
	if (!previewEnabled)
	{
		return;
	}

	if ((isIntersectsLandscape && linePoints.size() > 0) && (force || previewPoint != point))
	{
        Vector2 lastPoint = linePoints.back();
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

DAVA::float32 RulerToolSystem::GetLength(const DAVA::Vector2& startPoint, const DAVA::Vector2& endPoint)
{
	float32 lineSize = 0.f;

    Vector3 prevPoint = Vector3(startPoint);
    Vector3 prevLandscapePoint = drawSystem->GetLandscapeProxy()->PlacePoint(prevPoint); //

	for(int32 i = 1; i <= APPROXIMATION_COUNT; ++i)
	{
        Vector3 point = Vector3(startPoint + (endPoint - startPoint) * i / (float32)APPROXIMATION_COUNT);
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

    RenderSystem2D::Instance()->BeginRenderTargetPass(targetTexture);

    Vector<Vector2> points;
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

		const AABBox3 & boundingBox = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();
		const Vector3 landSize = boundingBox.max - boundingBox.min;
        Vector2 offsetPoint = Vector2(boundingBox.min.x, boundingBox.min.y);

        float32 koef = (float32)targetTexture->GetWidth() / landSize.x;

        Vector2 startPoint = points[0];
        for (uint32 i = 1; i < pointsCount; ++i)
		{
            Vector2 endPoint = points[i];

            Vector2 startPosition = (startPoint - offsetPoint) * koef;
            Vector2 endPosition = (endPoint - offsetPoint) * koef;

            if (previewEnabled && isIntersectsLandscape && i == (points.size() - 1))
            {
                RenderSystem2D::Instance()->DrawLine(startPosition, endPosition, blue);
            }
            else
            {
                RenderSystem2D::Instance()->DrawLine(startPosition, endPosition, red);
            }

            startPoint = endPoint;
		}
	}

    RenderSystem2D::Instance()->EndRenderTargetPass();
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

Vector2 RulerToolSystem::MirrorPoint(const Vector2& point) const
{
    const AABBox3 & boundingBox = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

    Vector2 newPoint = point;
    newPoint.y = (boundingBox.max.y - point.y) + boundingBox.min.y;
    
    return newPoint;
}
