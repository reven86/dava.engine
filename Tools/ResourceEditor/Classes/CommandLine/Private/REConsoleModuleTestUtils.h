#pragma once

class REConsoleModuleCommon;
class REConsoleModuleTestUtils
{
public:
    static void ExecuteModule(REConsoleModuleCommon* module);
    static void ClearTestFolder(const DAVA::FilePath& folder);

    static void CreateProjectInfrastructure(const DAVA::FilePath& projectPathname);
    static void CreateScene(const DAVA::FilePath& scenePathname);
};
