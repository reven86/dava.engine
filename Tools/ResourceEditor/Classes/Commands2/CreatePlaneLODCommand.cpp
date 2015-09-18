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
#include "Scene/SceneHelper.h"

#include "Render/Material/NMaterialNames.h"

using namespace DAVA;

CreatePlaneLODCommand::CreatePlaneLODCommand(DAVA::LodComponent * _lodComponent, int32 _fromLodLayer, uint32 _textureSize, const DAVA::FilePath & texturePath)
    : Command2(CMDID_LOD_CREATE_PLANE, "Create Plane LOD")
    , lodComponent(_lodComponent)
    , planeBatch(NULL)
    , planeImage(NULL)
    , newSwitchIndex(-1)
    , fromLodLayer(_fromLodLayer)
    , textureSize(_textureSize)
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
    SceneHelper::EnumerateEntityTextures(fromEntity->GetScene(), fromEntity, textures, SceneHelper::EXCLUDE_NULL);
    DAVA::eGPUFamily currentGPU = static_cast<DAVA::eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32());

    DAVA::TexturesMap::const_iterator it = textures.begin();
    DAVA::TexturesMap::const_iterator end = textures.end();
    for(; it != end; ++it)
        it->second->ReloadAs(GPU_ORIGIN);

    Rect oldViewport = RenderManager::Instance()->GetViewport();
    Texture * oldRenderTarget = RenderManager::Instance()->GetRenderTarget();
    
    Rect newViewport = viewport;
    if(newViewport.dx == -1)
        newViewport.dx = (float32)toTexture->GetWidth();
    if(newViewport.dy == -1)
        newViewport.dy = (float32)toTexture->GetHeight();

    RenderManager::Instance()->SetRenderTarget(toTexture);
	RenderManager::Instance()->SetViewport(newViewport);

    Scene * tempScene = new Scene();
    if(clearTarget)
        tempScene->SetClearBuffers(RenderManager::ALL_BUFFERS);
    else
        tempScene->SetClearBuffers(0);

    NMaterial * globalMaterial = fromEntity->GetScene()->GetGlobalMaterial();
    if(globalMaterial)
        tempScene->SetGlobalMaterial(globalMaterial->Clone());

    Entity * entity = SceneHelper::CloneEntityWithMaterials(fromEntity);
	entity->SetLocalTransform(DAVA::Matrix4::IDENTITY);

    SpeedTreeObject * treeObejct = GetSpeedTreeObject(entity);
    if(treeObejct)
    {
        Vector<Vector3> fakeSH(9, Vector3());
        fakeSH[0].x = fakeSH[0].y = fakeSH[0].z = 1.f/0.564188f; //fake SH value to make original object color
        treeObejct->SetSphericalHarmonics(fakeSH);
    }

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

    RenderManager::Instance()->SetRenderTarget(oldRenderTarget);
    RenderManager::Instance()->SetViewport(oldViewport);

    it = textures.begin();
    end = textures.end();
    for(; it != end; ++it)
        it->second->ReloadAs(currentGPU);
}

void CreatePlaneLODCommand::CreatePlaneImage()
{
    DVASSERT(planeImage == NULL);
    
    DAVA::Entity *fromEntity = GetEntity();
    
    AABBox3 bbox = GetRenderObject(fromEntity)->GetBoundingBox();
    bool isMeshHorizontal = IsHorisontalMesh(bbox);
    
    const Vector3 & min = bbox.min;
    const Vector3 & max = bbox.max;
    
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

    AABBox3 bbox = GetRenderObject(fromEntity)->GetBoundingBox();
    bool isMeshHorizontal = IsHorisontalMesh(bbox);
    
    const Vector3 & min = bbox.min;
    Vector3 size = bbox.GetSize();

    //
    // Textures:
    //  Vertical for tree:   Horizontal for bush:
    //  +---------------+     +---------------+ 
    //  |       |       |     |      ***      |
    //  |   *   |   *   |     |     *****     |
    //  |  ***  |  ***  |     |---------------|
    //  | ***** | ***** |     |      ***      |
    //  |   *   |   *   |     |     ******    |
    //  +---------------+     +---------------+
    //
    // Mesh Grid: 
    //
    // z
    // ^
    // |
    // |  9     10     11
    // |   *-----*-----*
    // |   | \ / | \ / |
    // |   |16*  |17*  |
    // |   | / \ | / \ |
    // |  6*-----*-----*8
    // |   | \ / | \ / |
    // |   |14*  |15*  |
    // |   | / \ | / \ |
    // |  3*-----*-----*5
    // |   | \ / | \ / |
    // |   |12*  |13*  |
    // |   | / \ | / \ |
    // |   *-----*-----*
    // |  0      1      2
    // |
    // +--------------------> x(y)
    //

    int32 gridSizeX = 2, gridSizeY = 3;
    int32 vxCount = ((gridSizeX + 1)*(gridSizeY + 1) + gridSizeX*gridSizeY) * 2; // (sx+1)*(sy+1) for cell corner vertices; sx*sy for cell center vertices; for 2 planes
    int32 indCount = gridSizeX * gridSizeY * 4 * 2 * 3; //4 triangles per cell; for 2 planes

    Vector2 txCoordPlane2Offset;
    Vector2 txCoordPlaneScale;
    if(isMeshHorizontal)
    {
        txCoordPlane2Offset = Vector2(0.f, .5f);
        txCoordPlaneScale = Vector2(1.f, .5f);
    }
    else
    {
        txCoordPlane2Offset = Vector2(.5f, 0.f);
        txCoordPlaneScale = Vector2(.5f, 1.f);
    }

    int32 plane2VxIndexOffset = vxCount / 2;

    Vector2 cellCenterTxCoordOffset = Vector2(.5f / gridSizeX, .5f / gridSizeY) * txCoordPlaneScale;

    PolygonGroup * planePG = new PolygonGroup();
    planePG->AllocateData(EVF_VERTEX | EVF_TEXCOORD0, vxCount, indCount);

    int32 currentIndex = 0;
    for(int32 z = 0; z <= gridSizeY; ++z)
    {
        float32 rowCoord = min.z + (size.z * z) / gridSizeY;
        float32 rowTxCoord = z / (float32)gridSizeY;
        int32 rowVxIndexOffset = (gridSizeX + 1) * z;
        int32 nextRowVxIndexOffset = rowVxIndexOffset + (gridSizeX + 1);

        int32 cellCenterVxIndexOffset = (gridSizeX + 1)*(gridSizeY + 1) - z;
        float32 rowCenterZCoord = rowCoord + size.z / gridSizeY / 2.f;

        for(int32 xy = 0; xy <= gridSizeX; ++xy) // xy and z - it's grid 'coords'. Variable 'xy' - shared variable for two planes.
        {
            //cell corner vertices
            int32 vxIndex1 = xy + rowVxIndexOffset;
            int32 vxIndex2 = vxIndex1 + plane2VxIndexOffset;
            float32 xCoord = min.x + size.x * xy / (float32)gridSizeX; //first plane in Oxz
            float32 yCoord = min.y + size.y * xy / (float32)gridSizeX; //second plane in Oyz

            Vector3 coord1(xCoord, 0.f, rowCoord); //1st plane
            Vector3 coord2(0.f, yCoord, rowCoord); //2nd plane

            Vector2 txCoord1 = Vector2(xy / (float32)gridSizeX, rowTxCoord) * txCoordPlaneScale;
            Vector2 txCoord2 = txCoord1 + txCoordPlane2Offset;

            planePG->SetCoord(vxIndex1, coord1);
            planePG->SetTexcoord(0, vxIndex1, txCoord1);

            planePG->SetCoord(vxIndex2, coord2);
            planePG->SetTexcoord(0, vxIndex2, txCoord2);

            //cell center vertices
            if(z != gridSizeY && xy != gridSizeX)
            {
                int32 centerVxIndex1 = vxIndex1 + cellCenterVxIndexOffset;
                int32 centerVxIndex2 = centerVxIndex1 + plane2VxIndexOffset;

                float32 centerXCoord = xCoord + size.x / gridSizeX / 2.f;
                float32 centerYCoord = yCoord + size.y / gridSizeX / 2.f;
                planePG->SetCoord(centerVxIndex1, Vector3(centerXCoord, 0.f, rowCenterZCoord));
                planePG->SetCoord(centerVxIndex2, Vector3(0.f, centerYCoord, rowCenterZCoord));
                planePG->SetTexcoord(0, centerVxIndex1, txCoord1 + cellCenterTxCoordOffset);
                planePG->SetTexcoord(0, centerVxIndex2, txCoord2 + cellCenterTxCoordOffset);

#define BIND_TRIANGLE_INDECIES(vi1, vi2, vi3) \
{\
    /*triangle for first plane */ \
    planePG->SetIndex(currentIndex, vi1); ++currentIndex;\
    planePG->SetIndex(currentIndex, vi2); ++currentIndex;\
    planePG->SetIndex(currentIndex, vi3); ++currentIndex;\
    /*triangle for second plane */ \
    planePG->SetIndex(currentIndex, vi1 + plane2VxIndexOffset); ++currentIndex;\
    planePG->SetIndex(currentIndex, vi2 + plane2VxIndexOffset); ++currentIndex;\
    planePG->SetIndex(currentIndex, vi3 + plane2VxIndexOffset); ++currentIndex;\
}\

                BIND_TRIANGLE_INDECIES( xy +  rowVxIndexOffset,        xy + nextRowVxIndexOffset,     centerVxIndex1);
                BIND_TRIANGLE_INDECIES( xy + nextRowVxIndexOffset,    (xy+1) + nextRowVxIndexOffset,  centerVxIndex1);
                BIND_TRIANGLE_INDECIES((xy+1) + nextRowVxIndexOffset, (xy+1) + rowVxIndexOffset,      centerVxIndex1);
                BIND_TRIANGLE_INDECIES((xy+1) + rowVxIndexOffset,      xy + rowVxIndexOffset,         centerVxIndex1);

#undef BIND_TRIANGLE_INDECIES
            }
        }
    }
    
    planeBatch = new RenderBatch();
    planeBatch->SetPolygonGroup(planePG);
    SafeRelease(planePG);

	NMaterial* material = NMaterial::CreateMaterialInstance(FastName(Format("%s_planes", fromEntity->GetName().c_str())),
															NMaterialName::TEXTURED_ALPHATEST,
															NMaterial::DEFAULT_QUALITY_NAME);

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
    ImageSystem::Instance()->Save(textureSavePath, planeImage);
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

bool CreatePlaneLODCommand::IsHorisontalMesh(const AABBox3 & bbox)
{
    const Vector3 & min = bbox.min;
    const Vector3 & max = bbox.max;

    return ((max.x - min.x) / (max.z - min.z) > 1.f || (max.y - min.y) / (max.z - min.z) > 1.f);
}
