#include "Project.h"

#include "Document/Document.h"
#include "Document/DocumentGroup.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/YamlPackageSerializer.h"
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"
#include "UI/ProjectView.h"

#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/ProjectInformation/ProjectStructure.h"
#include "QtTools/FileDialogs/FindFileDialog.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"

#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

using namespace DAVA;

Project::Project(MainWindow::ProjectView* aView, const ProjectProperties& aSettings)
    : QObject(nullptr)
    , view(aView)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , documentGroup(new DocumentGroup(this, view->GetDocumentGroupView()))
    , spritesPacker(new SpritesPacker())
    , projectStructure(new ProjectStructure(QStringList() << "yaml"))
    , properties(aSettings)
    , projectDirectory(QString::fromStdString(aSettings.projectFile.GetDirectory().GetAbsolutePathname()))
    , projectName(QString::fromStdString(aSettings.projectFile.GetFilename()))
{
    if (FileSystem::Instance()->IsDirectory(properties.GetAdditionalResourceDirectory()))
    {
        FilePath::AddResourcesFolder(properties.GetAdditionalResourceDirectory());
    }

    FilePath::AddResourcesFolder(properties.GetIntermediateResourceDirectory());
    FilePath::AddResourcesFolder(properties.GetResourceDirectory());

    editorFontSystem->SetDefaultFontsPath(properties.GetFontsConfigsDirectory());
    editorFontSystem->LoadLocalizedFonts();

    editorLocalizationSystem->SetDirectory(QDir(QString::fromStdString(properties.GetTextsDirectory().GetAbsolutePathname())));
    editorLocalizationSystem->SetCurrentLocale(QString::fromStdString(properties.defaultLanguage));

    FilePath uiDirectory = properties.GetUiDirectory();
    if (!FileSystem::Instance()->IsDirectory(uiDirectory))
    {
        uiDirectory = properties.GetUiDirectory(true);
    }
    DVASSERT(FileSystem::Instance()->IsDirectory(uiDirectory));
    uiResourcesPath = QString::fromStdString(uiDirectory.GetAbsolutePathname());

    projectStructure->AddProjectDirectory(uiResourcesPath);
    view->SetResourceDirectory(uiResourcesPath);

    view->SetProjectActionsEnabled(true);
    view->SetProjectPath(GetProjectPath());
    view->SetLanguages(GetAvailableLanguages(), GetCurrentLanguage());

    connect(editorLocalizationSystem.get(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &Project::CurrentLanguageChanged, Qt::DirectConnection);
    connect(editorFontSystem.get(), &EditorFontSystem::FontPresetChanged, documentGroup.get(), &DocumentGroup::FontPresetChanged, Qt::DirectConnection);

    connect(view, &MainWindow::ProjectView::CurrentLanguageChanged, this, &Project::SetCurrentLanguage);
    connect(view, &MainWindow::ProjectView::RtlChanged, this, &Project::SetRtl);
    connect(view, &MainWindow::ProjectView::BiDiSupportChanged, this, &Project::SetBiDiSupport);
    connect(view, &MainWindow::ProjectView::GlobalStyleClassesChanged, this, &Project::SetGlobalStyleClasses);
    connect(view, &MainWindow::ProjectView::ReloadSprites, this, &Project::OnReloadSprites);
    connect(view, &MainWindow::ProjectView::FindFileInProject, this, &Project::FindFileInProject);

    connect(this, &Project::CurrentLanguageChanged, view, &MainWindow::ProjectView::SetCurrentLanguage);

    connect(spritesPacker.get(), &SpritesPacker::Finished, this, &Project::OnReloadSpritesFinished);

    spritesPacker->ClearTasks();
    for (const auto& gfxOptions : properties.gfxDirectories)
    {
        QDir gfxDirectory(QString::fromStdString(gfxOptions.first));
        DVASSERT(gfxDirectory.exists());

        String uiRelativeDir = FilePath(gfxOptions.first).GetRelativePathname(properties.resourceDirectory);
        FilePath gfxOutDir = FilePath(properties.intermediateResourceDirectory) + uiRelativeDir;
        QDir gfxOutDirectory(QString::fromStdString(gfxOutDir.GetAbsolutePathname()));

        spritesPacker->AddTask(gfxDirectory, gfxOutDirectory);
    }
}

Project::~Project()
{
    view->SetLanguages(QStringList(), QString());
    view->SetProjectPath(QString());
    view->SetProjectActionsEnabled(false);

    projectStructure->RemoveProjectDirectory(uiResourcesPath);
    view->SetResourceDirectory(QString());

    editorLocalizationSystem->Cleanup();
    editorFontSystem->ClearAllFonts();
    FilePath::RemoveResourcesFolder(properties.GetResourceDirectory());
    FilePath::RemoveResourcesFolder(properties.GetAdditionalResourceDirectory());
    FilePath::RemoveResourcesFolder(properties.GetIntermediateResourceDirectory());
}

Vector<FilePath> Project::GetLibraryPackages() const
{
    return properties.GetLibraryPackages();
    //return properties.libraryPackages;
}

std::tuple<ResultList, ProjectProperties> Project::ParseProjectPropertiesFromFile(const QString& projectFile)
{
    ResultList resultList;

    RefPtr<YamlParser> parser(YamlParser::Create(projectFile.toStdString()));
    if (parser.Get() == nullptr || parser->GetRootNode() == nullptr)
    {
        QString message = tr("Can not parse project file %1.").arg(projectFile);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return std::make_tuple(resultList, ProjectProperties());
    }

    return ProjectProperties::Parse(projectFile.toStdString(), parser->GetRootNode());
}

bool Project::EmitProjectPropertiesToFile(const ProjectProperties& properties)
{
    RefPtr<YamlNode> node = ProjectProperties::Emit(properties);

    return YamlEmitter::SaveToYamlFile(FilePath(properties.projectFile), node.Get());
}

const QString& Project::GetProjectFileName()
{
    static const QString projectFile("ui.quicked");
    return projectFile;
}

const QStringList& Project::GetFontsFileExtensionFilter()
{
    static const QStringList filter(QStringList() << "*.ttf"
                                                  << "*.otf"
                                                  << "*.fon"
                                                  << "*.fnt"
                                                  << "*.def"
                                                  << "*.df");
    return filter;
}

const QString& Project::GetGraphicsFileExtensionFilter()
{
    static const QString filter("*.psd");
    return filter;
}

const QString& Project::Get3dFileExtensionFilter()
{
    static const QString filter("*.sc2");
    return filter;
}

const QString& Project::GetUIFileExtensionFilter()
{
    static const QString filter("*.yaml");
    return filter;
}

const QString& Project::GetUIFileExtension()
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

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
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
    UIControlSystem::Instance()->SetRtl(isRtl);

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void Project::SetBiDiSupport(bool support)
{
    UIControlSystem::Instance()->SetBiDiSupportEnabled(support);

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

void Project::SetGlobalStyleClasses(const QString& classesStr)
{
    Vector<String> tokens;
    Split(classesStr.toStdString(), " ", tokens);

    UIControlSystem::Instance()->GetStyleSheetSystem()->ClearGlobalClasses();
    for (String& token : tokens)
    {
        UIControlSystem::Instance()->GetStyleSheetSystem()->AddGlobalClass(FastName(token));
    }

    for (auto& document : documentGroup->GetDocuments())
    {
        document->RefreshAllControlProperties();
        document->RefreshLayout();
    }
}

QString Project::GetResourceDirectory() const
{
    return QString::fromStdString(properties.resourceDirectory);
}

void Project::OnReloadSprites()
{
    for (auto& document : documentGroup->GetDocuments())
    {
        if (!documentGroup->TryCloseDocument(document))
        {
            return;
        }
    }

    view->ExecDialogReloadSprites(spritesPacker.get());
}

void Project::OnReloadSpritesFinished()
{
    Sprite::ReloadSprites();
}

bool Project::TryCloseAllDocuments()
{
    auto documents = documentGroup->GetDocuments();
    bool hasUnsaved = std::find_if(documents.begin(), documents.end(), [](Document* document) { return document->CanSave(); }) != documents.end();

    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
        view->mainWindow,
        tr("Save changes"),
        tr("Some files has been modified.\n"
           "Do you want to save your changes?"),
        QMessageBox::SaveAll | QMessageBox::NoToAll | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::SaveAll)
        {
            documentGroup->SaveAllDocuments();
        }
    }

    for (auto& document : documentGroup->GetDocuments())
    {
        documentGroup->CloseDocument(document);
    }

    return true;
}

void Project::ConvertPathsAfterParse(ProjectProperties& properties)
{
    if (properties.projectFile.IsEmpty())
    {
        return;
    }
    FilePath projectDir = FilePath(properties.projectFile).GetDirectory();
    properties.resourceDirectory = FilePath(projectDir + properties.resourceDirectory).GetAbsolutePathname();
    properties.intermediateResourceDirectory = FilePath(projectDir + properties.intermediateResourceDirectory).GetAbsolutePathname();
    if (!properties.additionalResourceDirectory.empty())
    {
        properties.additionalResourceDirectory = FilePath(projectDir + properties.additionalResourceDirectory).GetAbsolutePathname();
    }

    for (auto& gfxDir : properties.gfxDirectories)
    {
        gfxDir.first = FilePath(projectDir + gfxDir.first).GetAbsolutePathname();
    }

    properties.uiDirectory = FilePath(projectDir + properties.uiDirectory).GetAbsolutePathname();
    properties.fontsDirectory = FilePath(projectDir + properties.fontsDirectory).GetAbsolutePathname();
    properties.fontsConfigsDirectory = FilePath(projectDir + properties.fontsConfigsDirectory).GetAbsolutePathname();
    properties.textsDirectory = FilePath(projectDir + properties.textsDirectory).GetAbsolutePathname();
}

void Project::ConvertPathsBeforeEmit(ProjectProperties& properties)
{
    FilePath projectDir = FilePath(properties.projectFile).GetDirectory();
    properties.resourceDirectory = FilePath(properties.resourceDirectory).GetRelativePathname(projectDir);
    properties.intermediateResourceDirectory = FilePath(properties.intermediateResourceDirectory).GetRelativePathname(projectDir);
    properties.additionalResourceDirectory = FilePath(properties.additionalResourceDirectory).GetRelativePathname(projectDir);

    for (auto& gfxDir : properties.gfxDirectories)
    {
        gfxDir.first = FilePath(gfxDir.first).GetRelativePathname(projectDir);
    }

    properties.uiDirectory = FilePath(properties.uiDirectory).GetRelativePathname(projectDir);
    properties.fontsDirectory = FilePath(properties.fontsDirectory).GetRelativePathname(projectDir);
    properties.fontsConfigsDirectory = FilePath(properties.fontsConfigsDirectory).GetRelativePathname(projectDir);
    properties.textsDirectory = FilePath(properties.textsDirectory).GetRelativePathname(projectDir);
}

void Project::FindFileInProject()
{
    QString filePath = FindFileDialog::GetFilePath(projectStructure.get(), "yaml", view->mainWindow);
    if (filePath.isEmpty())
    {
        return;
    }
    view->SelectFile(filePath);
    documentGroup->AddDocument(filePath);
}

void Project::SetAssetCacheClient(DAVA::AssetCacheClient* newCacheClient)
{
    spritesPacker->SetCacheClient(newCacheClient, "QuickEd.ReloadSprites");
}

QString Project::GetProjectPath() const
{
    return QString::fromStdString(properties.projectFile.GetAbsolutePathname());
}

const QString& Project::GetProjectDirectory() const
{
    return projectDirectory;
}

const QString& Project::GetProjectName() const
{
    return projectName;
}
