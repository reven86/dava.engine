/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <DAVAEngine.h>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QColorDialog>
#include <QDateTime>
#include <QPushButton>
#include <QUndoStack>
#include <QUndoGroup>

//////////////////////////////////////////////////////////////////////////
#include "fontmanagerdialog.h"
#include "FileSystem/FileSystem.h"
#include "Helpers/ResourcesManageHelper.h"
#include "Dialogs/localizationeditordialog.h"
#include "Grid/GridVisualizer.h"
#include "EditorFontManager.h"
//////////////////////////////////////////////////////////////////////////

#include "Project.h"
#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "UI/Document.h"
#include "UI/DocumentWidgets.h"
#include "UI/UIPackageLoader.h"
#include "Utils/QtDavaConvertion.h"
#include "Model/PackageHierarchy/PackageNode.h"

const QString APP_NAME = "QuickEd";
const QString APP_COMPANY = "DAVA";
const QString APP_GEOMETRY = "geometry";
const QString APP_STATE = "windowstate";

static const char* COLOR_PROPERTY_ID = "color";


using namespace DAVA;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , backgroundFrameUseCustomColorAction(nullptr)
    , backgroundFrameSelectCustomColorAction(nullptr)
    , project(nullptr)
    , activeDocument(nullptr)
    , documentWidgets(nullptr)
{
    undoGroup = new QUndoGroup(this);
    undoAction = undoGroup->createUndoAction(undoGroup);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/Icons/edit_undo.png"));

    redoAction = undoGroup->createRedoAction(undoGroup);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/Icons/edit_redo.png"));

    ui->setupUi(this);

    ui->mainToolbar->addAction(undoAction);
    ui->mainToolbar->addAction(redoAction);

	setWindowTitle(ResourcesManageHelper::GetProjectTitle());
    
    ui->tabBar->setTabsClosable(true);
    ui->tabBar->setUsesScrollButtons(true);
    connect(ui->tabBar, SIGNAL(currentChanged(int)), this, SLOT(CurrentTabChanged(int)));
    connect(ui->tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(TabCloseRequested(int)));
 
    setUnifiedTitleAndToolBarOnMac(true);

    connect(ui->actionFontManager, SIGNAL(triggered()), this, SLOT(OnOpenFontManager()));
    connect(ui->actionLocalizationManager, SIGNAL(triggered()), this, SLOT(OnOpenLocalizationManager()));


    connect(ui->fileSystemDockWidget, SIGNAL(OpenPackageFile(const QString&)), this, SLOT(OnOpenPackageFile(const QString&)));

	InitMenu();
	RestoreMainWindowState();
    
    ui->fileSystemDockWidget->setEnabled(false);
    
    documentWidgets = new DocumentWidgets(this, ui->packageWidget, ui->propertiesWidget, ui->previewWidget, ui->libraryWidget);
    
    RebuildRecentMenu();
}

MainWindow::~MainWindow()
{
	SaveMainWindowState();
    delete ui;
    delete undoAction;
    delete redoAction;
    delete undoGroup;
    
    delete documentWidgets;
    documentWidgets = nullptr;
}

void MainWindow::SaveMainWindowState()
{
	QSettings settings(APP_COMPANY, APP_NAME);
    settings.setValue(APP_GEOMETRY, saveGeometry());
    settings.setValue(APP_STATE, saveState());
}

void MainWindow::RestoreMainWindowState()
{
	QSettings settings(APP_COMPANY, APP_NAME);
	// Check settings befor applying it
	if (!settings.value(APP_GEOMETRY).isNull() && settings.value(APP_GEOMETRY).isValid())
	{
    	restoreGeometry(settings.value(APP_GEOMETRY).toByteArray());
	}
	if (!settings.value(APP_STATE).isNull() && settings.value(APP_STATE).isValid())
	{
    	restoreState(settings.value(APP_STATE).toByteArray());
	}
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    // Ask user to save the project before closing.
    if (!CloseProject())
    {
        event->ignore();
    }
    else
    {
    //    writeSettings();
        event->accept();
    }
}

void MainWindow::CurrentTabChanged(int index)
{
    if (activeDocument)
    {
        activeDocument->DisconnectFromWidgets(documentWidgets);
        disconnect(activeDocument->UndoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(OnCleanChanged(bool)));
    }
    
    activeDocument = GetTabDocument(ui->tabBar->currentIndex());
    
    if (activeDocument)
    {
        activeDocument->ConnectToWidgets(documentWidgets);
        connect(activeDocument->UndoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(OnCleanChanged(bool)));
    }

    UpdateSaveButtons();
}

void MainWindow::TabCloseRequested(int index)
{
    CloseTab(index);
    UpdateSaveButtons();
}

bool MainWindow::CloseTab(int index)
{
    Document *document = GetTabDocument(index);
//     if (!project->SavePackage(document->Package()))
//         return false;
    
    ui->tabBar->removeTab(index);
    undoGroup->removeStack(document->UndoStack());
    SafeDelete(document);
    return true;
}

bool MainWindow::CloseAllTabs()
{
    while(ui->tabBar->count())
    {
        if (!CloseTab(0))
            return false;
    }

    return true;
}

void MainWindow::OnOpenFontManager()
{
    FontManagerDialog *fontManagerDialog = new FontManagerDialog();
    if (fontManagerDialog->exec())
    {
        delete fontManagerDialog;
    }
}

void MainWindow::OnOpenLocalizationManager()
{
    LocalizationEditorDialog *localizationManagerDialog = new LocalizationEditorDialog();
    if (localizationManagerDialog->exec())
    {
        delete localizationManagerDialog;
    }
}

void MainWindow::OnShowHelp()
{
	FilePath docsPath = ResourcesManageHelper::GetDocumentationPath().toStdString() + "index.html";
	QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
	QDesktopServices::openUrl(QUrl(docsFile));
}

void MainWindow::InitMenu()
{
    SetupViewMenu();

	connect(ui->actionNew_project, SIGNAL(triggered()), this, SLOT(OnNewProject()));
	connect(ui->actionSaveDocument, SIGNAL(triggered()), this, SLOT(OnSaveDocument()));
	connect(ui->actionSaveAllDocuments, SIGNAL(triggered()), this, SLOT(OnSaveAllDocuments()));
    connect(ui->actionOpen_project, SIGNAL(triggered()), this, SLOT(OnOpenProject()));
	connect(ui->actionClose_project, SIGNAL(triggered()), this, SLOT(OnCloseProject()));

	connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(OnExitApplication()));
    connect(ui->menuRecent, SIGNAL(triggered(QAction *)), this, SLOT(RecentMenuTriggered(QAction *)));

	// Remap zoom in/out shorcuts for windows platform
#if defined(__DAVAENGINE_WIN32__)
	QList<QKeySequence> shortcuts;
	shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Equal));
	shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Plus));
	ui->actionZoomIn->setShortcuts(shortcuts);
#endif

	//Help contents dialog
    connect(ui->actionHelp, SIGNAL(triggered()), this, SLOT(OnShowHelp()));

    // Reload.
    connect(ui->actionRepack_And_Reload, SIGNAL(triggered()), this, SLOT(OnRepackAndReloadSprites()));

    // Pixelization.
    ui->actionPixelized->setChecked(EditorSettings::Instance()->IsPixelized());
    connect(ui->actionPixelized, SIGNAL(triggered()), this, SLOT(OnPixelizationStateChanged()));
    UpdateMenu();
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    ui->menuView->addAction(ui->propertiesWidget->toggleViewAction());
    ui->menuView->addAction(ui->fileSystemDockWidget->toggleViewAction());
    ui->menuView->addAction(ui->packageWidget->toggleViewAction());
    ui->menuView->addAction(ui->libraryWidget->toggleViewAction());
    ui->menuView->addAction(ui->consoleDockWidget->toggleViewAction());

    ui->menuView->addSeparator();
    ui->menuView->addAction(ui->mainToolbar->toggleViewAction());
    
    // Setup the Background Color menu.
    QMenu* setBackgroundColorMenu = new QMenu("Background Color");
    ui->menuView->addSeparator();
    ui->menuView->addMenu(setBackgroundColorMenu);

    static const struct
    {
        QColor color;
        QString colorName;
    } colorsMap[] =
    {
        { Qt::black, "Black" },
        { QColor(0x33, 0x33, 0x33, 0xFF), "Default" },
        { QColor(0x53, 0x53, 0x53, 0xFF), "Dark Gray" },
        { QColor(0xB8, 0xB8, 0xB8, 0xFF), "Medium Gray" },
        { QColor(0xD6, 0xD6, 0xD6, 0xFF), "Light Gray" },
    };
    
    Color curBackgroundColor = EditorSettings::Instance()->GetCurrentBackgroundFrameColor();
    int32 itemsCount = COUNT_OF(colorsMap);
    
    bool isCustomColor = true;
    for (int32 i = 0; i < itemsCount; i ++)
    {
        QAction* colorAction = new QAction(colorsMap[i].colorName, setBackgroundColorMenu);
		colorAction->setProperty(COLOR_PROPERTY_ID, colorsMap[i].color);
        
        Color curColor = QColorToColor(colorsMap[i].color);
        if (curColor == curBackgroundColor)
        {
            isCustomColor = false;
        }

        colorAction->setCheckable(true);
        colorAction->setChecked(curColor == curBackgroundColor);
        
        backgroundFramePredefinedColorActions.append(colorAction);
		setBackgroundColorMenu->addAction(colorAction);
	}
	
    backgroundFrameUseCustomColorAction = new QAction("Custom", setBackgroundColorMenu);
	backgroundFrameUseCustomColorAction->setProperty(COLOR_PROPERTY_ID, ColorToQColor(curBackgroundColor));
    backgroundFrameUseCustomColorAction->setCheckable(true);
    backgroundFrameUseCustomColorAction->setChecked(isCustomColor);
    setBackgroundColorMenu->addAction(backgroundFrameUseCustomColorAction);
    
    setBackgroundColorMenu->addSeparator();
    
    backgroundFrameSelectCustomColorAction = new QAction("Select Custom Color...", setBackgroundColorMenu);
    setBackgroundColorMenu->addAction(backgroundFrameSelectCustomColorAction);
    
    connect(setBackgroundColorMenu, SIGNAL(triggered(QAction*)), this, SLOT(SetBackgroundColorMenuTriggered(QAction*)));

    // Another actions below the Set Background Color.
    ui->menuView->addAction(ui->actionZoomIn);
    ui->menuView->insertSeparator(ui->actionZoomIn);
    ui->menuView->addAction(ui->actionZoomOut);
}

void MainWindow::UpdateMenu()
{
    bool projectCreated = false;//HierarchyTreeController::Instance()->GetTree().IsProjectCreated();
    bool projectNotEmpty = false;//(HierarchyTreeController::Instance()->GetTree().GetPlatforms().size() > 0);

    UpdateSaveButtons();

	ui->actionClose_project->setEnabled(projectCreated);
    ui->actionFontManager->setEnabled(projectNotEmpty);
    ui->actionLocalizationManager->setEnabled(projectNotEmpty);

    // Reload.
    ui->actionRepack_And_Reload->setEnabled(projectNotEmpty);
}

void MainWindow::OnNewProject()
{
	if (!CloseProject())
		return;
	
	QString projectDir = QFileDialog::getExistingDirectory(this, tr("Choose new project folder"),
											ResourcesManageHelper::GetDefaultDirectory());
				
	if (projectDir.isNull() || projectDir.isEmpty())
		return;
	// Convert directory path into Unix-style path
	projectDir = ResourcesManageHelper::ConvertPathToUnixStyle(projectDir);

//	if (!HierarchyTreeController::Instance()->NewProject(projectDir))
//	{
//		QMessageBox msgBox;
//		msgBox.setText(tr("Error while creating project"));
//		msgBox.exec();
//	}
    ui->fileSystemDockWidget->SetProjectDir(projectDir);
}

void MainWindow::RebuildRecentMenu()
{
    ui->menuRecent->clear();
    // Get up to date count of recent project actions
    int32 projectCount = EditorSettings::Instance()->GetLastOpenedCount();
    if (projectCount > 0)
    {
        for(int32 i = 0; i < projectCount; ++i)
        {
            QString projectPath = QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str());
            QAction *recentProject = new QAction(projectPath, this);
            recentProject->setData(projectPath);
            ui->menuRecent->addAction(recentProject);
        }
    }
    ui->menuRecent->setEnabled(projectCount > 0);
}

void MainWindow::RecentMenuTriggered(QAction *recentProjectAction)
{
    QString projectPath = recentProjectAction->data().toString();

    if (projectPath.isNull())
        return;
    
    OpenProject(projectPath);
}

bool MainWindow::CheckAndUnlockProject(const QString& projectPath)
{
    if (!FileSystem::Instance()->IsFileLocked(projectPath.toStdString()))
    {
        // Nothing to unlock.
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText(QString(tr("The project file %1 is locked by other user. Do you want to unlock it?").arg(projectPath)));
    QAbstractButton *unlockButton = msgBox.addButton(tr("Unlock"), QMessageBox::YesRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    msgBox.exec();

    if (msgBox.clickedButton() != unlockButton)
    {
        return false;
    }

    // Check whether it is possible to unlock project file.
    if (!FileSystem::Instance()->LockFile(projectPath.toStdString(), false))
    {
        QMessageBox errorBox;
        errorBox.setText(QString(tr("Unable to unlock project file %1. Please check whether the project is opened in another QuickEd and close it, if yes.").arg(projectPath)));
        errorBox.exec();
        
        return false;
    }

    return true;
}

void MainWindow::DoSaveProject(bool changesOnly)
{
    // TODO
//	QString projectPath = HierarchyTreeController::Instance()->GetTree().GetActiveProjectPath();
//
//	if (projectPath.isNull() || projectPath.isEmpty())
//		return;
//
//	HierarchyTreeController* controller = HierarchyTreeController::Instance();
//	bool saveSucceeded = changesOnly ? controller->SaveOnlyChangedScreens(projectPath) :
//		controller->SaveAll(projectPath);
//
//	if (saveSucceeded)
//	{
//		// If project was successfully saved - we should save new project file path
//		// and add this project to recent files list
//		UpdateProjectSettings(ResourcesManageHelper::GetProjectFilePath(projectPath));
//	}
//	else
//	{
//		QMessageBox msgBox;
//		msgBox.setText(tr("Error while saving project"));
//		msgBox.exec();
//	}
}

void MainWindow::OnOpenProject()
{
	QString projectPath = QFileDialog::getOpenFileName(this, tr("Select a project file"),
														ResourcesManageHelper::GetDefaultDirectory(),
														tr( "Project (*.uieditor)"));
    if (projectPath.isNull() || projectPath.isEmpty())
        return;
    
	// Convert file path into Unix-style path
	projectPath = ResourcesManageHelper::ConvertPathToUnixStyle(projectPath);
    
    OpenProject(projectPath);
}

void MainWindow::OnSaveDocument()
{
    Document * document = GetCurrentTabDocument();
    if (!document || !document->IsModified())
        return;

    DVVERIFY(project->SavePackage(activeDocument->GetPackage()));
    activeDocument->ClearModified();
}

void MainWindow::OnSaveAllDocuments()
{
	for (int i = 0; i < ui->tabBar->count(); ++i)
    {
        Document * document = GetTabDocument(i);
        if (!document)
            continue;
        DVVERIFY(project->SavePackage(document->GetPackage()));
        document->ClearModified();
    }
}

void MainWindow::OnCloseProject()
{
	CloseProject();
}

void MainWindow::OnExitApplication()
{
	if (CloseProject())
	{
		QCoreApplication::exit();
	}
}

void MainWindow::OnOpenPackageFile(const QString &path)
{
    if (project)
    {
        if (!path.isEmpty())
        {
            int index = GetTabIndexByPath(path);
            if (index == -1)
            {
                RefPtr<PackageNode> package = project->OpenPackage(path);
                if (package.Get() != nullptr)
                {
                    index = CreateTabContent(package.Get());
                }
            }
            
            if (index != -1)
                ui->tabBar->setCurrentIndex(index);
        }
        
    }
}

void MainWindow::OpenProject(const QString &path)
{
    CloseProject();
    
    if (!CheckAndUnlockProject(path))
        return;
    
    project = new Project();
    if (project->Open(path))
    {
        UpdateProjectSettings(path);
        RebuildRecentMenu();
        ui->fileSystemDockWidget->SetProjectDir(path);
        ui->fileSystemDockWidget->setEnabled(true);
        //CommandsController::Instance()->CleanupUndoRedoStack();
        //	if (HierarchyTreeController::Instance()->Load(path))
        //        return true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Error while loading project"));
        msgBox.exec();
    }
}

bool MainWindow::CloseProject()
{
    if (project)
    {
        if (!CloseAllTabs())
            return false;
        
        bool hasUnsavedChanged = false;//(HierarchyTreeController::Instance()->HasUnsavedChanges() ||
//                                 PreviewController::Instance()->HasUnsavedChanges());
        
        if (hasUnsavedChanged)
        {
            int ret = QMessageBox::warning(this, qApp->applicationName(),
                                           tr("The project has been modified.\n"
                                              "Do you want to save your changes?"),
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                           QMessageBox::Save);
            if (ret == QMessageBox::Cancel)
                return false;
            else if (ret == QMessageBox::Save)
                OnSaveDocument();
        }
        
        ui->fileSystemDockWidget->setEnabled(false);
        //HierarchyTreeController::Instance()->CloseProject();
        // Update project title
        this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());
        
        delete project;
        project = nullptr;
	}
    UpdateSaveButtons();
	return true;
}

void MainWindow::UpdateProjectSettings(const QString& projectPath)
{
	// Add file to recent project files list
	EditorSettings::Instance()->AddLastOpenedFile(projectPath.toStdString());
	
	// Save to settings default project directory
	QFileInfo fileInfo(projectPath);
	QString projectDir = fileInfo.absoluteDir().absolutePath();
	EditorSettings::Instance()->SetProjectPath(projectDir.toStdString());

	// Update window title
	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle(projectPath));
    
    // Apply the pixelization value.
    Texture::SetPixelization(EditorSettings::Instance()->IsPixelized());
}

void MainWindow::OnRepackAndReloadSprites()
{
    RepackAndReloadSprites();
}

void MainWindow::OnPixelizationStateChanged()
{
    bool isPixelized = ui->actionPixelized->isChecked();
    EditorSettings::Instance()->SetPixelized(isPixelized);

    Texture::SetPixelization(isPixelized);
}

void MainWindow::RepackAndReloadSprites()
{
    Set<String> errorsSet;// = HierarchyTreeController::Instance()->RepackAndReloadSprites();

    if (!errorsSet.empty())
	{
// 		ErrorsListDialog errorsDialog;
// 		errorsDialog.InitializeErrorsList(errorsSet);
// 		errorsDialog.exec();
	}
}

void MainWindow::SetBackgroundColorMenuTriggered(QAction* action)
{
    Color newColor;

    if (action == backgroundFrameSelectCustomColorAction)
    {
        // Need to select new Background Frame color.
        QColor curColor = ColorToQColor(EditorSettings::Instance()->GetCustomBackgroundFrameColor());
        QColor color = QColorDialog::getColor(curColor, this, "Select color", QColorDialog::ShowAlphaChannel);
        if (color.isValid() == false)
        {
            return;
        }

        newColor = QColorToColor(color);
        EditorSettings::Instance()->SetCustomBackgroundFrameColor(newColor);
    }
    else if (action == backgroundFrameUseCustomColorAction)
    {
        // Need to use custom Background Frame Color set up earlier.
        newColor = EditorSettings::Instance()->GetCustomBackgroundFrameColor();
    }
    else
    {
        // Need to use predefined Background Frame Color.
        newColor = QColorToColor(action->property(COLOR_PROPERTY_ID).value<QColor>());
    }

    EditorSettings::Instance()->SetCurrentBackgroundFrameColor(newColor);
    //ScreenWrapper::Instance()->SetBackgroundFrameColor(newColor);
    
    // Update the check marks.
    bool colorFound = false;
    foreach (QAction* colorAction, backgroundFramePredefinedColorActions)
    {
        Color color = QColorToColor(colorAction->property(COLOR_PROPERTY_ID).value<QColor>());
        if (color == newColor)
        {
            colorAction->setChecked(true);
            colorFound = true;
        }
        else
        {
            colorAction->setChecked(false);
        }
    }

    // In case we don't found current color in predefined ones - select "Custom" menu item.
    backgroundFrameUseCustomColorAction->setChecked(!colorFound);
}

void MainWindow::UpdateSaveButtons()
{
    bool enableSingleSave = activeDocument && activeDocument->IsModified();
    ui->actionSaveDocument->setEnabled(enableSingleSave);


    bool enableMultiSave = ui->tabBar->count() > 0;
    ui->actionSaveAllDocuments->setEnabled(enableMultiSave);
}

int MainWindow::CreateTabContent(PackageNode *package)
{
    int oldIndex = ui->tabBar->currentIndex();
    Document *document = new Document(project, package, this);
    
    QVariant var;
    var.setValue<Document *>(document);

    ui->tabBar->blockSignals(true);//block currentTabChanged signal, because tabData is empty
    int index = ui->tabBar->addTab(QString(document->PackageFilePath().GetBasename().c_str()));
    ui->tabBar->setTabData(index, var);
    ui->tabBar->blockSignals(false);
    
    undoGroup->addStack(document->UndoStack());
    if (oldIndex < 0)
    {
        CurrentTabChanged(index);
    }
    
    return index;
}

Document *MainWindow::GetCurrentTabDocument() const
{
    int index = ui->tabBar->currentIndex();
    if(index>=0)
    {
        return ui->tabBar->tabData(index).value<Document*>();
    }

    return nullptr;
}

int MainWindow::GetTabIndexByPath(const QString &fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    FilePath davaPath(canonicalFilePath.toStdString());
    
    for(int index = 0; index < ui->tabBar->count(); ++index)
    {
        Document *document = ui->tabBar->tabData(index).value<Document *>();
        if (document->PackageFilePath() == davaPath)
            return index;
    }
    
    return -1;
}

Document *MainWindow::GetTabDocument(int index) const
{
    Document *document = ui->tabBar->tabData(index).value<Document *>();
    return document;
}

void MainWindow::OnCleanChanged(bool clean)
{
    QString tabText(activeDocument->PackageFilePath().GetBasename().c_str());
    if(!clean)
        tabText.append("*");

    ui->tabBar->setTabText(ui->tabBar->currentIndex(), tabText);

    UpdateSaveButtons();
}
