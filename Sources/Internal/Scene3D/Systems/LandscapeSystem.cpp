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


#include "LandscapeSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/Landscape.h"
#include "Math/Math2D.h"

namespace DAVA
{
LandscapeSystem::LandscapeSystem(Scene* scene)
    : SceneSystem(scene)
{
}

LandscapeSystem::~LandscapeSystem()
{
    DVASSERT(landscapeEntities.size() == 0);
}

void LandscapeSystem::AddEntity(Entity* entity)
{
    if (GetLandscape(entity))
        landscapeEntities.push_back(entity);
}

void LandscapeSystem::RemoveEntity(Entity* entity)
{
    uint32 eCount = static_cast<uint32>(landscapeEntities.size());
    for (uint32 e = 0; e < eCount; ++e)
    {
        if (landscapeEntities[e] == entity)
        {
            RemoveExchangingWithLast(landscapeEntities, e);
            break;
        }
    }
}

void LandscapeSystem::Process(float32 timeElapsed)
{
    TIME_PROFILE("LandscapeSystem::Process");

    for (Entity* e : landscapeEntities)
    {
        Landscape* landscapeObject = GetLandscape(e);
        if (landscapeObject->debugDrawMetrics)
            DrawPatchMetrics(landscapeObject, 0, 0, 0);
    }
}

void LandscapeSystem::DrawPatchMetrics(Landscape* landscape, uint32 level, uint32 x, uint32 y)
{
    Landscape::SubdivisionLevelInfo& levelInfo = landscape->subdivLevelInfoArray[level];
    Landscape::SubdivisionPatchInfo* subdivPatchInfo = &landscape->subdivPatchArray[levelInfo.offset + (y << level) + x];

    uint32 state = subdivPatchInfo->subdivisionState;
    if (state == Landscape::SubdivisionPatchInfo::CLIPPED)
        return;

    if (state == Landscape::SubdivisionPatchInfo::SUBDIVIDED)
    {
        uint32 x2 = x * 2;
        uint32 y2 = y * 2;

        DrawPatchMetrics(landscape, level + 1, x2 + 0, y2 + 0);
        DrawPatchMetrics(landscape, level + 1, x2 + 1, y2 + 0);
        DrawPatchMetrics(landscape, level + 1, x2 + 0, y2 + 1);
        DrawPatchMetrics(landscape, level + 1, x2 + 1, y2 + 1);
    }
    else
    {
        Landscape::PatchQuadInfo* patch = &landscape->patchQuadArray[levelInfo.offset + (y << level) + x];

        Camera* camera = GetScene()->GetRenderSystem()->GetMainCamera();
        float32 tanFovY = tanf(camera->GetFOV() * PI / 360.f) / camera->GetAspect();

        float32 distance = Distance(camera->GetPosition(), patch->positionOfMaxError);
        float32 hError = Abs(patch->maxError) / (distance * tanFovY);

        Vector3 patchOrigin = patch->bbox.GetCenter();
        float32 patchDistance = Distance(camera->GetPosition(), patchOrigin);
        float32 rError = patch->radius / (patchDistance * tanFovY);

        float32 rErrorRel = Min(rError / landscape->maxPatchRadiusError, 1.f);
        float32 hErrorRel = Min(hError / landscape->maxHeightError, 1.f);

        RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
        Color color;
        if (rErrorRel > hErrorRel)
        {
            color = Color(0.f, 0.f, 1.f, 1.f);
            drawer->DrawLine(patch->bbox.GetCenter(), patch->bbox.GetCenter() + Vector3(0.f, 0.f, patch->radius), color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        }
        else
        {
            color = Color(1.f, 0.f, 0.f, 1.f);
            drawer->DrawLine(patch->positionOfMaxError - Vector3(0.f, 0.f, patch->maxError), patch->positionOfMaxError, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
            float32 arrowToHeight = Max(patch->positionOfMaxError.z, patch->positionOfMaxError.z - patch->maxError) + patch->radius * .05f;
            Vector3 arrowTo = Vector3(patch->positionOfMaxError.x, patch->positionOfMaxError.y, arrowToHeight);
            drawer->DrawArrow(arrowTo + Vector3(0.f, 0.f, patch->radius * .2f), arrowTo, patch->radius * .05f, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        }

        float32 bboxMiddleH = (patch->bbox.min.z + patch->bbox.max.z) / 2.f;
        Vector3 p0(patch->bbox.min.x, patch->bbox.min.y, bboxMiddleH);
        Vector3 e1(patch->bbox.max.x - patch->bbox.min.x, 0.f, 0.f);
        Vector3 e2(0.f, patch->bbox.max.y - patch->bbox.min.y, 0.f);

        drawer->DrawLine(p0, p0 + e1, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(p0, p0 + e2, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(p0 + e1, p0 + e1 + e2, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(p0 + e2, p0 + e1 + e2, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }
}
};