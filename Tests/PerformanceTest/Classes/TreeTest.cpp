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



#include "TreeTest.h"

TreeTest::TreeTest(const String &testName, const String &scenePathname)
    :   TestTemplate<TreeTest>(testName)
{
    testScenePathname = scenePathname;
    
    testCounter = 0;

    PerfFuncData data;
    data.testData.name = testName;
    data.testData.runCount = TEST_FRAMES_COUNT;
    data.func = NULL;
    data.screen = this;
    
    perfFuncs.push_back(data);
}

void TreeTest::LoadResources()
{
    Scene *scene = new Scene();
    scene->AddNode(scene->GetRootNode(testScenePathname)); 

    Camera *cam = (Camera *)scene->FindByName(String("CameraTEST"));
    scene->AddCamera(cam); 
    scene->SetCurrentCamera(cam);
    
    UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
    sceneView->SetScene(scene);
    AddControl(sceneView);
    SafeRelease(sceneView);
    
    SafeRelease(scene);
    
    testCounter = 0;
}

void TreeTest::UnloadResources()
{
    RemoveAllControls();
    testCounter = 0;
}

bool TreeTest::RunTest(int32 testNum)
{
    if(0 == testCounter)
    {
        perfFuncs[0].testData.startTime = SystemTimer::Instance()->AbsoluteMS();
    }
    else if(TEST_FRAMES_COUNT == testCounter)
    {
        perfFuncs[0].testData.endTime = SystemTimer::Instance()->AbsoluteMS();
        perfFuncs[0].testData.totalTime = perfFuncs[0].testData.endTime - perfFuncs[0].testData.startTime;
        
//        perfFuncs[0].testData.maxTimeIndex = (int32)((float32)TEST_FRAMES_COUNT / ((float32)perfFuncs[0].testData.totalTime / 1000.0f));            
        perfFuncs[0].testData.minTime = (uint64)((float32)TEST_FRAMES_COUNT / ((float32)perfFuncs[0].testData.totalTime / 1000.0f));            
        perfFuncs[0].testData.maxTime = (uint64)((float32)perfFuncs[0].testData.totalTime / 1000.0f) / ((float32)TEST_FRAMES_COUNT);            
        return true;
    }
    ++testCounter;

    return false;
}




void TreeTest::Draw(const UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearDepthBuffer();

    TestTemplate<TreeTest>::Draw(geometricData);
}
