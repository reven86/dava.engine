#pragma once

#include "Infrastructure/BaseScreen.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"

class TestBed;
class YamlFilesTest : public BaseScreen
{
public:
    YamlFilesTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;
    FilePath inPath = "~res:/yamlFiles/in/";
    FilePath outPath = "~res:/yamlFiles/out/";
    Vector<FilePath> allFiles;
    Vector<std::pair<YamlNode*, FilePath>> yamlNodes;
    DAVA::UIStaticText* info = nullptr;
};
