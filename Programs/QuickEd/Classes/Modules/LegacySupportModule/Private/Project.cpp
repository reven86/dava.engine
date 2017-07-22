#include "Modules/LegacySupportModule/Private/Project.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/YamlPackageSerializer.h"
#include "Modules/ProjectModule/Private/EditorLocalizationSystem.h"
#include "Modules/ProjectModule/Private/EditorFontSystem.h"
#include "Modules/FileSystemCacheModule/FileSystemCacheData.h"

#include "Sound/SoundSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "UI/ProjectView.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/OperationInvoker.h>
#include <TArc/DataProcessing/DataContext.h>

#include <QtTools/ReloadSprites/SpritesPacker.h>
#include <QtTools/ProjectInformation/FileSystemCache.h>
#include <QtTools/FileDialogs/FindFileDialog.h>

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/YamlEmitter.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlParser.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/UIControlSystem.h>
#include <Utils/Utils.h>

#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

using namespace DAVA;

Project::Project(MainWindow::ProjectView* view_, DAVA::TArc::ContextAccessor* accessor_)
    : view(view_)
    , editorFontSystem(new EditorFontSystem())
    , editorLocalizationSystem(new EditorLocalizationSystem())
    , accessor(accessor_)
{
    using namespace TArc;
    DataContext* globalContext = accessor->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);
    projectDirectory = QString::fromStdString(projectData->GetProjectDirectory().GetStringValue());
    projectName = QString::fromStdString(projectData->GetProjectFile().GetFilename());

    if (!projectData->GetFontsConfigsDirectory().absolute.IsEmpty()) //for support legacy empty project
    {
        editorFontSystem->SetDefaultFontsPath(projectData->GetFontsConfigsDirectory().absolute);
        editorFontSystem->LoadLocalizedFonts();
    }

    if (!projectData->GetTextsDirectory().absolute.IsEmpty()) //support legacy empty project
    {
        editorLocalizationSystem->SetDirectory(QDir(QString::fromStdString(projectData->GetTextsDirectory().absolute.GetStringValue())));
    }

    if (!projectData->GetDefaultLanguage().empty()) //support legacy empty project
    {
        editorLocalizationSystem->SetCurrentLocale(QString::fromStdString(projectData->GetDefaultLanguage()));
    }

    view->SetProjectActionsEnabled(true);
    view->SetProjectPath(GetProjectPath());
    view->SetLanguages(GetAvailableLanguages(), GetCurrentLanguage());
    view->OnProjectChanged(this);

    connections.AddConnection(editorLocalizationSystem.get(), &EditorLocalizationSystem::CurrentLocaleChanged, MakeFunction(view, &MainWindow::ProjectView::SetCurrentLanguage));
    connections.AddConnection(editorFontSystem.get(), &EditorFontSystem::FontPresetChanged, MakeFunction(this, &Project::OnFontPresetChanged));

    connections.AddConnection(view, &MainWindow::ProjectView::CurrentLanguageChanged, MakeFunction(this, &Project::SetCurrentLanguage));
    connections.AddConnection(view, &MainWindow::ProjectView::RtlChanged, MakeFunction(this, &Project::SetRtl));
    connections.AddConnection(view, &MainWindow::ProjectView::BiDiSupportChanged, MakeFunction(this, &Project::SetBiDiSupport));
    connections.AddConnection(view, &MainWindow::ProjectView::GlobalStyleClassesChanged, MakeFunction(this, &Project::SetGlobalStyleClasses));

    QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
    const EngineContext* engineContext = accessor->GetEngineContext();
    engineContext->soundSystem->InitFromQualitySettings();
}

Project::~Project()
{
    const EngineContext* engineContext = GetEngineContext();
    engineContext->soundSystem->UnloadFMODProjects();

    view->SetLanguages(QStringList(), QString());
    view->SetProjectPath(QString());
    view->SetProjectActionsEnabled(false);

    view->OnProjectChanged(nullptr);

    editorLocalizationSystem->Cleanup();
    editorFontSystem->ClearAllFonts();
}

Vector<ProjectData::ResDir> Project::GetLibraryPackages() const
{
    ProjectData* projectData = accessor->GetGlobalContext()->GetData<ProjectData>();
    return projectData->GetLibraryPackages();
}

const Map<String, Set<DAVA::FastName>>& Project::GetPrototypes() const
{
    ProjectData* projectData = accessor->GetGlobalContext()->GetData<ProjectData>();
    return projectData->GetPrototypes();
}

const QString& Project::GetGraphicsFileExtension()
{
    static const QString filter(".psd");
    return filter;
}

const QString& Project::Get3dFileExtension()
{
    static const QString filter(".sc2");
    return filter;
}

const QString& Project::GetUiFileExtension()
{
    static const QString extension(".yaml");
    return extension;
}

QStringList Project::GetAvailableLanguages() const
{
    return editorLocalizationSystem->GetAvailableLocales();
}

QString Project::GetCurrentLanguage() const
{
    return editorLocalizationSystem->GetCurrentLocale();
}

void Project::SetCurrentLanguage(const QString& newLanguageCode)
{
    editorLocalizationSystem->SetCurrentLocale(newLanguageCode);
    editorFontSystem->RegisterCurrentLocaleFonts();

    accessor->ForEachContext([](DAVA::TArc::DataContext& context)
                             {
                                 DocumentData* data = context.GetData<DocumentData>();
                                 DVASSERT(data != nullptr);
                                 data->RefreshAllControlProperties();
                                 data->RefreshLayout();
                             });
}

const QStringList& Project::GetDefaultPresetNames() const
{
    return editorFontSystem->GetDefaultPresetNames();
}

EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem.get();
}

void Project::SetRtl(bool isRtl)
{
    const EngineContext* engineContext = GetEngineContext();
    engineContext->uiControlSystem->SetRtl(isRtl);

    accessor->ForEachContext([](DAVA::TArc::DataContext& context)
                             {
                                 DocumentData* data = context.GetData<DocumentData>();
                                 DVASSERT(data != nullptr);
                                 data->RefreshAllControlProperties();
                                 data->RefreshLayout();
                             });
}

void Project::SetBiDiSupport(bool support)
{
    const EngineContext* engineContext = GetEngineContext();
    engineContext->uiControlSystem->SetBiDiSupportEnabled(support);

    accessor->ForEachContext([](DAVA::TArc::DataContext& context)
                             {
                                 DocumentData* data = context.GetData<DocumentData>();
                                 DVASSERT(data != nullptr);
                                 data->RefreshAllControlProperties();
                                 data->RefreshLayout();
                             });
}

void Project::SetGlobalStyleClasses(const QString& classesStr)
{
    Vector<String> tokens;
    Split(classesStr.toStdString(), " ", tokens);
    const EngineContext* engineContext = GetEngineContext();
    UIControlSystem* uiControlSystem = engineContext->uiControlSystem;
    uiControlSystem->GetStyleSheetSystem()->ClearGlobalClasses();
    for (String& token : tokens)
    {
        uiControlSystem->GetStyleSheetSystem()->AddGlobalClass(FastName(token));
    }

    accessor->ForEachContext([](DAVA::TArc::DataContext& context)
                             {
                                 DocumentData* data = context.GetData<DocumentData>();
                                 DVASSERT(data != nullptr);
                                 data->RefreshAllControlProperties();
                                 data->RefreshLayout();
                             });
}

QString Project::GetResourceDirectory() const
{
    ProjectData* projectData = accessor->GetGlobalContext()->GetData<ProjectData>();
    return QString::fromStdString(projectData->GetResourceDirectory().absolute.GetStringValue());
}

QString Project::GetProjectPath() const
{
    ProjectData* projectData = accessor->GetGlobalContext()->GetData<ProjectData>();
    return QString::fromStdString(projectData->GetProjectFile().GetStringValue());
}

const QString& Project::GetProjectDirectory() const
{
    return projectDirectory;
}

const QString& Project::GetProjectName() const
{
    return projectName;
}

void Project::OnFontPresetChanged()
{
    accessor->ForEachContext([](DAVA::TArc::DataContext& context)
                             {
                                 DocumentData* data = context.GetData<DocumentData>();
                                 DVASSERT(data != nullptr);
                                 data->RefreshAllControlProperties();
                             });
}
