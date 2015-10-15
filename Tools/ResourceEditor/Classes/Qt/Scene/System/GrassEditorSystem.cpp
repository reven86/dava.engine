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


#include "GrassEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/GrassEditorProxy.h"
#include "Scene/SceneSignals.h"
#include "Commands2/ImageRegionCopyCommand.h"

#include "Render/PixelFormatDescriptor.h"

#include <QApplication>

GrassEditorSystem::GrassEditorSystem(Scene* scene)
: LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/squareCursor.tex")
, inDrawState(false)
, curBrushMode(BRUSH_REPLACE)
, curBrushAffect(AFFECT_ALL)
, curHeight(8)
, curDensity(8)
, curLayer(0)
, vegetationMap(NULL)
, vegetationMapCopy(NULL)
, curVegetation(NULL)
{
}

GrassEditorSystem::~GrassEditorSystem()
{
    SafeRelease(vegetationMap);
    SafeRelease(vegetationMapCopy);
    SafeRelease(curVegetation);
}

void GrassEditorSystem::Update(DAVA::float32 timeElapsed)
{ }

void GrassEditorSystem::Input(DAVA::UIEvent *event)
{
    if(enabled && NULL != vegetationMap)
    {
        UpdateCursorPos();

        if (event->tid == UIEvent::BUTTON_1)
	    {
		    Vector3 point;

		    switch(event->phase)
		    {
            case UIEvent::Phase::BEGAN:
                    {
                        inDrawState = true;
                        affectedArea.Empty();
                        if(curCursorPos.x >= 0 && curCursorPos.y >= 0)
                        {
                            // make lastDrawPos not equal to the curCursorPos
                            lastDrawPos = curCursorPos;
                            lastDrawPos.x += 1;

                            // draw
                            affectedArea.AddPoint(curCursorPos);
                            DrawGrass(curCursorPos);
                        }
                    }
				    break;

                    case UIEvent::Phase::DRAG:
                    if(inDrawState)
                    {
                        if(curCursorPos.x >= 0 && curCursorPos.y >= 0)
                        {
                            affectedArea.AddPoint(curCursorPos);
                            DrawGrass(curCursorPos);
                        }
                    }
				    break;

                    case UIEvent::Phase::ENDED:
                    if(inDrawState)
                    {
                        inDrawState = false;
                        DrawGrassEnd();
                    }
				    break;
		    }
	    }
    }
}

void GrassEditorSystem::ProcessCommand(const Command2 *command, bool redo)
{
    if(NULL != command)
    {
        int cmdId = command->GetId();
        if(cmdId == CMDID_IMAGE_REGION_COPY)
        {
            ImageRegionCopyCommand* imCmd = (ImageRegionCopyCommand *) command;
            BuildGrassCopy(DAVA::AABBox2(imCmd->pos, DAVA::Vector2(imCmd->pos.x + imCmd->orig->width - 1, imCmd->pos.y + imCmd->orig->height - 1)));
        }
    }
}

bool GrassEditorSystem::EnableGrassEdit(bool enable)
{
    bool ret = false;

    /*if(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == drawSystem->VerifyLandscape())
    {
        if(enable != isEnabled)
        {
            if(enable)
            {
                DAVA::VegetationRenderObject *veg = SearchVegetation(GetScene());

                DVASSERT(NULL == curVegetation);
                curVegetation = SafeRetain(veg);

                if(NULL != veg)
                {
                    isEnabled = true;
                    ret = true;

                    selectionSystem->SetLocked(true);
                    modifSystem->SetLocked(true);

                    drawSystem->EnableCursor(vegetationMap->width);
                    drawSystem->SetCursorTexture(cursorTexture);
                    drawSystem->SetCursorSize(1);

                    BuildGrassCopy();
                }
            }
            else
            {
                selectionSystem->SetLocked(false);
                modifSystem->SetLocked(false);

                drawSystem->DisableCursor();
                drawSystem->DisableCustomDraw();

                if(NULL != curVegetation)
                {
                    curVegetation->SetLayerVisibilityMask(0xFF);
                    
                }

                SafeRelease(vegetationMap);
                SafeRelease(vegetationMapCopy);
                SafeRelease(curVegetation);

                isEnabled = false;
                ret = true;
            }
        }
    }*/

    return ret;
}

void GrassEditorSystem::UpdateCursorPos()
{
    Vector3 landPos;
    if(NULL != vegetationMap && collisionSystem->LandRayTestFromCamera(landPos))
    {
        Vector2 point(landPos.x, landPos.y);
        AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

        curCursorPos.x = (point.x - box.min.x) * vegetationMap->width / (box.max.x - box.min.x);
        curCursorPos.y = (point.y - box.min.y) * vegetationMap->height / (box.max.y - box.min.y);
        curCursorPos.x = (int32) curCursorPos.x;
        curCursorPos.y = (int32) curCursorPos.y;

        drawSystem->SetCursorPosition(curCursorPos);
    }
    else
    {
        curCursorPos = DAVA::Vector2(-100, -100);
    }

    drawSystem->SetCursorPosition(curCursorPos);
}

DAVA::VegetationRenderObject* GrassEditorSystem::SearchVegetation(DAVA::Entity *entity) const
{
    DAVA::VegetationRenderObject* ret = DAVA::GetVegetation(entity);

    if(NULL == ret)
    {
        for(int i = 0; NULL == ret && i < entity->GetChildrenCount(); ++i)
        {
            ret = SearchVegetation(entity->GetChild(i));
        }
    }

    return ret;
}

void GrassEditorSystem::SetLayerVisible(uint8 layer, bool visible)
{
    if(NULL != curVegetation)
    {
        DAVA::uint8 mask =  curVegetation->GetLayerVisibilityMask();

        if(visible)
        {
            mask |= (1 << layer);
        }
        else
        {
            mask &= (~(1 << layer));
        }

        curVegetation->SetLayerVisibilityMask(mask);
    }
}

bool GrassEditorSystem::IsLayerVisible(uint8 layer) const
{
    DAVA::uint8 mask = curVegetation->GetLayerVisibilityMask();
    return (0 != (mask & (1 << layer)));
}

void GrassEditorSystem::SetBrushMode(int mode)
{
    curBrushMode = mode;
}

int GrassEditorSystem::GetBrushMode() const
{
    return curBrushMode;
}

void GrassEditorSystem::SetBrushAffect(int affect)
{
    curBrushAffect = affect;
}

int GrassEditorSystem::GetBrushAffect() const
{
    return curBrushAffect;
}

void GrassEditorSystem::SetCurrentLayer(uint8 layer)
{
    DVASSERT(layer < 4);

    curLayer = layer;
}

uint8 GrassEditorSystem::GetCurrentLayer() const
{
    return curLayer;
}

void GrassEditorSystem::SetBrushHeight(uint8 height)
{
    curHeight = height;
}

uint8 GrassEditorSystem::GetBrushHeight() const
{
    return curHeight;
}

void GrassEditorSystem::SetBrushDensity(uint8 density)
{
    curDensity = density;
}

uint8 GrassEditorSystem::GetBrushDensity() const
{
    return curDensity;
}

void GrassEditorSystem::DrawGrass(DAVA::Vector2 pos)
{
    if(NULL != vegetationMap && pos != lastDrawPos && curBrushAffect != AFFECT_NONE)
    {
        DVASSERT(pos.x < vegetationMap->width);
        DVASSERT(pos.y < vegetationMap->height);

        int curKeyModifiers = QApplication::keyboardModifiers();

        DAVA::uint32 offset = (pos.y * vegetationMap->width + pos.x);
        DAVA::uint8 applyFromLayer = curLayer;
        DAVA::uint8 applyToLayer = curLayer;

        if(curKeyModifiers & Qt::ControlModifier)
        {
            applyFromLayer = 0;
            applyToLayer = 3;
        }

        for(DAVA::uint8 layer = applyFromLayer; layer <= applyToLayer; ++layer)
        {
            DAVA::uint32 index = offset * DAVA::PixelFormatDescriptor::GetPixelFormatSizeInBytes(vegetationMap->format);
            index += layer;

            uint8 mapData = vegetationMap->data[index];
            uint8 h = mapData >> 4;
            uint8 d = mapData & 0xf;

            bool opPlus = true;

            // when shift is pressed we should subtract brush from map
            if(curKeyModifiers & Qt::ShiftModifier)
            {
                opPlus = false;
            }

            if(curBrushAffect & AFFECT_DENSITY)
            {
                if(curBrushMode & BRUSH_ADD_DENSITY)
                {
                    if(opPlus)
                    {
                        d += curDensity;
                        if(d > 0xf) d = 0xf;
                    }
                    else
                    {
                        d -= curDensity;
                        if(d > 0xf) d = 0;
                    }
                }
                else
                {
                    d = curDensity;
                }
            }

            if(curBrushAffect & AFFECT_HEIGHT)
            {
                if(curBrushMode & BRUSH_ADD_HEIGHT)
                {
                    if(opPlus)
                    {
                        h += curHeight;
                        if(h > 0xf) h = 0xf;
                    }
                    else
                    {
                        h -= curHeight;
                        if(h > 0xf) h = 0;
                    }
                }
                else
                {
                    h = curHeight;
                }
            }

            mapData = ((h << 4) | d);
            vegetationMap->data[index] = mapData;
        }

        lastDrawPos = pos;
    }
}

void GrassEditorSystem::DrawGrassEnd()
{
    /*if(!affectedArea.IsEmpty() && NULL != vegetationMapCopy)
    {
        SceneEditor2 *sceneEditor = (SceneEditor2 *) GetScene();

        DAVA::Rect2i affectedRect2i = GetAffectedImageRect(affectedArea);
        DAVA::Rect affectedRect = DAVA::Rect(affectedRect2i.x, affectedRect2i.y, affectedRect2i.dx, affectedRect2i.dy);
        DAVA::Image *orig = DAVA::Image::CopyImageRegion(vegetationMapCopy, affectedRect);
        sceneEditor->Exec(new ImageRegionCopyCommand(vegetationMap, affectedRect.GetPosition(), vegetationMap, affectedRect, curVegetation->GetVegetationMapPath(), orig));
    }*/
}

void GrassEditorSystem::BuildGrassCopy(DAVA::AABBox2 area)
{
    if(NULL != vegetationMap)
    {
        if(NULL == vegetationMapCopy)
        {
            vegetationMapCopy = DAVA::Image::CreateFromData(vegetationMap->width, vegetationMap->height, vegetationMap->format, vegetationMap->data);
        }
        else
        {
            DAVA::Rect2i r2i = GetAffectedImageRect(area);
            DAVA::Rect r(r2i.x, r2i.y, r2i.dx, r2i.dy);

            vegetationMapCopy->InsertImage(vegetationMap, r.GetPosition(), r);
        }
    }
}

DAVA::VegetationRenderObject* GrassEditorSystem::GetCurrentVegetationObject() const
{
    return curVegetation;
}

DAVA::Rect2i GrassEditorSystem::GetAffectedImageRect(DAVA::AABBox2 &area)
{
    DAVA::Rect2i ret;

    if(!area.IsEmpty() && area.max.x >= area.min.x && area.max.y >= area.min.y)
    {
        int x = (int) floor(area.min.x);
        int y = (int) floor(area.min.y);
        int dx = ((int) ceil(area.max.x)) - x + 1;
        int dy = ((int) ceil(area.max.y)) - y + 1;

        ret = Rect2i(x, y, dx, dy);
    }

    return ret;
}