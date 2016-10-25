#pragma once

#include <memory>

class REConsoleModuleCommon;
class REConsoleModuleTestUtils
{
public:
    class TextureLoadingGuard final
    {
    public:
        TextureLoadingGuard(const DAVA::Vector<DAVA::eGPUFamily>& newLoadingOrder);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

    static TextureLoadingGuard CreateTextureGuard(const DAVA::Vector<DAVA::eGPUFamily>& newLoadingOrder);

    static void ExecuteModule(REConsoleModuleCommon* module);
    static void ClearTestFolder(const DAVA::FilePath& folder);

    static void CreateProjectInfrastructure(const DAVA::FilePath& projectPathname);
    static void CreateScene(const DAVA::FilePath& scenePathname);
};
