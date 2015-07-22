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
const FastName MaterialsTest::PLANE_ENTITY = FastName("plane");

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
    SafeRelease(materialsScene);
    
    for(NMaterial* material : materials)
    {
        SafeRelease(material);
    }
    
    for(Entity* child : planes)
    {
        SafeRelease(child);
    }
    
    for(Entity* child : spoPlanes)
    {
        SafeRelease(child);
    }
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
    
    for(int32 i = 0; i < 11; i++)
    {
        Entity* clone = planeEntity->Clone();
        Matrix4 cloneMatrix = planeEntity->GetLocalTransform() * Matrix4::MakeTranslation(Vector3(0.0f + i * 10.0f, 0.0f, 0.0f));
        clone->SetLocalTransform(cloneMatrix);
        GetScene()->AddNode(clone);
        
        planes.push_back(clone);
        spoPlanes.push_back(CreateSpeedTreeEntity(clone));
        skinnedPlanes.push_back(CreateSkinnedEntity(clone));
        lightmapMaterialPlanes.push_back(CreateEntityForLightmapMaterial(clone));
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
    
    GetScene()->SetCurrentCamera(camera);
    
    SafeRelease(planeEntity);
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

    // set next material
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
    
    // material test finished
    if(GetTestFrameNumber() - currentTestStartFrame == FRAMES_PER_MATERIAL_TEST)
    {
        float32 testTime = (SystemTimer::Instance()->FrameStampTimeMS() - currentTestStartTime) / 1000.0f;
        materialTestsElapsedTime.push_back(testTime);
        
        NMaterial* material = materials[currentMaterialIndex]->GetParent();
        
        if(material->GetMaterialName().find("Spherical") != String::npos ||
           material->GetMaterialName().find("Skinning") != String::npos ||
           material->GetMaterialName().find("TextureLightmap") != String::npos)
        {
            ReplacePlanes(planes);
        }
        
        currentMaterialIndex++;
    }
    
    // material test started
    if(GetTestFrameNumber() % FRAMES_PER_MATERIAL_TEST == 0)
    {
        if(currentMaterialIndex < materials.size() && materials[currentMaterialIndex]->GetParent()->GetMaterialName().find("Spherical") != String::npos)
        {
            ReplacePlanes(spoPlanes);
        }
        
        if(currentMaterialIndex < materials.size() && materials[currentMaterialIndex]->GetParent()->GetMaterialName().find("Skinning") != String::npos)
        {
            ReplacePlanes(skinnedPlanes);
        }
        
        if(currentMaterialIndex < materials.size() && materials[currentMaterialIndex]->GetParent()->GetMaterialName().find("TextureLightmap") != String::npos)
        {
            ReplacePlanes(lightmapMaterialPlanes);
        }
        
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
        String materialName = "MaterialSubtestName:" + String(materials[i]->GetParent()->GetMaterialName().c_str());
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

Entity* MaterialsTest::CreateSpeedTreeEntity(Entity* entity)
{
    Entity* spoEntity = entity->Clone();
    RenderComponent* spoRenderComponent = static_cast<RenderComponent*>(spoEntity->GetComponent(Component::RENDER_COMPONENT));
    RenderObject* renderObject = spoRenderComponent->GetRenderObject();
    
    PolygonGroup* spoPolygonGroup = new PolygonGroup();
    PolygonGroup* polygonGroup = renderObject->GetRenderBatch(0)->GetPolygonGroup();
    
    int32 vertexCount = polygonGroup->GetVertexCount();
    int32 indexCount = polygonGroup->GetIndexCount();
    
    spoPolygonGroup->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_NORMAL , vertexCount, indexCount);
    
    for(int32 i = 0; i < vertexCount; i++)
    {
        Vector3 v;
        Vector2 t;
        
        polygonGroup->GetCoord(i, v);
        spoPolygonGroup->SetCoord(i, v);
        
        polygonGroup->GetNormal(i, v);
        spoPolygonGroup->SetNormal(i, v);
        
        polygonGroup->GetTexcoord(0, i, t);
        spoPolygonGroup->SetTexcoord(0, i, t);
        
        spoPolygonGroup->SetColor(i, Color(1.0f, 0.0f, 1.0f, 1.0f).GetRGBA());
    }
    
    for(int32 i = 0; i < indexCount; i++)
    {
        int32 index;
        polygonGroup->GetIndex(i, index);
        spoPolygonGroup->SetIndex(i, index);
    }
    
    spoPolygonGroup->RecalcAABBox();
    spoPolygonGroup->BuildBuffers();
    
    // fix for calculate normals in ShpericalLit shader
    spoPolygonGroup->aabbox.max.z = 1.0f;
    
    RenderBatch* spoRenderBatch = new RenderBatch();
    spoRenderBatch->SetPolygonGroup(spoPolygonGroup);
    spoRenderBatch->SetMaterial(renderObject->GetRenderBatch(0)->GetMaterial()->Clone());
    
    SpeedTreeObject* spoRenderObject = new SpeedTreeObject();
    spoRenderComponent->SetRenderObject(spoRenderObject);
    spoRenderObject->AddRenderBatch(spoRenderBatch);
    
    Vector<Vector3> fakeSH(9, Vector3());
    fakeSH[0].x = fakeSH[0].y = fakeSH[0].z = 1.f/0.564188f; //fake SH value to make original object color
    spoRenderObject->SetSphericalHarmonics(fakeSH);
    
    return spoEntity;
}

Entity* MaterialsTest::CreateSkinnedEntity(Entity* sourceEntity)
{
    Entity* skinnedEntity = sourceEntity->Clone();
    
    Entity* entityHierarhy = new Entity();
    entityHierarhy->AddNode(sourceEntity->Clone());
    
    Vector<SkeletonComponent::JointConfig> joints;
    RenderObject * skinnedRo = MeshUtils::CreateSkinnedMesh(entityHierarhy, joints);
    
    RenderComponent* renderComponent = static_cast<RenderComponent*>(skinnedEntity->GetComponent(Component::RENDER_COMPONENT));
    renderComponent->SetRenderObject(skinnedRo);
    
    SkeletonComponent* skeleton = new SkeletonComponent();
    skinnedEntity->AddComponent(skeleton);
    
    skeleton->SetConfigJoints(joints);
    skeleton->RebuildFromConfig();
    
    SafeRelease(entityHierarhy);

    return skinnedEntity;
}

Entity* MaterialsTest::CreateEntityForLightmapMaterial(DAVA::Entity *entity)
{
    Entity* lightmapEntity = entity->Clone();
    RenderComponent* lightmapRenderComponent = static_cast<RenderComponent*>(lightmapEntity->GetComponent(Component::RENDER_COMPONENT));
    RenderObject* renderObject = lightmapRenderComponent->GetRenderObject();
    
    PolygonGroup* lightmapPolygonGroup = new PolygonGroup();
    PolygonGroup* polygonGroup = renderObject->GetRenderBatch(0)->GetPolygonGroup();
    
    int32 vertexCount = polygonGroup->GetVertexCount();
    int32 indexCount = polygonGroup->GetIndexCount();
    
    int32 format = polygonGroup->GetFormat() | EVF_TEXCOORD1;
    
    lightmapPolygonGroup->AllocateData(format, vertexCount, indexCount);
    
    for(int32 i = 0; i < vertexCount; i++)
    {
        Vector3 v;
        Vector2 t;
        
        polygonGroup->GetCoord(i, v);
        lightmapPolygonGroup->SetCoord(i, v);
        
        polygonGroup->GetNormal(i, v);
        lightmapPolygonGroup->SetNormal(i, v);
        
        polygonGroup->GetTexcoord(0, i, t);
        lightmapPolygonGroup->SetTexcoord(0, i, t);
        lightmapPolygonGroup->SetTexcoord(1, i, t);
    }
    
    for(int32 i = 0; i < indexCount; i++)
    {
        int32 index;
        polygonGroup->GetIndex(i, index);
        lightmapPolygonGroup->SetIndex(i, index);
    }
    
    lightmapPolygonGroup->RecalcAABBox();
    lightmapPolygonGroup->BuildBuffers();
    
    // fix for calculate normals in ShpericalLit shader
    //lightmapPolygonGroup->aabbox.max.z = 1.0f;
    
    renderObject->GetRenderBatch(0)->SetPolygonGroup(lightmapPolygonGroup);
    
    return lightmapEntity;
}

void MaterialsTest::ReplacePlanes(const Vector<Entity*>& planes)
{
    List<Entity*> children;
    GetScene()->FindNodesByNamePart(PLANE_ENTITY.c_str(), children);
    
    for(Entity* child : children)
    {
        GetScene()->RemoveNode(child);
    }
    
    for(Entity* child : planes)
    {
        GetScene()->AddNode(child);
    }
}

