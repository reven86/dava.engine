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

    struct SceneBuilder
    {
        explicit SceneBuilder(const FilePath& scenePathname);
        ~SceneBuilder();

        static void CreateFullScene(const FilePath& scenePathname);

        enum R2OMode
        {
            WITH_REF_TO_OWNER,
            WITHOUT_REF_TO_OWNER
        };

        void CreateScene();
        Entity* AddCamera(R2OMode mode = WITHOUT_REF_TO_OWNER);
        Entity* AddBox(R2OMode mode = WITHOUT_REF_TO_OWNER);
        Entity* AddLandscape(R2OMode mode = WITHOUT_REF_TO_OWNER);
        Entity* AddWater(R2OMode mode = WITHOUT_REF_TO_OWNER);
        Entity* AddSky(R2OMode mode = WITHOUT_REF_TO_OWNER);
        Entity* AddVegetation(R2OMode mode = WITHOUT_REF_TO_OWNER);
        Entity* AddStaticOcclusion(R2OMode mode = WITHOUT_REF_TO_OWNER);
        void AddR2O(Entity*);
        void SaveScene();

        const FilePath scenePathname;
        ScopedPtr<Scene> scene = nullptr;
    };
};
