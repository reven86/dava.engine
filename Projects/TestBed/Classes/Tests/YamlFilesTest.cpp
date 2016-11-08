#include "Tests/YamlFilesTest.h"

#include "Infrastructure/TestBed.h"

using namespace DAVA;

YamlFilesTest::YamlFilesTest(TestBed& app)
    : BaseScreen(app, "YamlFilesTest")
{
}

void YamlFilesTest::LoadResources()
{
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(24.0f);
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    BaseScreen::LoadResources();
    info = new UIStaticText(Rect(0.f, 0.f, static_cast<float32>(screenSize.dx), static_cast<float32>(screenSize.dy)));
    info->SetTextColor(Color::White);
    info->SetFont(font);
    info->SetMultiline(true);
    info->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(info);

    std::wstringstream infoStream;

    BaseScreen::LoadResources();
    allFiles = FileSystem::Instance()->EnumerateFilesInDirectory(inPath);
    for (FilePath file : allFiles)
    {
        try
        {
            yamlNodes.push_back(std::make_pair(YamlParser::Create(file)->GetRootNode(), FilePath(outPath.GetAbsolutePathname() + file.GetFilename())));
        }
        catch (const String& str)
        {
            infoStream << L"find: " << StringToWString(str) << L" in file: " << StringToWString(file.GetFilename()) << L"\n";
        }
    }
    info->SetText(infoStream.str());

    //TODO: Initialize resources here
}

void YamlFilesTest::UnloadResources()
{
    bool res = true;
    FilePath path(FileSystem::Instance()->GetUserDocumentsPath() + "/out/");
    if (FileSystem::Instance()->Exists(path))
    {
        FileSystem::Instance()->DeleteDirectory(path);
        FileSystem::Instance()->CreateDirectoryW(StringToWString(path.GetAbsolutePathname()));
    }
    else
    {
        FileSystem::Instance()->CreateDirectoryW(StringToWString(path.GetAbsolutePathname()));
    }

    for (std::pair<YamlNode*, FilePath> pair : yamlNodes)
    {
        res &= YamlEmitter::SaveToYamlFile(path + pair.second.GetFilename(), pair.first);
    }
    DVASSERT(res);
    BaseScreen::UnloadResources();
    //TODO: Release resources here
}
