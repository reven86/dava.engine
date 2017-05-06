#include "Modules/UIPreviewModule/UIPreviewModule.h"
#include "Modules/UIPreviewModule/Private/UIPreviewPackageBuilder.h"

#include "Modules/ProjectModule/ProjectData.h"

#include "UI/Find/PackageInformation/StaticPackageInformationBuilder.h"
#include "UI/Find/PackageInformation/ControlNodeInformation.h"
#include "UI/Find/PackageInformation/PackageNodeInformation.h"

#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <FileSystem/FilePath.h>
#include <FileSystem/FileList.h>
#include <FileSystem/KeyedArchive.h>
#include <Functional/Function.h>
#include <Logger/Logger.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Time/SystemTimer.h>
#include <UI/UIPackageLoader.h>
#include <Utils/CRC32.h>

#include <QDockWidget>

DAVA_VIRTUAL_REFLECTION_IMPL(UIPreviewModule)
{
    DAVA::ReflectionRegistrator<UIPreviewModule>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace UIPreviewModuleDetails
{
class ScopedTimeMeasure
{
public:
    ScopedTimeMeasure(const DAVA::char8* scopeName_)
        : scopeName(scopeName_)
    {
        timestamp = DAVA::SystemTimer::GetMs();
    }

    ~ScopedTimeMeasure()
    {
        DAVA::int64 delta = DAVA::SystemTimer::GetMs() - timestamp;
        DAVA::Logger::Info(":: %s takes %lld ms", scopeName, delta);
    }

private:
    const DAVA::char8* scopeName;
    DAVA::int64 timestamp = 0;
};

struct FileDescriptor
{
    FileDescriptor(const DAVA::String& relativePath_, DAVA::uint32 crc_, DAVA::uint64 size_)
        : relativePath(relativePath_)
        , crc(crc_)
        , size(size_)
    {
    }
    DAVA::String relativePath;
    DAVA::uint32 crc = 0;
    DAVA::uint64 size = 0;
};

void EnumerateFile(const DAVA::FilePath& path, const DAVA::FilePath& rootFolder, DAVA::List<FileDescriptor>& files)
{
    using namespace DAVA;

    DAVA::uint64 fSize = 0;

    { //get size for debug info
        //        ScopedPtr<File> f(File::Create(path, DAVA::File::OPEN | DAVA::File::READ));
        //        if (f)
        //        {
        //            fSize = f->GetSize();
        //        }
    }

    files.emplace_back(path.GetRelativePathname(rootFolder), CRC32::ForFile(path), fSize);
}

void EnumerateFiles(const DAVA::FilePath& folder, const DAVA::FilePath& rootFolder, DAVA::List<FileDescriptor>& files, bool recursive)
{
    using namespace DAVA;

    ScopedPtr<FileList> fl(new FileList(folder, false));
    for (uint32 index = 0; index < fl->GetCount(); ++index)
    {
        if (fl->IsNavigationDirectory(index))
        {
            continue;
        }

        const FilePath& path = fl->GetPathname(index);
        if (fl->IsDirectory(index))
        {
            if (recursive)
            {
                EnumerateFiles(path, rootFolder, files, recursive);
            }
        }
        else
        {
            EnumerateFile(path, rootFolder, files);
        }
    }
}

void EnumerateEmitter(DAVA::ParticleEmitterInstance* instance, DAVA::Set<DAVA::FilePath>& pathCollection, DAVA::Set<DAVA::FilePath>& spriteFolders)
{
    using namespace DAVA;
    DVASSERT(nullptr != instance);

    ParticleEmitter* emitter = instance->GetEmitter();
    pathCollection.insert(emitter->configPath);

    for (ParticleLayer* layer : emitter->layers)
    {
        DVASSERT(nullptr != layer);

        if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            ScopedPtr<ParticleEmitterInstance> instance(new ParticleEmitterInstance(nullptr, layer->innerEmitter, true));
            EnumerateEmitter(instance, pathCollection, spriteFolders);
        }
        else
        {
            Sprite* sprite = layer->sprite;
            if (sprite != nullptr)
            {
                FilePath tmp = sprite->GetRelativePathname().GetAbsolutePathname();
                spriteFolders.insert(tmp.GetDirectory());
            }
        }
    }
}

void EnumerateEffect(const DAVA::FilePath& sc2Path, DAVA::Set<DAVA::FilePath>& pathCollection, DAVA::Set<DAVA::FilePath>& spriteFolders)
{
    using namespace DAVA;
    if (pathCollection.count(sc2Path) > 0)
    {
        return;
    }
    pathCollection.insert(sc2Path);

    ScopedPtr<Scene> scene(new Scene());
    if (SceneFileV2::ERROR_NO_ERROR != scene->LoadScene(sc2Path))
    {
        Logger::Error("[%s] Can't open file %s", __FUNCTION__, sc2Path.GetStringValue().c_str());
        return;
    }

    Vector<Entity*> entities;
    scene->GetChildEntitiesWithComponent(entities, Component::PARTICLE_EFFECT_COMPONENT);
    for (Entity* e : entities)
    {
        uint32 count = e->GetComponentCount(Component::PARTICLE_EFFECT_COMPONENT);
        for (uint32 c = 0; c < count; ++c)
        {
            ParticleEffectComponent* effect = static_cast<ParticleEffectComponent*>(e->GetComponent(Component::PARTICLE_EFFECT_COMPONENT, c));

            const DAVA::int32 emittersCount = effect->GetEmittersCount();
            for (DAVA::int32 em = 0; em < emittersCount; ++em)
            {
                EnumerateEmitter(effect->GetEmitterInstance(em), pathCollection, spriteFolders);
            }
        }
    }
}
}

void UIPreviewModule::PostInit()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    {
        QtAction* action = new QtAction(GetAccessor(), QString("Collect All Files"));
        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &UIPreviewModule::CollectAllFiles));
        FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = DAVA::FastName(ProjectData::projectPathPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
            return !fieldValue.Cast<DAVA::FilePath>(DAVA::FilePath()).IsEmpty();
        });

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("UIPreview", { InsertionParams::eInsertionMethod::AfterItem }));
        GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }

    {
        QtAction* action = new QtAction(GetAccessor(), QString("Collect Used Files"));
        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &UIPreviewModule::CollectUsedFiles));
        FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = DAVA::FastName(ProjectData::projectPathPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
            return !fieldValue.Cast<DAVA::FilePath>(DAVA::FilePath()).IsEmpty();
        });

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("UIPreview", { InsertionParams::eInsertionMethod::AfterItem }));
        GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }
}

void UIPreviewModule::CollectAllFiles()
{
    using namespace DAVA;
    using namespace UIPreviewModuleDetails;

    ScopedTimeMeasure tm(__FUNCTION__);

    List<FileDescriptor> collectedFiles;

    ProjectData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
    DVASSERT(nullptr != projectData);
    const FilePath& projectFolder = projectData->GetProjectDirectory();

    //fonts
    EnumerateFiles(projectData->GetFontsDirectory().absolute, projectFolder, collectedFiles, true);
    //    EnumerateFiles(projectData->GetFontsConfigsDirectory().absolute, projectFolder, collectedFiles); // it is included into UI directory

    //strings
    EnumerateFiles(projectData->GetTextsDirectory().absolute, projectFolder, collectedFiles, true);

    //particles
    EnumerateFiles(projectFolder + "/DataSource/3d/FX/UI/", projectFolder, collectedFiles, true);

    //sounds
    EnumerateFiles(projectFolder + "/DataSource/Sfx/", projectFolder, collectedFiles, true);
    EnumerateFile(projectFolder + "/DataSource/sounds.yaml", projectFolder, collectedFiles);

    //UI
    EnumerateFiles(projectData->GetUiDirectory().absolute, projectFolder, collectedFiles, true);

    //configs
    EnumerateFiles(projectFolder + "/DataSource/Configs/", projectFolder, collectedFiles, true);

    //sprites
    const FilePath& resDirectory = projectData->GetConvertedResourceDirectory().absolute;
    const Vector<ProjectData::GfxDir>& gfxDirs = projectData->GetGfxDirectories();
    for (const ProjectData::GfxDir& gfx : gfxDirs)
    {
        EnumerateFiles(resDirectory + gfx.directory.relative, projectFolder, collectedFiles, true);
    }

    //DUMP
    uint64 totalSize = 0;
    for (const FileDescriptor& fd : collectedFiles)
    {
        totalSize += fd.size;
    }

    Logger::Info("*** Files Count = %d, File Size = %lld", static_cast<uint32>(collectedFiles.size()), totalSize);
}

void UIPreviewModule::CollectUsedFiles()
{
    using namespace DAVA;
    using namespace UIPreviewModuleDetails;

    ScopedTimeMeasure tm(__FUNCTION__);

    ProjectData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
    DVASSERT(nullptr != projectData);
    const FilePath& projectFolder = projectData->GetProjectDirectory();

    List<FileDescriptor> collectedFiles;

    { //project depended files
        ScopedTimeMeasure tt("Setup Environment");

        //        //fonts
        //        EnumerateFiles(projectData->GetFontsDirectory().absolute, projectFolder, collectedFiles, true);
        //        EnumerateFiles(projectData->GetFontsConfigsDirectory().absolute, projectFolder, collectedFiles, true);
        //
        //        //strings
        //        EnumerateFiles(projectData->GetTextsDirectory().absolute, projectFolder, collectedFiles, true);
        //
        //        //sounds
        //        EnumerateFiles(projectFolder + "/DataSource/Sfx/", projectFolder, collectedFiles, true);
        //        EnumerateFile(projectFolder + "/DataSource/sounds.yaml", projectFolder, collectedFiles);
    }

    FilesCollection collector;
    UIPreviewPackageBuilder builder(&collector);
    UIPackageLoader(projectData->GetPrototypes()).LoadPackage("~res:/UI/Screens/Battle/BattleLoadingScreen.yaml", &builder);
    UIPackageLoader(projectData->GetPrototypes()).LoadPackage("~res:/UI/Styles/ButtonSoundStyles.yaml", &builder);
    UIPackageLoader(projectData->GetPrototypes()).LoadPackage("~res:/UI/Styles/ButtonStyle.yaml", &builder);
    UIPackageLoader(projectData->GetPrototypes()).LoadPackage("~res:/UI/Screens/Common/BattleButton.yaml", &builder);

    {
        for (const FilePath& path : collector.yamlFiles)
        {
            collectedFiles.emplace_back(path.GetRelativePathname(projectFolder), CRC32::ForFile(path), 0);
        }

        DAVA::Set<DAVA::FilePath> pathCollection;
        for (const FilePath& path : collector.effectsFiles)
        {
            EnumerateEffect(path, pathCollection, collector.spritesFolders);
        }
        for (const FilePath& path : pathCollection)
        {
            collectedFiles.emplace_back(path.GetRelativePathname(projectFolder), CRC32::ForFile(path), 0);
        }

        for (const FilePath& path : collector.spritesFolders)
        {
            EnumerateFiles(path, projectFolder, collectedFiles, false);
        }
    }

    uint64 totalSize = 0;
    for (const FileDescriptor& fd : collectedFiles)
    {
        Logger::Info("__ __ %s", fd.relativePath.c_str());

        totalSize += fd.size;
    }
    Logger::Info("*** Files Count = %d, File Size = %lld", static_cast<uint32>(collectedFiles.size()), totalSize);
}

#if !defined(DEPLOY_BUILD)
DECL_GUI_MODULE(UIPreviewModule);
#endif
