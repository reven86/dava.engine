#include "CreatePlaneLODCommandHelper.h"

#include "Render/Material/NMaterialNames.h"
#include "Render/Renderer.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Scene3D/Lod/LodSystem.h"

#include "Scene/SceneHelper.h"
#include "Settings/SettingsManager.h"
#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"

using namespace DAVA;

namespace CreatePlaneLODCommandHelper
{
bool IsHorisontalMesh(const DAVA::AABBox3& bbox);

void CreatePlaneImageForRequest(RequestPointer&);
void CreatePlaneBatchForRequest(RequestPointer&);

void DrawToTextureForRequest(RequestPointer&, DAVA::Entity* entity, DAVA::Camera* camera,
                             DAVA::int32 fromLodLayer, const rhi::Viewport& viewport, bool clearTarget);
}

CreatePlaneLODCommandHelper::RequestPointer CreatePlaneLODCommandHelper::RequestRenderToTexture(DAVA::LodComponent* lodComponent,
                                                                                                DAVA::int32 fromLodLayer, DAVA::uint32 textureSize, const DAVA::FilePath& texturePath)
{
    CreatePlaneLODCommandHelper::RequestPointer result(new Request());
    result->lodComponent = lodComponent;
    result->fromLodLayer = fromLodLayer;
    result->textureSize = textureSize;
    result->texturePath = texturePath;

    result->newLodIndex = GetLodLayersCount(lodComponent);
    DVASSERT(result->newLodIndex > 0);

    if (fromLodLayer == -1)
        fromLodLayer = result->newLodIndex - 1;

    CreatePlaneImageForRequest(result);
    CreatePlaneBatchForRequest(result);

    return result;
}

void CreatePlaneLODCommandHelper::CreatePlaneImageForRequest(RequestPointer& request)
{
    DVASSERT(request->planeImage == nullptr);
    DVASSERT(request->targetTexture == nullptr);

    DAVA::Entity* fromEntity = request->lodComponent->GetEntity();

    AABBox3 bbox = GetRenderObject(fromEntity)->GetBoundingBox();
    bool isMeshHorizontal = IsHorisontalMesh(bbox);

    const Vector3& min = bbox.min;
    const Vector3& max = bbox.max;

    ScopedPtr<Camera> camera(new Camera());
    camera->SetTarget(Vector3(0, 0, 0));
    camera->SetUp(Vector3(0.f, 0.f, 1.f));
    camera->SetIsOrtho(true);

    float32 textureSize = static_cast<float>(request->textureSize);
    float32 halfSizef = 0.5f * textureSize;

    rhi::Viewport firstSideViewport;
    rhi::Viewport secondSideViewport;
    if (isMeshHorizontal)
    {
        firstSideViewport = rhi::Viewport(0, 0, textureSize, halfSizef);
        secondSideViewport = rhi::Viewport(0, halfSizef, textureSize, halfSizef);
    }
    else
    {
        firstSideViewport = rhi::Viewport(0, 0, halfSizef, textureSize);
        secondSideViewport = rhi::Viewport(halfSizef, 0, halfSizef, textureSize);
    }

    rhi::Texture::Descriptor descriptor = {};
    descriptor.width = textureSize;
    descriptor.height = textureSize;
    descriptor.autoGenMipmaps = false;
    descriptor.type = rhi::TEXTURE_TYPE_2D;
    descriptor.format = rhi::TEXTURE_FORMAT_D24S8;

    DVASSERT(request->targetTexture == nullptr);
    request->targetTexture = Texture::CreateFBO(textureSize, textureSize, FORMAT_RGBA8888);
    request->depthTexture = rhi::CreateTexture(descriptor);
    request->RegisterRenderCallback();

    // draw 1st side
    float32 depth = max.y - min.y;
    camera->Setup(min.x, max.x, max.z, min.z, -depth, 2.0f * depth);
    camera->SetPosition(Vector3(0.0f, min.y, 0.0f));
    DrawToTextureForRequest(request, fromEntity, camera, request->fromLodLayer, firstSideViewport, true);

    // draw 2nd side
    depth = max.x - min.x;
    camera->Setup(min.y, max.y, max.z, min.z, -depth, 2.0f * depth);
    camera->SetPosition(Vector3(max.x, 0.0f, 0.0f));
    DrawToTextureForRequest(request, fromEntity, camera, request->fromLodLayer, secondSideViewport, false);
}

void CreatePlaneLODCommandHelper::CreatePlaneBatchForRequest(RequestPointer& request)
{
    DAVA::Entity* fromEntity = request->lodComponent->GetEntity();

    AABBox3 bbox = GetRenderObject(fromEntity)->GetBoundingBox();
    bool isMeshHorizontal = IsHorisontalMesh(bbox);

    const Vector3& min = bbox.min;
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

    // (sx+1) * (sy+1) for cell corner vertices; sx*sy for cell center vertices; for 2 planes
    int32 vxCount = ((gridSizeX + 1) * (gridSizeY + 1) + gridSizeX * gridSizeY) * 2;
    int32 indCount = gridSizeX * gridSizeY * 4 * 2 * 3; //4 triangles per cell; for 2 planes

    Vector2 txCoordPlane2Offset;
    Vector2 txCoordPlaneScale;
    if (isMeshHorizontal)
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

    ScopedPtr<PolygonGroup> planePG(new PolygonGroup());
    planePG->AllocateData(EVF_VERTEX | EVF_TEXCOORD0 | EVF_PIVOT4, vxCount, indCount);

    int32 currentIndex = 0;
    for (int32 z = 0; z <= gridSizeY; ++z)
    {
        float32 rowCoord = min.z + (size.z * z) / gridSizeY;
        float32 rowTxCoord = z / (float32)gridSizeY;
        int32 rowVxIndexOffset = (gridSizeX + 1) * z;
        int32 nextRowVxIndexOffset = rowVxIndexOffset + (gridSizeX + 1);

        int32 cellCenterVxIndexOffset = (gridSizeX + 1) * (gridSizeY + 1) - z;
        float32 rowCenterZCoord = rowCoord + size.z / gridSizeY / 2.f;

        // xy and z - it's grid 'coords'. Variable 'xy' - shared variable for two planes.
        for (int32 xy = 0; xy <= gridSizeX; ++xy)
        {
            //cell corner vertices
            int32 vxIndex1 = xy + rowVxIndexOffset;
            int32 vxIndex2 = vxIndex1 + plane2VxIndexOffset;
            float32 xCoord = min.x + size.x * xy / (float32)gridSizeX; //first plane in Oxz
            float32 yCoord = min.y + size.y * xy / (float32)gridSizeX; //second plane in Oyz

            if (xy == gridSizeX / 2) //align middle vertices to center
            {
                xCoord = 0.f;
                yCoord = 0.f;
            }

            Vector3 coord1(xCoord, 0.f, rowCoord); //1st plane
            Vector3 coord2(0.f, yCoord, rowCoord); //2nd plane

            Vector2 txCoord1 = Vector2((xCoord - min.x) / size.x, rowTxCoord) * txCoordPlaneScale;
            Vector2 txCoord2 = Vector2((yCoord - min.y) / size.y, rowTxCoord) * txCoordPlaneScale + txCoordPlane2Offset;

            planePG->SetCoord(vxIndex1, coord1);
            planePG->SetTexcoord(0, vxIndex1, txCoord1);
            planePG->SetPivot(vxIndex1, Vector4());

            planePG->SetCoord(vxIndex2, coord2);
            planePG->SetTexcoord(0, vxIndex2, txCoord2);
            planePG->SetPivot(vxIndex2, Vector4());

            // cell center vertices
            if (z != gridSizeY && xy != gridSizeX)
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
				}

                BIND_TRIANGLE_INDECIES(xy + rowVxIndexOffset, xy + nextRowVxIndexOffset, centerVxIndex1);
                BIND_TRIANGLE_INDECIES(xy + nextRowVxIndexOffset, (xy + 1) + nextRowVxIndexOffset, centerVxIndex1);
                BIND_TRIANGLE_INDECIES((xy + 1) + nextRowVxIndexOffset, (xy + 1) + rowVxIndexOffset, centerVxIndex1);
                BIND_TRIANGLE_INDECIES((xy + 1) + rowVxIndexOffset, xy + rowVxIndexOffset, centerVxIndex1);

#undef BIND_TRIANGLE_INDECIES
            }
        }
    }

    if (GetSpeedTreeObject(fromEntity) != nullptr)
        planePG.reset(SpeedTreeObject::CreateSortedPolygonGroup(planePG));

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetMaterialName(FastName(DAVA::Format("plane_lod_%d_for_%s", request->newLodIndex, fromEntity->GetName().c_str())));
    material->SetFXName(NMaterialName::TEXTURED_ALPHABLEND);

    ScopedPtr<NMaterial> materialInstance(new NMaterial());
    materialInstance->SetParent(material);
    materialInstance->SetMaterialName(FastName("Instance0"));

    request->planeBatch->SetPolygonGroup(planePG);
    request->planeBatch->SetMaterial(materialInstance);
}

void CreatePlaneLODCommandHelper::DrawToTextureForRequest(RequestPointer& request, DAVA::Entity* fromEntity, DAVA::Camera* camera,
                                                          DAVA::int32 fromLodLayer, const rhi::Viewport& viewport, bool clearTarget)
{
    request->ReloadTexturesToGPU(GPU_ORIGIN);
    ScopedPtr<Scene> tempScene(new Scene());

    tempScene->SetMainRenderTarget(request->targetTexture->handle, request->depthTexture,
                                   clearTarget ? rhi::LOADACTION_CLEAR : rhi::LOADACTION_NONE, Color::Clear);

    tempScene->SetMainPassProperties(PRIORITY_SERVICE_3D, Rect(viewport.x, viewport.y, viewport.width, viewport.height),
                                     request->targetTexture->GetWidth(), request->targetTexture->GetHeight(), request->targetTexture->GetFormat());

    tempScene->GetRenderSystem()->SetAntialiasingAllowed(false);

    NMaterial* globalMaterial = fromEntity->GetScene()->GetGlobalMaterial();
    if (globalMaterial)
    {
        ScopedPtr<NMaterial> global(globalMaterial->Clone());
        if (global->HasLocalFlag(NMaterialFlagName::FLAG_VERTEXFOG))
            global->RemoveFlag(NMaterialFlagName::FLAG_VERTEXFOG);

        tempScene->SetGlobalMaterial(global);
    }

    ScopedPtr<Entity> clonedEnity(SceneHelper::CloneEntityWithMaterials(fromEntity));
    clonedEnity->SetLocalTransform(DAVA::Matrix4::IDENTITY);

    SpeedTreeObject* treeObejct = GetSpeedTreeObject(clonedEnity);
    if (treeObejct)
    {
        DAVA::Array<DAVA::float32, SpeedTreeObject::HARMONICS_BUFFER_CAPACITY> harmonics = {};
        harmonics[0] = harmonics[1] = harmonics[2] = 1.f / 0.564188f; //fake SH value to make original object color
        treeObejct->SetSphericalHarmonics(harmonics);
    }

    tempScene->AddNode(clonedEnity);
    tempScene->AddCamera(camera);
    tempScene->SetCurrentCamera(camera);
    camera->SetupDynamicParameters(false);

    tempScene->lodSystem->SetForceLodLayer(GetLodComponent(clonedEnity), fromLodLayer);
    clonedEnity->SetVisible(true);

    tempScene->Update(1.0f / 60.0f);
    tempScene->Draw();
}

bool CreatePlaneLODCommandHelper::IsHorisontalMesh(const AABBox3& bbox)
{
    const Vector3& min = bbox.min;
    const Vector3& max = bbox.max;
    return ((max.x - min.x) / (max.z - min.z) > 1.f || (max.y - min.y) / (max.z - min.z) > 1.f);
}

/*
 * Request methods
 */

CreatePlaneLODCommandHelper::Request::Request()
    : RefCounter()
    , planeBatch(new RenderBatch())
    , completed(false)
{
}

CreatePlaneLODCommandHelper::Request::~Request()
{
    SafeRelease(planeBatch);
    SafeRelease(planeImage);
    SafeRelease(targetTexture);
    rhi::DeleteTexture(depthTexture);
}

void CreatePlaneLODCommandHelper::Request::RegisterRenderCallback()
{
    Renderer::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(),
                                   MakeFunction(this, &CreatePlaneLODCommandHelper::Request::OnRenderCallback));
}

void CreatePlaneLODCommandHelper::Request::ReloadTexturesToGPU(DAVA::eGPUFamily targetGPU)
{
    auto entity = lodComponent->GetEntity();

    SceneHelper::TextureCollector collector;
    SceneHelper::EnumerateEntityTextures(entity->GetScene(), entity, collector);
    DAVA::TexturesMap& textures = collector.GetTextures();

    for (auto& tex : textures)
    {
        tex.second->ReloadAs(targetGPU);
    }

    DAVA::Set<DAVA::NMaterial*> materials;
    SceneHelper::EnumerateMaterials(entity, materials);
    for (auto& mat : materials)
    {
        mat->InvalidateTextureBindings();
    }
}

void CreatePlaneLODCommandHelper::Request::OnRenderCallback(rhi::HSyncObject object)
{
    completed = true;

    DVASSERT(planeImage == nullptr);
    planeImage = targetTexture->CreateImageFromMemory();
    SafeRelease(targetTexture);

    DAVA::eGPUFamily currentGPU = static_cast<DAVA::eGPUFamily>(
    SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32());
    ReloadTexturesToGPU(currentGPU);
}
