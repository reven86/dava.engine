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

#include "CreatePlaneLODCommand.h"
#include "Qt/Scene/SceneHelper.h"
#include "Qt/Settings/SettingsManager.h"
#include "Classes/CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

#include "Render/Material/NMaterialNames.h"

using namespace DAVA;

CreatePlaneLODCommand::CreatePlaneLODCommand(DAVA::LodComponent * _lodComponent, int32 _fromLodLayer, uint32 _textureSize, const DAVA::FilePath & texturePath)
    : Command2(CMDID_LOD_CREATE_PLANE, "Create Plane LOD")
    , newSwitchIndex(-1)
    , planeImage(NULL)
    , planeBatch(NULL)
    , fromLodLayer(_fromLodLayer)
    , textureSize(_textureSize)
    , lodComponent(_lodComponent)
    , textureSavePath(texturePath)
{
    DVASSERT(GetRenderObject(GetEntity()));
    
	savedDistances = lodComponent->lodLayersArray;

    newLodIndex = GetLodLayersCount(lodComponent);
    DVASSERT(newLodIndex > 0);
    
    if(fromLodLayer == -1)
        fromLodLayer = newLodIndex - 1;
    
    CreatePlaneImage();
    CreatePlaneBatch();
}

CreatePlaneLODCommand::~CreatePlaneLODCommand()
{
    SafeRelease(planeBatch);
    SafeRelease(planeImage);
}

void CreatePlaneLODCommand::Redo()
{
    CreateTextureFiles();

    Texture *t = planeBatch->GetMaterial()->GetTexture(NMaterial::TEXTURE_ALBEDO);
    t->Reload();
    
    DAVA::Entity *entity = GetEntity();
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);
    
    ro->AddRenderBatch(planeBatch, newLodIndex, newSwitchIndex);
    
    lodComponent->SetLodLayerDistance(newLodIndex, lodComponent->GetLodLayerDistance(newLodIndex-1) * 2);
}

void CreatePlaneLODCommand::Undo()
{
    DAVA::Entity *entity = GetEntity();
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);

    //restore batches
	ro->RemoveRenderBatch(planeBatch);

    //restore distances
    lodComponent->lodLayersArray = savedDistances;

    //fix visibility settings
    DAVA::int32 maxLodIndex = ro->GetMaxLodIndex();
    if(lodComponent->forceLodLayer > maxLodIndex)
    {
        lodComponent->forceLodLayer = maxLodIndex;
    }
    
	lodComponent->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
    DeleteTextureFiles();
}

DAVA::Entity* CreatePlaneLODCommand::GetEntity() const
{
    return lodComponent->GetEntity();
}


void CreatePlaneLODCommand::DrawToTexture(DAVA::Entity * fromEntity, DAVA::Camera * camera, DAVA::Texture * toTexture, DAVA::int32 fromLodLayer, const DAVA::Rect & viewport /* = DAVA::Rect(0, 0, -1, -1) */, bool clearTarget /* = true */)
{
    DAVA::TexturesMap textures;
    SceneHelper::EnumerateEntityTextures(fromEntity->GetScene(), fromEntity, textures);
    DAVA::eGPUFamily currentGPU = (DAVA::eGPUFamily)SettingsManager::Instance()->GetValue("TextureViewGPU", SettingsManager::INTERNAL).AsInt32();

    DAVA::TexturesMap::const_iterator it = textures.begin();
    DAVA::TexturesMap::const_iterator end = textures.end();
    for(; it != end; ++it)
        it->second->ReloadAs(GPU_UNKNOWN);

    Rect oldViewport = RenderManager::Instance()->GetViewport();
    Rect newViewport = viewport;

    if(newViewport.dx == -1)
        newViewport.dx = (float32)toTexture->GetWidth();
    if(newViewport.dy == -1)
        newViewport.dy = (float32)toTexture->GetHeight();

    RenderManager::Instance()->SetRenderTarget(toTexture);

	RenderManager::Instance()->SetViewport(newViewport, true);

    if(clearTarget)
        RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

    Scene * tempScene = new Scene();
    Entity * entity = fromEntity->Clone();
	entity->SetLocalTransform(DAVA::Matrix4::IDENTITY);

    tempScene->AddNode(entity);
    tempScene->AddCamera(camera);
    tempScene->SetCurrentCamera(camera);

	camera->SetupDynamicParameters();

	DAVA::LodComponent *lodComponent = GetLodComponent(entity);
	lodComponent->SetForceLodLayer(fromLodLayer);
	entity->SetVisible(true);
	tempScene->Update(0.1f);
    tempScene->Draw();

    SafeRelease(entity);
    SafeRelease(tempScene);

    RenderManager::Instance()->SetViewport(oldViewport, true);

#ifdef __DAVAENGINE_OPENGL__
    RenderManager::Instance()->HWglBindFBO(RenderManager::Instance()->GetFBOViewFramebuffer());
#endif //#ifdef __DAVAENGINE_OPENGL__

    it = textures.begin();
    end = textures.end();
    for(; it != end; ++it)
        it->second->ReloadAs(currentGPU);
}

void CreatePlaneLODCommand::CreatePlaneImage()
{
    DVASSERT(planeImage == NULL);
    
    DAVA::Entity *fromEntity = GetEntity();
    
    AABBox3 bbox; Matrix4 wtMx;
    fromEntity->GetWorldTransform().GetInverse(wtMx);
    fromEntity->GetWTMaximumBoundingBoxSlow().GetTransformedBox(wtMx, bbox);
    
    const Vector3 & min = bbox.min;
    const Vector3 & max = bbox.max;
    bool isMeshHorizontal = ((max.x - min.x) / (max.z - min.z) > 1.f) || ((max.y - min.y) / (max.z - min.z) > 1.f);
    
    Camera * camera = new Camera();
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetUp(Vector3(0.f, 0.f, 1.f));
    camera->SetIsOrtho(true);

    float32 halfSizef = textureSize / 2.f;
    
    
    Rect firstSideViewport, secondSideViewport;
    if(isMeshHorizontal) //bushes
    {
        firstSideViewport = Rect(0, 0, (float32)textureSize, halfSizef);
        secondSideViewport = Rect(0, halfSizef, (float32)textureSize, halfSizef);
    }
    else //trees
    {
        firstSideViewport = Rect(0, 0, halfSizef, (float32)textureSize);
        secondSideViewport = Rect(halfSizef, 0, halfSizef, (float32)textureSize);
    }
    
    Texture * fboTexture = Texture::CreateFBO(textureSize, textureSize, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);

    float32 depth = 0.f;
    //draw 1st side
    depth = max.y - min.y;
 	camera->Setup(min.x, max.x, max.z, min.z, -depth, depth * 2);
    camera->SetPosition(Vector3(0.f, min.y, 0.f));
    DrawToTexture(fromEntity, camera, fboTexture, fromLodLayer, firstSideViewport, true);
    
    //draw 2nd side
    depth = max.x - min.x;
	camera->Setup(min.y, max.y, max.z, min.z, -depth, depth * 2);
    camera->SetPosition(Vector3(max.x, 0.f, 0.f));
    DrawToTexture(fromEntity, camera, fboTexture, fromLodLayer, secondSideViewport, false);
    
    SafeRelease(camera);

    planeImage = fboTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_OPAQUE);
    SafeRelease(fboTexture);
}

void CreatePlaneLODCommand::CreatePlaneBatch()
{
    DVASSERT(planeBatch == NULL);
    
    DAVA::Entity *fromEntity = GetEntity();
    
    AABBox3 bbox; Matrix4 wtMx;
    fromEntity->GetWorldTransform().GetInverse(wtMx);
    fromEntity->GetWTMaximumBoundingBoxSlow().GetTransformedBox(wtMx, bbox);
    
    const Vector3 & min = bbox.min;
    const Vector3 & max = bbox.max;
    bool isMeshHorizontal = (max.x - min.x) / (max.z - min.z) > 1.f || (max.y - min.y) / (max.z - min.z) > 1.f;

    int32 vxCount = 8, indCount = 12;

    PolygonGroup * planePG = new PolygonGroup();
    planePG->AllocateData(EVF_VERTEX | EVF_TEXCOORD0, vxCount, indCount);
    
    //1st plane
    planePG->SetCoord(0, Vector3(0.f, min.y, max.z));
    planePG->SetCoord(1, Vector3(0.f, max.y, max.z));
    planePG->SetCoord(2, Vector3(0.f, max.y, min.z));
    planePG->SetCoord(3, Vector3(0.f, min.y, min.z));
    
    planePG->SetIndex(0, 1);
    planePG->SetIndex(1, 0);
    planePG->SetIndex(2, 3);
    planePG->SetIndex(3, 1);
    planePG->SetIndex(4, 3);
    planePG->SetIndex(5, 2);
    
    //2nd plane
    planePG->SetCoord(4, Vector3(min.x, 0.f, max.z));
    planePG->SetCoord(5, Vector3(max.x, 0.f, max.z));
    planePG->SetCoord(6, Vector3(max.x, 0.f, min.z));
    planePG->SetCoord(7, Vector3(min.x, 0.f, min.z));
    
    planePG->SetIndex(6,  5);
    planePG->SetIndex(7,  4);
    planePG->SetIndex(8,  7);
    planePG->SetIndex(9,  5);
    planePG->SetIndex(10, 7);
    planePG->SetIndex(11, 6);
    
    if(isMeshHorizontal)
    {
        //1st plane
        planePG->SetTexcoord(0, 0, Vector2(.0f, .5f));
        planePG->SetTexcoord(0, 1, Vector2(1.f, .5f));
        planePG->SetTexcoord(0, 2, Vector2(1.f, 1.f));
        planePG->SetTexcoord(0, 3, Vector2(.0f, 1.f));
        
        //2nd plane
        planePG->SetTexcoord(0, 4, Vector2(.0f, 0.f));
        planePG->SetTexcoord(0, 5, Vector2(1.f, 0.f));
        planePG->SetTexcoord(0, 6, Vector2(1.f, .5f));
        planePG->SetTexcoord(0, 7, Vector2(.0f, .5f));
    }
    else
    {
        //1st plane
        planePG->SetTexcoord(0, 0, Vector2(.5f, 0.f));
        planePG->SetTexcoord(0, 1, Vector2(1.f, 0.f));
        planePG->SetTexcoord(0, 2, Vector2(1.f, 1.f));
        planePG->SetTexcoord(0, 3, Vector2(.5f, 1.f));
        
        //2nd plane
        planePG->SetTexcoord(0, 4, Vector2(.0f, 0.f));
        planePG->SetTexcoord(0, 5, Vector2(.5f, 0.f));
        planePG->SetTexcoord(0, 6, Vector2(.5f, 1.f));
        planePG->SetTexcoord(0, 7, Vector2(.0f, 1.f));
    }
    
    planeBatch = new RenderBatch();
    planeBatch->SetPolygonGroup(planePG);
    SafeRelease(planePG);

	NMaterial* material = NMaterial::CreateMaterialInstance(FastName(Format("%s_planes", fromEntity->GetName().c_str())),
															NMaterialName::TEXTURED_ALPHATEST,
															NMaterial::DEFAULT_QUALITY_NAME);
	material->SetFlag(NMaterial::FLAG_VERTEXFOG, NMaterial::FlagOn);
	NMaterialHelper::DisableStateFlags(PASS_FORWARD, material, RenderStateData::STATE_CULL);
	material->SetTexture(NMaterial::TEXTURE_ALBEDO, TextureDescriptor::GetDescriptorPathname(textureSavePath));
	
    planeBatch->SetMaterial(material);
    SafeRelease(material);
}


void CreatePlaneLODCommand::CreateTextureFiles()
{
    DVASSERT(planeImage);
    
    FilePath folder = textureSavePath.GetDirectory();
    FileSystem::Instance()->CreateDirectory(folder, true);
    
    ImageLoader::Save(planeImage, textureSavePath);
    TextureDescriptorUtils::CreateDescriptorIfNeed(textureSavePath);
}

void CreatePlaneLODCommand::DeleteTextureFiles()
{
    FileSystem::Instance()->DeleteFile(textureSavePath);
    FileSystem::Instance()->DeleteFile(TextureDescriptor::GetDescriptorPathname(textureSavePath));
}


DAVA::RenderBatch * CreatePlaneLODCommand::GetRenderBatch() const
{
    return planeBatch;
}
