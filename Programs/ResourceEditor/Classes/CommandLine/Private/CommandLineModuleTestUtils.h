#pragma once

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Scene3D/Scene.h"
#include "FileSystem/FilePath.h"

#include <memory>

namespace DAVA
{
class FilePath;
};

namespace CommandLineModuleTestUtils
{
using namespace DAVA;

class TextureLoadingGuard final
{
public:
    TextureLoadingGuard(const Vector<eGPUFamily>& newLoadingOrder);
    ~TextureLoadingGuard();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

std::unique_ptr<TextureLoadingGuard> CreateTextureGuard(const Vector<eGPUFamily>& newLoadingOrder);

void CreateTestFolder(const FilePath& folder);
void ClearTestFolder(const FilePath& folder);

void CreateProjectInfrastructure(const FilePath& projectPathname);

class SceneBuilder
{
public:
    /*
    creates scene
    */
    explicit SceneBuilder(const FilePath& scenePathname);

    /*
    saves scene into scenePathname passed to constructor
    */
    ~SceneBuilder();

    /*
    creates scene with all objects that can be added with SceneBuilder: camera, box etc.
    */
    static void CreateFullScene(const FilePath& scenePathname);

    enum R2OMode
    {
        WITH_REF_TO_OWNER,
        WITHOUT_REF_TO_OWNER
    };

    Entity* AddCamera(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddBox(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddLandscape(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddWater(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddSky(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddVegetation(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddLights(R2OMode mode = WITHOUT_REF_TO_OWNER);
    Entity* AddStaticOcclusion(R2OMode mode = WITHOUT_REF_TO_OWNER);

    /*
    adds 'reference to owner' scene to passed `entity`
    */
    void AddR2O(Entity* entity);

    const FilePath scenePathname;
    ScopedPtr<Scene> scene;
};
};
