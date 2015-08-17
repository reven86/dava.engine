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

const FastName MaterialsTest::LIGHT_ENTITY = FastName("Light");
const FastName MaterialsTest::CAMERA_ENTITY = FastName("OrthoCamera");
const FastName MaterialsTest::PLANE_ENTITY = FastName("plane");
const FastName MaterialsTest::MATERIALS_ENTITY = FastName("Materials");

const String MaterialsTest::TEST_NAME = "MaterialsTest";
const String MaterialsTest::SPHERICAL_LIT_MATERIAL = "SphericalLit";
const String MaterialsTest::SKINNED_MATERIAL = "Skinning";
const String MaterialsTest::LIGHTMAP_MATERIAL = "Lightmap";

const uint32 MaterialsTest::FRAMES_PER_MATERIAL_TEST = 60;

MaterialsTest::MaterialsTest(const TestParams& testParams)
    :   BaseTest(TEST_NAME, testParams)
    ,   currentTestStartFrame(0)
    ,   currentTestStartTime(0)
    ,   currentMaterialIndex(0)
{
}

MaterialsTest::~MaterialsTest()
{
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
    
    for(Entity* child : skinnedPlanes)
    {
        SafeRelease(child);
    }
    
    for(Entity* child : lightmapMaterialPlanes)
    {
        SafeRelease(child);
    }
}

void MaterialsTest::LoadResources()
{
    BaseTest::LoadResources();
    
    ScopedPtr<Scene> materialsScene(new Scene());
    
    SceneFileV2::eError error = materialsScene->LoadScene(FilePath("~res:/3d/Maps/" + GetParams().scenePath));
    DVASSERT_MSG(error == SceneFileV2::eError::ERROR_NO_ERROR, ("can't load scene " + GetParams().scenePath).c_str());
    
    Entity* materialsEntity = materialsScene->FindByName(MATERIALS_ENTITY);
    
    for(int32 i = 0; i < materialsEntity->GetChildrenCount(); i++)
    {
        RenderComponent* renderComponent = static_cast<RenderComponent*>(materialsEntity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
        NMaterial* material = renderComponent->GetRenderObject()->GetRenderBatch(0)->GetMaterial()->Clone();

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
    
    Entity* light = materialsScene->FindByName(LIGHT_ENTITY);
    GetScene()->AddNode(light);
    
    Entity* camera = materialsScene->FindByName(CAMERA_ENTITY);
    CameraComponent* cameraComponent = static_cast<CameraComponent*>(camera->GetComponent(Component::CAMERA_COMPONENT));
    GetScene()->SetCurrentCamera(cameraComponent->GetCamera());
}

void MaterialsTest::BeginFrame()
{
    BaseTest::BeginFrame();
    
    // material test finished
    if(GetTestFrameNumber() - currentTestStartFrame == FRAMES_PER_MATERIAL_TEST)
    {
        float32 testTime = (SystemTimer::Instance()->FrameStampTimeMS() - currentTestStartTime) / 1000.0f;
        materialTestsElapsedTime.push_back(testTime);
        
        NMaterial* material = materials[currentMaterialIndex]->GetParent();
        
        if(material->GetMaterialName().find(SPHERICAL_LIT_MATERIAL) != String::npos ||
           material->GetMaterialName().find(SKINNED_MATERIAL) != String::npos ||
           material->GetMaterialName().find(LIGHTMAP_MATERIAL) != String::npos)
        {
            ReplacePlanes(planes);
        }
        
        currentMaterialIndex++;
    }
    
    // material test started
    if(GetTestFrameNumber() % FRAMES_PER_MATERIAL_TEST == 0)
    {
        size_t materialsCount = materials.size();
        
        if(currentMaterialIndex < materialsCount)
        {
            NMaterial* currentMaterial = materials[currentMaterialIndex]->GetParent();
            
            if(currentMaterial->GetMaterialName().find(SPHERICAL_LIT_MATERIAL) != String::npos)
            {
                ReplacePlanes(spoPlanes);
            }
            
            if(currentMaterial->GetMaterialName().find(SKINNED_MATERIAL) != String::npos)
            {
                ReplacePlanes(skinnedPlanes);
            }
            
            if(currentMaterial->GetMaterialName().find(LIGHTMAP_MATERIAL) != String::npos)
            {
                ReplacePlanes(lightmapMaterialPlanes);
            }

            List<Entity*> children;
            GetScene()->FindNodesByNamePart(PLANE_ENTITY.c_str(), children);
            
            for(Entity* child : children)
            {
                RenderComponent* renderComponent = static_cast<RenderComponent*>(child->GetComponent(Component::RENDER_COMPONENT));
                renderComponent->GetRenderObject()->GetRenderBatch(0)->SetMaterial(materials[currentMaterialIndex]);
            }
        }
        
        currentTestStartTime = SystemTimer::Instance()->FrameStampTimeMS();
        currentTestStartFrame = GetTestFrameNumber();
    }
}

bool MaterialsTest::IsFinished() const
{
    return GetTestFrameNumber() > materials.size() * FRAMES_PER_MATERIAL_TEST;
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
    PolygonGroup* polygonGroup = renderObject->GetRenderBatch(0)->GetPolygonGroup();
    
    ScopedPtr<RenderBatch> spoRenderBatch(new RenderBatch());
    ScopedPtr<PolygonGroup> spoPolygonGroup(new PolygonGroup());
    ScopedPtr<SpeedTreeObject> spoRenderObject(new SpeedTreeObject());
    
    int32 vertexCount = polygonGroup->GetVertexCount();
    int32 indexCount = polygonGroup->GetIndexCount();
    
    spoPolygonGroup->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_NORMAL , vertexCount, indexCount);
    
    Vector3 v;
    Vector2 t;
    
    int32 index;
    
    for(int32 i = 0; i < vertexCount; i++)
    {
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
        polygonGroup->GetIndex(i, index);
        spoPolygonGroup->SetIndex(i, index);
    }
    
    spoPolygonGroup->RecalcAABBox();
    spoPolygonGroup->BuildBuffers();
    
    // fix for calculate normals in ShpericalLit shader
    spoPolygonGroup->aabbox.max.z = 1.0f;
    
    spoRenderBatch->SetPolygonGroup(spoPolygonGroup);
    spoRenderBatch->SetMaterial(ScopedPtr<NMaterial>(renderObject->GetRenderBatch(0)->GetMaterial()->Clone()));
    spoRenderObject->AddRenderBatch(spoRenderBatch);
    spoRenderComponent->SetRenderObject(spoRenderObject);
    
    Vector<Vector3> fakeSH(9, Vector3());
    fakeSH[0].x = fakeSH[0].y = fakeSH[0].z = 1.f/0.564188f; //fake SH value to make original object color
    spoRenderObject->SetSphericalHarmonics(fakeSH);
    
    return spoEntity;
}

Entity* MaterialsTest::CreateSkinnedEntity(Entity* sourceEntity)
{
    Entity* skinnedEntity = sourceEntity->Clone();
    
    ScopedPtr<Entity> entityHierarhy(new Entity());
    entityHierarhy->AddNode(ScopedPtr<Entity>(sourceEntity->Clone()));
    
    Vector<SkeletonComponent::JointConfig> joints;
    
    RenderComponent* renderComponent = static_cast<RenderComponent*>(skinnedEntity->GetComponent(Component::RENDER_COMPONENT));
    renderComponent->SetRenderObject(ScopedPtr<RenderObject>(MeshUtils::CreateSkinnedMesh(entityHierarhy, joints)));
    
    SkeletonComponent* skeleton = new SkeletonComponent();
    skinnedEntity->AddComponent(skeleton);
    
    skeleton->SetConfigJoints(joints);
    skeleton->RebuildFromConfig();

    return skinnedEntity;
}

Entity* MaterialsTest::CreateEntityForLightmapMaterial(DAVA::Entity *entity)
{
    Entity* lightmapEntity = entity->Clone();
    RenderComponent* lightmapRenderComponent = static_cast<RenderComponent*>(lightmapEntity->GetComponent(Component::RENDER_COMPONENT));
    RenderObject* renderObject = lightmapRenderComponent->GetRenderObject();
    PolygonGroup* polygonGroup = renderObject->GetRenderBatch(0)->GetPolygonGroup();
    
    ScopedPtr<PolygonGroup> lightmapPolygonGroup(new PolygonGroup());
    
    int32 vertexCount = polygonGroup->GetVertexCount();
    int32 indexCount = polygonGroup->GetIndexCount();
    
    lightmapPolygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL | EVF_TEXCOORD0 | EVF_TEXCOORD1 , vertexCount, indexCount);
    
    Vector3 v;
    Vector2 t;
    
    int32 index;
    
    for(int32 i = 0; i < vertexCount; i++)
    {
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
        polygonGroup->GetIndex(i, index);
        lightmapPolygonGroup->SetIndex(i, index);
    }
    
    lightmapPolygonGroup->RecalcAABBox();
    lightmapPolygonGroup->BuildBuffers();
    
    renderObject->GetRenderBatch(0)->SetPolygonGroup(lightmapPolygonGroup);
    
    return lightmapEntity;
}

void MaterialsTest::ReplacePlanes(const Vector<Entity*>& planes)
{
    List<Entity*> children;
    GetScene()->FindNodesByNamePart(PLANE_ENTITY.c_str(), children);
    
    for(auto *child : children)
    {
        GetScene()->RemoveNode(child);
    }
    
    for(auto *child : planes)
    {
        GetScene()->AddNode(child);
    }
}

const String& MaterialsTest::GetSceneName() const
{
    return GetTestName();
}

