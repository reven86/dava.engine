#include "CommandLine/Private/REConsoleModuleTestUtils.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"

#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

namespace RECMTUDetail
{
using namespace DAVA;

bool CreateImageFile(const FilePath& imagePathname, uint32 width, uint32 height, PixelFormat format)
{
    ScopedPtr<Image> image(Image::Create(width, height, format));

    uint8 byte = Random::Instance()->Rand(255);
    Memset(image->data, byte, image->dataSize);

    FileSystem::Instance()->CreateDirectory(imagePathname.GetDirectory(), true);
    eErrorCode saved = ImageSystem::Save(imagePathname, image, image->format);
    return (saved == eErrorCode::SUCCESS);
}

bool CreateHeightmapFile(const FilePath& heightmapPathname, uint32 width, PixelFormat format)
{
    DVASSERT(format == PixelFormat::FORMAT_A8 || format == PixelFormat::FORMAT_A16);
    return CreateImageFile(heightmapPathname, width, width, format);
}

NMaterial* CreateMaterial(const FastName& materialName, const FastName& fxName)
{
    NMaterial* material = new NMaterial();
    material->SetMaterialName(materialName);
    material->SetFXName(fxName);
    return material;
}

PolygonGroup* CreatePolygonGroup()
{
    PolygonGroup* renderData = new PolygonGroup();
    renderData->AllocateData(EVF_VERTEX | EVF_TEXCOORD0, 4, 6);
    renderData->SetPrimitiveType(rhi::PrimitiveType::PRIMITIVE_TRIANGLELIST);
    renderData->SetCoord(0, Vector3(-1.0f, -1.0f, 0.0f));
    renderData->SetCoord(1, Vector3(1.0f, -1.0f, 0.0f));
    renderData->SetCoord(2, Vector3(-1.0f, 1.0f, 0.0f));
    renderData->SetCoord(3, Vector3(1.0f, 1.0f, 0.0f));
    renderData->SetTexcoord(0, 0, Vector2(0.0f, 0.0f));
    renderData->SetTexcoord(0, 1, Vector2(1.0f, 0.0f));
    renderData->SetTexcoord(0, 2, Vector2(0.0f, 1.0f));
    renderData->SetTexcoord(0, 3, Vector2(1.0f, 1.0f));
    renderData->SetIndex(0, 0);
    renderData->SetIndex(1, 1);
    renderData->SetIndex(2, 2);
    renderData->SetIndex(3, 2);
    renderData->SetIndex(4, 1);
    renderData->SetIndex(5, 3);
    renderData->BuildBuffers();

    return renderData;
}

bool CreateTextureFiles(const FilePath& texturePathname, uint32 width, uint32 height, PixelFormat format)
{
    FilePath pngPathname = FilePath::CreateWithNewExtension(texturePathname, ".png");
    if (CreateImageFile(pngPathname, width, height, format))
    {
        TextureDescriptorUtils::CreateOrUpdateDescriptor(pngPathname);

        std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(texturePathname));
        if (descriptor)
        {
            descriptor->compression[eGPUFamily::GPU_POWERVR_IOS].format = PixelFormat::FORMAT_RGBA8888;
            descriptor->compression[eGPUFamily::GPU_POWERVR_ANDROID].format = PixelFormat::FORMAT_RGBA5551;
            descriptor->compression[eGPUFamily::GPU_ADRENO].format = PixelFormat::FORMAT_RGBA4444;
            descriptor->compression[eGPUFamily::GPU_MALI].format = PixelFormat::FORMAT_RGB888;
            descriptor->compression[eGPUFamily::GPU_TEGRA].format = PixelFormat::FORMAT_RGB565;
            descriptor->compression[eGPUFamily::GPU_DX11].format = PixelFormat::FORMAT_A8;

            descriptor->SetGenerateMipmaps(true);
            descriptor->Save();
            return true;
        }
    }
    return false;
}

void CreateR2OCustomProperty(Entity* entity, const FilePath& scenePathname)
{
    String entityName = entity->GetName().c_str();

    FilePath entityPathname = scenePathname;
    entityPathname.ReplaceBasename(entityName);
    FilePath folderPathname = scenePathname.GetDirectory();

    CustomPropertiesComponent* cp = new CustomPropertiesComponent();
    cp->GetArchive()->SetString("editor.referenceToOwner", entityPathname.GetAbsolutePathname());
    entity->AddComponent(cp);
}

Entity* CreateLandscapeEnity(const FilePath& scenePathname)
{
    ScopedPtr<Landscape> landscape(new Landscape());
    landscape->SetRenderMode(Landscape::RENDERMODE_NO_INSTANCING);

    // create heightmap
    FilePath heightmapPathname = scenePathname;
    heightmapPathname.ReplaceFilename("landscape.heightmap.png");
    if (CreateHeightmapFile(heightmapPathname, 512u, PixelFormat::FORMAT_A8) == false)
    {
        return nullptr;
    }

    //create geometry
    AABBox3 bboxForLandscape(Vector3(-300.0f, -300.0f, 0.0f), Vector3(300.0f, 300.0f, 50.0f));
    landscape->BuildLandscapeFromHeightmapImage(heightmapPathname, bboxForLandscape);

    //setup textures
    NMaterial* material = landscape->GetMaterial();
    DVASSERT(material != nullptr);
    material->SetQualityGroup(Landscape::LANDSCAPE_QUALITY_NAME);

    auto setupTexture = [&](const String& fileName, const FastName& slotName)
    {
        FilePath textuePathname = scenePathname;
        textuePathname.ReplaceFilename(fileName);
        CreateTextureFiles(textuePathname, 2048u, 2048u, PixelFormat::FORMAT_RGBA8888);

        ScopedPtr<Texture> texture(Texture::CreateFromFile(textuePathname));
        material->AddTexture(slotName, texture);
    };

    setupTexture("landscape.colorTexture.tex", Landscape::TEXTURE_COLOR);
    setupTexture("landscape.tileMask.tex", Landscape::TEXTURE_TILEMASK);
    setupTexture("landscape.tileTexture0.tex", Landscape::TEXTURE_TILE);
    setupTexture("landscape.fullTiledTexture.tex", FastName("fullTiledTexture"));

    Entity* entity = new Entity();
    entity->SetName("landscape");
    RenderComponent* rc = new RenderComponent(landscape);
    entity->AddComponent(rc);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

Entity* CreateWaterEntity(const FilePath& scenePathname)
{
    Entity* entity = new Entity();
    entity->SetName(FastName("water"));

    ScopedPtr<NMaterial> material(CreateMaterial(FastName("water"), NMaterialName::WATER_ALL_QUALITIES));
    material->SetQualityGroup(FastName("Water"));
    ScopedPtr<PolygonGroup> geometry(CreatePolygonGroup());
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    batch->SetMaterial(material);
    batch->SetPolygonGroup(geometry);

    ScopedPtr<RenderObject> ro(new RenderObject());
    ro->AddRenderBatch(batch);
    RenderComponent* rc = new RenderComponent(ro);
    entity->AddComponent(rc);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

Entity* CreateSkyEntity(const FilePath& scenePathname)
{
    Entity* entity = new Entity();
    entity->SetName(FastName("sky"));

    ScopedPtr<NMaterial> material(CreateMaterial(FastName("sky"), NMaterialName::SKYOBJECT));
    ScopedPtr<PolygonGroup> geometry(CreatePolygonGroup());
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    batch->SetMaterial(material);
    batch->SetPolygonGroup(geometry);

    ScopedPtr<RenderObject> ro(new RenderObject());
    ro->AddRenderBatch(batch);
    RenderComponent* rc = new RenderComponent(ro);
    entity->AddComponent(rc);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

Entity* CreateBoxEntity(const FilePath& scenePathname)
{
    Entity* entity = new Entity();
    entity->SetName(FastName("box"));

    ScopedPtr<NMaterial> material(CreateMaterial(FastName("box"), NMaterialName::TEXTURE_LIGHTMAP_OPAQUE));
    ScopedPtr<PolygonGroup> geometry(CreatePolygonGroup());
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    batch->SetMaterial(material);
    batch->SetPolygonGroup(geometry);

    ScopedPtr<RenderObject> ro(new RenderObject());
    ro->AddRenderBatch(batch);
    RenderComponent* rc = new RenderComponent(ro);
    entity->AddComponent(rc);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

Entity* CreateVegetationEntity(const FilePath& scenePathname)
{
    FilePath customGeometryPathname = scenePathname;
    customGeometryPathname.ReplaceBasename("customGeometry");

    { //create custom geometry
        ScopedPtr<Scene> scene(new Scene());

        ScopedPtr<Entity> vegetationGeometry(new Entity());
        vegetationGeometry->SetName(FastName("variation_0"));

        Vector<FastName> VEGETATION_ENTITY_LAYER_NAMES =
        {
          FastName("layer_0"),
          FastName("layer_1"),
          FastName("layer_2"),
          FastName("layer_3")
        };

        for (uint32 i = 0; i < VEGETATION_ENTITY_LAYER_NAMES.size(); ++i)
        {
            ScopedPtr<Entity> vegetationLayer(new Entity());
            vegetationLayer->SetName(VEGETATION_ENTITY_LAYER_NAMES[i]);

            ScopedPtr<NMaterial> material(CreateMaterial(VEGETATION_ENTITY_LAYER_NAMES[i], NMaterialName::TEXTURED_OPAQUE));
            ScopedPtr<PolygonGroup> geometry(CreatePolygonGroup());
            ScopedPtr<RenderBatch> batch(new RenderBatch());
            batch->SetMaterial(material);
            batch->SetPolygonGroup(geometry);

            ScopedPtr<RenderObject> ro(new RenderObject());
            ro->AddRenderBatch(batch);
            RenderComponent* rc = new RenderComponent(ro);
            vegetationLayer->AddComponent(rc);

            vegetationGeometry->AddNode(vegetationLayer);
        }

        scene->AddNode(vegetationGeometry);
        scene->SaveScene(customGeometryPathname, true);
    }

    Entity* entity = new Entity();
    entity->SetName(FastName("vegetation"));

    ScopedPtr<VegetationRenderObject> ro(new VegetationRenderObject());
    ro->SetCustomGeometryPath(customGeometryPathname);

    FilePath lightmapPathname = scenePathname;
    lightmapPathname.ReplaceFilename("vegetation.lightmap.tex");
    CreateTextureFiles(lightmapPathname, 128, 128u, PixelFormat::FORMAT_RGBA8888);
    ro->SetLightmap(lightmapPathname);

    RenderComponent* rc = new RenderComponent(ro);
    entity->AddComponent(rc);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

//TODO: create particels

Entity* CreateCameraEntity(const FilePath& scenePathname)
{
    Entity* entity = new Entity();
    entity->SetName(FastName("camera"));

    ScopedPtr<Camera> camera(new Camera());
    entity->AddComponent(new CameraComponent(camera));

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

Entity* CreateLightsEntity(const FilePath& scenePathname)
{
    Entity* entity = new Entity();
    entity->SetName(FastName("lights"));

    //dynamic light
    ScopedPtr<Entity> dynamicLightEntity(new Entity());
    dynamicLightEntity->SetName(FastName("dynamicLight"));
    ScopedPtr<Light> dynamicLight(new Light());
    dynamicLight->SetType(Light::TYPE_DIRECTIONAL);
    dynamicLight->SetDynamic(true);
    dynamicLightEntity->AddComponent(new LightComponent(dynamicLight));
    entity->AddNode(dynamicLightEntity);

    //not dynamic light
    ScopedPtr<Entity> notDynamicLightEntity(new Entity());
    notDynamicLightEntity->SetName(FastName("notDynamicLight"));
    ScopedPtr<Light> notDynamicLight(new Light());
    notDynamicLight->SetType(Light::TYPE_DIRECTIONAL);
    notDynamicLight->SetDynamic(false);
    notDynamicLightEntity->AddComponent(new LightComponent(notDynamicLight));
    entity->AddNode(notDynamicLightEntity);

    //sky light
    ScopedPtr<Entity> skyLightEntity(new Entity());
    skyLightEntity->SetName(FastName("skyLight"));
    ScopedPtr<Light> skyLight(new Light());
    skyLight->SetType(Light::TYPE_SKY);
    skyLight->SetDynamic(false);
    skyLightEntity->AddComponent(new LightComponent(skyLight));
    entity->AddNode(skyLightEntity);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}

Entity* CreateStaticOcclusionEntity(const FilePath& scenePathname)
{
    Entity* entity = new Entity();
    entity->SetName(FastName("staticOcclusion"));

    StaticOcclusionComponent* so = new StaticOcclusionComponent();
    entity->AddComponent(so);

    CreateR2OCustomProperty(entity, scenePathname);
    return entity;
}
}

void REConsoleModuleTestUtils::ExecuteModule(REConsoleModuleCommon* module)
{
    DVASSERT(module != nullptr);

    module->PostInit();

    while (module->OnFrame() != REConsoleModuleCommon::eFrameResult::FINISHED)
    {
        //game loop
    }

    module->BeforeDestroyed();
}

void REConsoleModuleTestUtils::ClearTestFolder(const DAVA::FilePath& folder)
{
    DVASSERT(folder.IsDirectoryPathname());

    DAVA::FileSystem::Instance()->DeleteDirectoryFiles(folder, true);
    DAVA::FileSystem::Instance()->DeleteDirectory(folder, true);
}

void REConsoleModuleTestUtils::CreateProjectInfrastructure(const DAVA::FilePath& projectPathname)
{
    DAVA::FilePath dataPath = projectPathname + "Data/";
    DAVA::FilePath datasourcePath = projectPathname + "DataSource/3d/";

    DAVA::FileSystem::Instance()->CreateDirectory(dataPath, true);
    DAVA::FileSystem::Instance()->CreateDirectory(datasourcePath, true);

    DAVA::FileSystem::Instance()->CopyFile("~res:/quality.template.yaml", dataPath + "quality.yaml", true);
}

void REConsoleModuleTestUtils::CreateScene(const DAVA::FilePath& scenePathname)
{
    using namespace DAVA;
    ScopedPtr<Scene> scene(new Scene());

    ScopedPtr<Entity> landscape(RECMTUDetail::CreateLandscapeEnity(scenePathname));
    scene->AddNode(landscape);

    ScopedPtr<Entity> water(RECMTUDetail::CreateWaterEntity(scenePathname));
    scene->AddNode(water);

    ScopedPtr<Entity> sky(RECMTUDetail::CreateSkyEntity(scenePathname));
    scene->AddNode(sky);

    ScopedPtr<Entity> box(RECMTUDetail::CreateBoxEntity(scenePathname));
    scene->AddNode(box);

    //    { //Temporary disabled
    //        ScopedPtr<Entity> vegetation(RECMTUDetail::CreateVegetationEntity(scenePathname));
    //        scene->AddNode(vegetation);
    //    }

    ScopedPtr<Entity> camera(RECMTUDetail::CreateCameraEntity(scenePathname));
    scene->AddNode(camera);

    ScopedPtr<Entity> lights(RECMTUDetail::CreateLightsEntity(scenePathname));
    scene->AddNode(lights);

    ScopedPtr<Entity> occlusion(RECMTUDetail::CreateStaticOcclusionEntity(scenePathname));
    scene->AddNode(occlusion);

    FileSystem::Instance()->CreateDirectory(scenePathname.GetDirectory(), true);
    scene->SaveScene(scenePathname, true);
}
