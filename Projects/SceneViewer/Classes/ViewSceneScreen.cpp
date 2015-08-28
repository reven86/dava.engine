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


#include "ViewSceneScreen.h"
#include "GameCore.h"

ViewSceneScreen::ViewSceneScreen()
    : BaseScreen()
    , camera(NULL)
    , info(NULL)
    , frameCounter(0)
    , framesTime(0.f)
    , drawTime(0)
    , updateTime(0)
{
}

void ViewSceneScreen::LoadResources()
{
    BaseScreen::LoadResources();
 
    ScopedPtr<Scene> scene(new Scene());
    scene->LoadScene(GameCore::Instance()->GetScenePath());
    
    DVASSERT(camera == NULL);
    camera = new Camera();
    
    VirtualCoordinatesSystem* vcs = DAVA::VirtualCoordinatesSystem::Instance();
	float32 aspect = (float32)vcs->GetVirtualScreenSize().dy / (float32)vcs->GetVirtualScreenSize().dx;
    
	camera->SetupPerspective(70.f, aspect, 0.5f, 2500.f);
	camera->SetLeft(Vector3(1, 0, 0));
	camera->SetUp(Vector3(0, 0, 1.f));
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetPosition(Vector3(0, 0, 100));


    //TODO: debug
    camera->SetPosition(Vector3(18.7855f, 111.0697f, 57.0635f));
    camera->SetDirection(Vector3(-1.2904f, 9.0496f, -4.0545f));
    //ENDOFTODO

    
    scene->AddCamera(camera);
    scene->SetCurrentCamera(camera);
    
    Rect screenRect = GetRect();
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    screenRect.dx = screenSize.dx;
    screenRect.dy = screenSize.dy;
    SetRect(screenRect);
    ScopedPtr<UI3DView> sceneView(new UI3DView(screenRect));
    sceneView->SetScene(scene);
    sceneView->SetInputEnabled(false);
    AddControl(sceneView);
    
    ScopedPtr<UIButton> backButton(CreateButton(Rect(0, 0, 90, 30), L"Back"));
    backButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ViewSceneScreen::OnBack));
    backButton->SetDebugDraw(true);
    AddControl(backButton);
    
    
    const float32 joyPADSize = 100.f;
    
    moveJoyPAD = new UIJoypad(Rect(0, screenRect.dy - joyPADSize, joyPADSize, joyPADSize));
    moveJoyPAD->GetBackground()->SetSprite("~res:/Gfx/Joypad/joypad", 0);
    moveJoyPAD->SetStickSprite("~res:/Gfx/Joypad/joypad", 1);

    AddControl(moveJoyPAD);
    
    viewJoyPAD = new UIJoypad(Rect(screenRect.dx - joyPADSize, screenRect.dy - joyPADSize, joyPADSize, joyPADSize));
    viewJoyPAD->GetBackground()->SetSprite("~res:/Gfx/Joypad/joypad", 0);
    viewJoyPAD->SetStickSprite("~res:/Gfx/Joypad/joypad", 1);
    AddControl(viewJoyPAD);
    
    viewXAngle = 0;
    viewYAngle = 0;
    
    DVASSERT(info == NULL);
    info = new UIStaticText(Rect(0, 0, screenRect.dx, 30.f));
    info->SetFont(font);
    info->SetTextColor(Color::White);
    info->SetTextAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    
    AddControl(info);
}

void ViewSceneScreen::UnloadResources()
{
    SafeRelease(info);
    
    SafeRelease(viewJoyPAD);
    SafeRelease(moveJoyPAD);
    
    SafeRelease(camera);
    
    BaseScreen::UnloadResources();
}

void ViewSceneScreen::OnBack(BaseObject *caller, void *param, void *callerData)
{
    SetPreviousScreen();
}

void ViewSceneScreen::Draw(const DAVA::UIGeometricData &geometricData)
{
    uint64 startTime = SystemTimer::Instance()->GetAbsoluteNano();
    
    RenderManager::Instance()->ClearDepthBuffer();

    BaseScreen::Draw(geometricData);

    drawTime += (SystemTimer::Instance()->GetAbsoluteNano() - startTime);
}



void ViewSceneScreen::Update(float32 timeElapsed)
{
    uint64 startTime = SystemTimer::Instance()->GetAbsoluteNano();

    BaseScreen::Update(timeElapsed);
    
    updateTime += (SystemTimer::Instance()->GetAbsoluteNano() - startTime);

    UpdateCamera(timeElapsed);
    UpdateInfo(timeElapsed);
}


void ViewSceneScreen::UpdateCamera(float32 timeElapsed)
{
    Vector2 angleJoypadPos = viewJoyPAD->GetDigitalPosition();
    viewXAngle += angleJoypadPos.x * timeElapsed * 25.0f;
    viewYAngle += angleJoypadPos.y * timeElapsed * 25.0f;
    
    aimUser.Identity();
    Matrix4 mt, mt2;
    mt.CreateTranslation(Vector3(0,10,0));
    aimUser *= mt;
    mt.CreateRotation(Vector3(0,0,1), DegToRad(viewXAngle));
    mt2.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
    mt2 *= mt;
    aimUser *= mt2;
    
    Vector3 dir = Vector3() * aimUser;
    
    Vector2 joypadPos = moveJoyPAD->GetDigitalPosition();
    
    Vector3 pos = camera->GetPosition();
    pos += -joypadPos.y * dir * timeElapsed * 4;
    //pos.y += joypadPos.y * dir.y;
    
    camera->SetPosition(pos);
    camera->SetDirection(dir);
}

static const float32 INFO_UPDATE_TIME = 1.0f;
void ViewSceneScreen::UpdateInfo(float32 timeElapsed)
{
    ++frameCounter;
    framesTime += timeElapsed;
    
    if(framesTime > INFO_UPDATE_TIME)
    {
        int32 fps = (int32)(frameCounter / framesTime);
        info->SetText(Format(L"FPS: %d  Draw: %ldns  Update: %ldns", fps, drawTime / frameCounter, updateTime / frameCounter));
        
        framesTime -= INFO_UPDATE_TIME;
        frameCounter = 0;
        
        drawTime = updateTime = 0;
    }
}

void ViewSceneScreen::DidAppear()
{
    framesTime = 0.f;
    frameCounter = 0;
    
    drawTime = updateTime = 0;
    
    info->SetText(L"");
}

