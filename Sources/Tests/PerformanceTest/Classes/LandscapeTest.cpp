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



#include "LandscapeTest.h"

LandscapeTest::LandscapeTest(const String &testName, Landscape::eTiledShaderMode mode)
    :   TestTemplate<LandscapeTest>(testName)
    ,   shaderMode(mode)
{
    testCounter = 0;
    
    for(int32 i = 0; i < Landscape::TEXTURE_COUNT; ++i)
    {
        textures[i] = NULL;
    }
    
    if(Landscape::TILED_MODE_COUNT == shaderMode)
    {
        for(int32 i = 0; i < Landscape::TEXTURE_COUNT; ++i)
        {
            RegisterFunction(this, &LandscapeTest::DrawSprite, Format("TestLandscapeTexture_%02d", i), TEST_FRAMES_COUNT/2, (void *)i);
        }
    }
    else 
    {
        PerfFuncData data;
        data.testData.name = testName;
        data.testData.runCount = TEST_FRAMES_COUNT;
        data.func = NULL;
        data.screen = this;
        
        perfFuncs.push_back(data);
    }
}

void LandscapeTest::LoadResources()
{
	Landscape * land = new Landscape();  
    SafeRelease(land);
    
    Scene *scene = new Scene();
    scene->AddNode(scene->GetRootNode(String("~res:/3d/LandscapeTest/landscapetest.sc2"))); 

    Entity *cameraHolder = scene->FindByName(String("TestCamera"));
    Camera *cam = GetCamera(cameraHolder);
    scene->AddCamera(cam); 
    scene->SetCurrentCamera(cam);
  
    Entity *landscapeHolder = scene->FindByName(String("Landscape"));
    land = GetLandscape(landscapeHolder);
    if(land)
    {
        if(Landscape::TILED_MODE_COUNT == shaderMode)
        {
            for(int32 i = 0; i < Landscape::TEXTURE_COUNT; ++i)
            {
                Texture *t = land->GetTexture((Landscape::eTextureLevel)i);
                if(t)
                {
                    textures[i] = Sprite::CreateFromTexture(t, 0, 0, t->GetWidth(), t->GetHeight());
                }
            }
        }
        else 
        {
            land->SetTiledShaderMode(shaderMode);
            UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
            sceneView->SetScene(scene);
            AddControl(sceneView);
            SafeRelease(sceneView);
        }
    }
    
    SafeRelease(scene);
    
    testCounter = 0;
}

void LandscapeTest::UnloadResources()
{
    for(int32 i = 0; i < Landscape::TEXTURE_COUNT; ++i)
    {
        SafeRelease(textures[i]);
    }

    RemoveAllControls();

    testCounter = 0;
}

bool LandscapeTest::RunTest(int32 testNum)
{
    if(Landscape::TILED_MODE_COUNT == shaderMode)
    {
        return TestTemplate<LandscapeTest>::RunTest(testNum);
    }
    else 
    {
        if(0 == testCounter)
        {
            perfFuncs[0].testData.startTime = SystemTimer::Instance()->AbsoluteMS();
        }
        else if(TEST_FRAMES_COUNT == testCounter)
        {
            perfFuncs[0].testData.endTime = SystemTimer::Instance()->AbsoluteMS();
            perfFuncs[0].testData.totalTime = perfFuncs[0].testData.endTime - perfFuncs[0].testData.startTime;

            perfFuncs[0].testData.maxTimeIndex = (int32)((float32)TEST_FRAMES_COUNT / ((float32)perfFuncs[0].testData.totalTime / 1000.0f));            
            return true;
        }
        ++testCounter;
    }
    return false;
}


void LandscapeTest::DrawSprite(PerfFuncData * data)
{
#if defined (__DAVAENGINE_MACOS__) && defined (__x86_64__) 
	int32 spriteIndex = (int64)data->testData.userData;
#else //#if defined (__DAVAENGINE_MACOS__) && defined (__x86_64__) 
	int32 spriteIndex = (int32)data->testData.userData;
#endif //#if defined (__DAVAENGINE_MACOS__) && defined (__x86_64__) 

    if(textures[spriteIndex])
    {
        textures[spriteIndex]->Reset();
        textures[spriteIndex]->SetFrame(0);
        textures[spriteIndex]->SetPosition(0.f, 0.f);
        textures[spriteIndex]->Draw();
    }
}


void LandscapeTest::Draw(const UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearDepthBuffer();

    TestTemplate<LandscapeTest>::Draw(geometricData);
}
