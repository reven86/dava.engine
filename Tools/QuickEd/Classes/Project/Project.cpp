#include "DAVAEngine.h"

#include <QDir>
#include <QApplication>
#include <QVariant>
#include <QMessageBox>

#include "Project.h"
#include "EditorFontSystem.h"
#include "UI/UIPackageLoader.h"
#include "UI/DefaultUIPackageBuilder.h"
#include "Model/EditorUIPackageBuilder.h"
#include "Model/LegacyEditorUIPackageLoader.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageRef.h"
#include "Helpers/ResourcesManageHelper.h"
#include "Project/EditorFontSystem.h"

using namespace DAVA;

Project::Project(QObject *parent)
    : QObject(parent)
    , Singleton<Project>()
    , editorFontSystem(new EditorFontSystem(this))
    , isOpen(false)
{
    legacyData = new LegacyControlData();
}

Project::~Project()
{
    SafeRelease(legacyData);
}

bool Project::Open(const QString &path)
{
    bool result = OpenInternal(path);
    SetIsOpen(result);
    return result;
}

bool Project::OpenInternal(const QString &path)
{
    // Attempt to create a project
    YamlParser* parser = YamlParser::Create(path.toStdString());
    if (!parser)
        return false;
    
    QDir dir(path);
    dir.cdUp();
    projectDir = dir.absolutePath();
    
    YamlNode* projectRoot = parser->GetRootNode();
    if (!projectRoot)
    {
        SafeRelease(parser);
        return false;
    }
    
    // Build the list of file names to be locked.
    List<QString> fileNames;
    fileNames.push_back(path);
    
    LocalizationSystem::Instance()->Cleanup();
    
    FilePath bundleName(projectDir.toStdString());
    bundleName.MakeDirectoryPathname();
    
    List<FilePath> resFolders = FilePath::GetResourcesFolders();
    List<FilePath>::const_iterator searchIt = find(resFolders.begin(), resFolders.end(), bundleName);
    
    if(searchIt == resFolders.end())
    {
        FilePath::AddResourcesFolder(bundleName);
    }
      
    const YamlNode *fontNode = projectRoot->Get("font");
    
    // Get font node
    if (nullptr != fontNode)
    {
        // Get default font node
        const YamlNode *defaultFontPath = fontNode->Get("DefaultFontsPath");
        if (nullptr != defaultFontPath)
        {        
            FilePath localizationFontsPath(defaultFontPath->AsString());
            if(localizationFontsPath.Exists())
            {
                editorFontSystem->SetDefaultFontsPath(localizationFontsPath.GetDirectory());
            }
        }
    }
    
    if(editorFontSystem->GetDefaultFontsPath().IsEmpty())
    {
        editorFontSystem->SetDefaultFontsPath(FilePath(bundleName.GetAbsolutePathname() + "Data/UI/Fonts/"));
    }

    editorFontSystem->LoadLocalizedFonts();

    FontManager::Instance()->PrepareToSaveFonts(true);
    
    const YamlNode* platforms = projectRoot->Get("platforms");
    for (uint32 i = 0; i < platforms->GetCount(); i++)
    {
        const String &platformName = platforms->GetItemKeyName(i);
        if (platformName.empty())
            continue;
        const YamlNode *platform = platforms->Get(platformName);
        float platformWidth = platform->Get("width")->AsFloat();
        float platformHeight = platform->Get("height")->AsFloat();
        
        const YamlNode *screens = platform->Get("screens");
        for (int j = 0; j < (int32) screens->GetCount(); j++)
        {
            const String &screenName = screens->Get(j)->AsString();
            LegacyControlData::Data data;
            data.name = screenName;
            data.isAggregator = false;
            data.size = Vector2(platformWidth, platformHeight);
            String key = "~res:/UI/" + platformName + "/" + screenName + ".yaml";
            legacyData->Put(key, data);
        }
        
        const YamlNode *aggregators = platform->Get("aggregators");
        for (int j = 0; j < (int32) aggregators->GetCount(); j++)
        {
            String aggregatorName = aggregators->GetItemKeyName(j);
            const YamlNode *aggregator = aggregators->Get(j);
            float aggregatorWidth = aggregator->Get("width")->AsFloat();
            float aggregatorHeight = aggregator->Get("height")->AsFloat();

            LegacyControlData::Data data;
            data.name = aggregatorName;
            data.isAggregator = false;
            data.size = Vector2(aggregatorWidth, aggregatorHeight);
            String key = "~res:/UI/" + platformName + "/" + aggregatorName + ".yaml";
            legacyData->Put(key, data);
        }
        
        if (i == 0)
        {
            const YamlNode *localizationPathNode = platform->Get("LocalizationPath");
            const YamlNode *localeNode = platform->Get("Locale");
            if (localizationPathNode && localeNode)
            {
                LocalizationSystem::Instance()->SetDirectory(localizationPathNode->AsString());
                LocalizationSystem::Instance()->SetCurrentLocale(localeNode->AsString());
                LocalizationSystem::Instance()->Init();
            }
        }
    }
    
    SafeRelease(parser);
    
    return true;
}

bool Project::CheckAndUnlockProject(const QString& projectPath)
{
    if (!FileSystem::Instance()->IsFileLocked(projectPath.toStdString()))
    {
        // Nothing to unlock.
        return true;
    }

    if (QMessageBox::question(qApp->activeWindow(), tr("File is locked!"), tr("The project file %1 is locked by other user. Do you want to unlock it?")) == QMessageBox::No)
    {
        return false;
    }

    // Check whether it is possible to unlock project file.
    if (!FileSystem::Instance()->LockFile(projectPath.toStdString(), false))
    {
        QMessageBox::critical(qApp->activeWindow(), tr("Unable to unlock project file!"),
            tr("Unable to unlock project file %1. Please check whether the project is opened in another QuickEd and close it, if yes.").arg(projectPath));
        return false;
    }

    return true;
}

DAVA::RefPtr<PackageNode> Project::NewPackage(const QString &path)
{
    return DAVA::RefPtr<PackageNode>();
}

RefPtr<PackageNode> Project::OpenPackage(const QString &packagePath)
{
    FilePath path(packagePath.toStdString());
    String fwPath = path.GetFrameworkPath();

    EditorUIPackageBuilder builder;
    UIPackage *newPackage = UIPackageLoader(&builder).LoadPackage(path);
    if (nullptr == newPackage)
    {
        newPackage = LegacyEditorUIPackageLoader(&builder, legacyData).LoadPackage(path);
    }

    if (nullptr != newPackage)
    {
        SafeRelease(newPackage);
        return builder.GetPackageNode();
    }
    return RefPtr<PackageNode>();
}

bool Project::SavePackage(PackageNode *package)
{
    YamlPackageSerializer serializer;
    package->Serialize(&serializer);
    serializer.WriteToFile(package->GetPackageRef()->GetPath());
    return true;
}

EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem;
}


bool Project::IsOpen() const
{
    return isOpen;
}

void Project::SetIsOpen(bool arg)
{
    if (isOpen == arg)
    {
        return;
    }
    isOpen = arg;
    if (arg)
    {
        ResourcesManageHelper::SetProjectPath(projectDir);
    }
    emit IsOpenChanged(arg);
}