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

#include "MaterialsTest.h"

const FastName MaterialsTest::CAMERA_START = FastName("CameraStart");
const FastName MaterialsTest::CAMERA_TARGET = FastName("CameraTarget");
const FastName MaterialsTest::MATERIALS = FastName("Materials");
const FastName MaterialsTest::PLANE_ENTITY = FastName("plane160x.sc2");

const uint32 MaterialsTest::FRAMES_PER_MATERIAL_TEST = 60;

MaterialsTest::MaterialsTest(const TestParams& testParams)
    :   BaseTest("MaterialsTest", testParams)
    ,   currentTestStartFrame(0)
    ,   currentTestStartTime(0)
    ,   currentMaterialIndex(0)
    ,   materialsScene(nullptr)
    ,   camera(nullptr)
{
    
}

MaterialsTest::~MaterialsTest()
{
    SafeRelease(camera);
}

void MaterialsTest::LoadResources()
{
    BaseTest::LoadResources();
    
    materialsScene = new Scene();
    
    SceneFileV2::eError error = materialsScene->LoadScene(FilePath("~res:/3d/Maps/materials/materials.sc2"));
    DVASSERT_MSG(error == SceneFileV2::eError::ERROR_NO_ERROR, "can't load scene ~res:/3d/Maps/materials/materials.sc2");
    
    Entity* materialsEntity = materialsScene->FindByName(MATERIALS);
    
    for(int32 i = 0; i < materialsEntity->GetChildrenCount(); i++)
    {
        RenderComponent* renderComponent = static_cast<RenderComponent*>(materialsEntity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
        NMaterial* material = renderComponent->GetRenderObject()->GetRenderBatch(0)->GetMaterial();
        
        materials.push_back(material);
    }
    
    Entity* planeEntity = materialsScene->FindByName(PLANE_ENTITY);
    
    for(int32 i = 0; i < 1; i++)
    {
        Entity* clone = planeEntity->Clone();
        Matrix4 cloneMatrix = planeEntity->GetLocalTransform() * Matrix4::MakeTranslation(Vector3(0.0f + i * 10.0f, 0.0f, 0.0f));
        clone->SetLocalTransform(cloneMatrix);
        GetScene()->AddNode(clone);
    }
    
    Entity* light = materialsScene->FindByName("Light");
    GetScene()->AddNode(light);
    
    camera = new Camera();
    camera->SetIsOrtho(true);
    camera->SetOrthoWidth(1024);
    camera->SetAspect(1.33f);
    camera->SetPosition(Vector3(-20.0f, 0.0f, 0.0f));
    camera->SetTarget(Vector3(-19.0f, 0.0f, 0.0f));
    camera->SetUp(Vector3::UnitZ);
    camera->SetLeft(-Vector3::UnitY);

    /*Light* dirLight = new Light();
    dirLight->SetDirection(Vector3(1.0f, 0.0f, 0.0f));
    
    Entity* lightEntity = new Entity();
    lightEntity->AddComponent(new LightComponent(dirLight));
    
    Matrix4 lightRotation = Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(-102))
                            * Matrix4::MakeRotation(Vector3(0.0f, 1.0f, 1.0f), DegToRad(-39))
                            * Matrix4::MakeRotation(Vector3(1.0f, 0.0f, 1.0f), DegToRad(-28));
    
    lightEntity->SetLocalTransform(lightRotation); */
    
    GetScene()->SetCurrentCamera(camera);
    //GetScene()->AddNode(lightEntity);
}

void MaterialsTest::UnloadResources()
{
    BaseTest::UnloadResources();
}

void MaterialsTest::PerformTestLogic(float32 timeElapsed)
{
    
}

void MaterialsTest::BeginFrame()
{
    BaseTest::BeginFrame();
    
    // test next materials
    if(GetTestFrameNumber() > 0 && GetTestFrameNumber() % FRAMES_PER_MATERIAL_TEST == 1)
    {
        NMaterial* currentMaterial = materials[currentMaterialIndex];
        
        List<Entity*> children;
        GetScene()->FindNodesByNamePart(PLANE_ENTITY.c_str(), children);
        
        for(Entity* child : children)
        {
            RenderComponent* renderComponent = static_cast<RenderComponent*>(child->GetComponent(Component::RENDER_COMPONENT));
            renderComponent->GetRenderObject()->GetRenderBatch(0)->SetMaterial(currentMaterial);
        }
    }
    
    if(GetTestFrameNumber() - currentTestStartFrame == FRAMES_PER_MATERIAL_TEST)
    {
        float32 testTime = (SystemTimer::Instance()->FrameStampTimeMS() - currentTestStartTime) / 1000.0f;
        materialTestsElapsedTime.push_back(testTime);
        
        currentMaterialIndex++;
    }
    
    if(GetTestFrameNumber() % FRAMES_PER_MATERIAL_TEST == 0)
    {
        currentTestStartTime = SystemTimer::Instance()->FrameStampTimeMS();
        currentTestStartFrame = GetTestFrameNumber();
    }
}

bool MaterialsTest::IsFinished() const
{
    return GetTestFrameNumber() == materials.size() * FRAMES_PER_MATERIAL_TEST;
}

void MaterialsTest::PrintStatistic(const Vector<BaseTest::FrameInfo>& frames)
{
    BaseTest::PrintStatistic(frames);
    
    for(int32 i = 0; i < materials.size(); i++)
    {
        String materialName = "MaterialSubtestName:" + String(materials[i]->GetMaterialName().c_str());
        Logger::Info(materialName.c_str());
        
        float32 materialSubtestTime = 0.0f;
        float32 materialSubtestElapsedTime = materialTestsElapsedTime[i];
        
        for(int32 j = FRAMES_PER_MATERIAL_TEST * i; j < FRAMES_PER_MATERIAL_TEST * (i + 1); j++)
        {
            materialSubtestTime += frames[j].delta;
            
            Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
                                                                   TeamcityTestsOutput::MATERIAL_FRAME_DELTA,
                                                                   DAVA::Format("%f", frames[j].delta)).c_str());
        }
        
        Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
                                                               TeamcityTestsOutput::MATERIAL_TEST_TIME,
                                                               DAVA::Format("%f", materialSubtestTime)).c_str());
        
        Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
                                                               TeamcityTestsOutput::MATERIAL_ELAPSED_TEST_TIME,
                                                               DAVA::Format("%f", materialSubtestElapsedTime)).c_str());
        
    }
}
