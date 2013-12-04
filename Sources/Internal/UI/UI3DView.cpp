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



#include "UI/UI3DView.h"
#include "Scene3D/Scene.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Core/Core.h"

namespace DAVA 
{


UI3DView::UI3DView(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
    ,   scene(0)
{

}

UI3DView::~UI3DView()
{
    SafeRelease(scene);
}


void UI3DView::SetScene(Scene * _scene)
{
    SafeRelease(scene);
    
    scene = SafeRetain(_scene);
    
    if(scene)
    {
        float32 aspect = size.dx / size.dy;
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

Scene * UI3DView::GetScene() const
{
    return scene;
}

void UI3DView::AddControl(UIControl *control)
{
    DVASSERT(0 && "UI3DView do not support children");
}

    
void UI3DView::Update(float32 timeElapsed)
{
    if (scene)
        scene->Update(timeElapsed);
}

void UI3DView::Draw(const UIGeometricData & geometricData)
{

    
    const Rect & viewportRect = geometricData.GetUnrotatedRect();
    viewportRc = viewportRect;
    
        //glViewport(viewportRect.x, viewportRect.y, viewportRect.dx, viewportRect.dy);
    RenderManager::Instance()->PushDrawMatrix();
	RenderManager::Instance()->PushMappingMatrix();
    Matrix4 modelViewSave = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
        //    Logger::Info("Model matrix");
        //    modelViewSave.Dump();
    Matrix4 projectionSave = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
        //    Logger::Info("Proj matrix");
        //    projectionSave.Dump();
    int32 renderOrientation = RenderManager::Instance()->GetRenderOrientation();
    
    Rect viewportSave = RenderManager::Instance()->GetViewport();
    RenderManager::Instance()->SetViewport(viewportRect, false);
    RenderManager::Instance()->ClearDepthBuffer();
	RenderManager::Instance()->ClearStencilBuffer(0);
    
        //    glEnable(GL_DEPTH_TEST);
    
        //  glMatrixMode(GL_MODELVIEW);
        //	glPushMatrix();
        //	glMatrixMode(GL_PROJECTION);
        //	glPushMatrix();
    
        //  Not required because Scene should setup it state before draw
        //    RenderManager::Instance()->EnableDepthWrite(true);
        //    RenderManager::Instance()->EnableDepthTest(true);
        //    RenderManager::Instance()->EnableBlending(false);
    
    
    if (scene)
        scene->Draw();

        
    //    RenderManager::Instance()->EnableDepthTest(false);
    //    RenderManager::Instance()->EnableDepthWrite(false);
    //    RenderManager::Instance()->EnableBlending(true);
    
    /*
     Restore render orientation
     */
    RenderManager::Instance()->SetViewport(viewportSave, true);
    RenderManager::Instance()->SetRenderOrientation(renderOrientation);
    
        //RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_2D_STATE_BLEND);
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, modelViewSave);
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_PROJECTION, projectionSave);
	RenderManager::Instance()->PopDrawMatrix();
	RenderManager::Instance()->PopMappingMatrix();
        //    modelViewSave = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
        //    Logger::Info("Model matrix");
        //    modelViewSave.Dump();
        //    projectionSave = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
        //    Logger::Info("Proj matrix");
        //    projectionSave.Dump();
}
    
void UI3DView::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    float32 aspect = size.dx / size.dy;
    
    if(scene)
    {
        for (int32 k = 0; k < scene->GetCameraCount(); ++k)
        {
            scene->GetCamera(k)->SetAspect(aspect);
        }
    }
}

}