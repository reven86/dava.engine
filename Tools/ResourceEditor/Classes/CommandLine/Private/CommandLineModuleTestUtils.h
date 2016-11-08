#pragma once

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

#include <memory>

namespace DAVA
{
class FilePath;
};

class CommandLineModule;
class CommandLineModuleTestUtils
{
public:
    class TextureLoadingGuard final
    {
    public:
        TextureLoadingGuard(const DAVA::Vector<DAVA::eGPUFamily>& newLoadingOrder);
        ~TextureLoadingGuard();

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

    static std::unique_ptr<TextureLoadingGuard> CreateTextureGuard(const DAVA::Vector<DAVA::eGPUFamily>& newLoadingOrder);

    static void ExecuteModule(CommandLineModule* module);
    static void InitModule(CommandLineModule* module);
    static bool ProcessModule(CommandLineModule* module);
    static void FinalizeModule(CommandLineModule* module);

    static void CreateTestFolder(const DAVA::FilePath& folder);
    static void ClearTestFolder(const DAVA::FilePath& folder);

    static void CreateProjectInfrastructure(const DAVA::FilePath& projectPathname);
    static void CreateScene(const DAVA::FilePath& scenePathname);
};
