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



#include "LibraryWidget.h"
#include "LibraryFileSystemModel.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "Project/ProjectManager.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/DAEConvertAction.h"


#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QHeaderView>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QProcess>
#include <QFileSystemModel>
#include <QMenu>
#include <QAction>
#include <QFileInfo>


struct FileType
{
    FileType() {}

    FileType(const QString &n)
    {
        name = n;
    }

    
    FileType(const QString &n, const QString &f)
    {
        name = n;
        filter << f;
    }

    FileType(const QString &n, const QStringList &_filter)
    {
        name = n;
        filter = _filter;
    }

    QString name;
    QStringList filter;
};

QVector<FileType> fileTypeValues;

LibraryWidget::LibraryWidget(QWidget *parent /* = 0 */)
	: QWidget(parent)
	, curTypeIndex(-1)
{
    SetupFileTypes();
    SetupToolbar();
    SetupView();
    SetupLayout();

    ViewAsList();
    OnFilesTypeChanged(0);
}

LibraryWidget::~LibraryWidget()
{
}

void LibraryWidget::SetupFileTypes()
{
    UnorderedSet<String> sourceFiles;
    sourceFiles.insert(DAVA::TextureDescriptor::GetDescriptorExtension());
    for(auto formatType : DAVA::TextureDescriptor::sourceTextureTypes)
    {
        auto extensions = DAVA::ImageSystem::Instance()->GetExtensionsFor(formatType);
        for(auto ex : extensions)
        {
            sourceFiles.insert(ex.c_str());
        }
    }
    
    QStringList sourceImagesList;
    for(auto sf : sourceFiles)
    {
        sourceImagesList << (QString("*") + sf.c_str());
    }
    
    UnorderedSet<String> compressedFiles;
    for(auto formatType : DAVA::TextureDescriptor::compressedTextureTypes)
    {
        auto extensions = DAVA::ImageSystem::Instance()->GetExtensionsFor(formatType);
        for(auto ex : extensions)
        {
            compressedFiles.insert(ex.c_str());
        }
    }
    
    QStringList compressedImagesList;
    for(auto cf : compressedFiles)
    {
        compressedImagesList << (QString("*") + cf.c_str());
    }

    FileType allFiles("All files");
    allFiles.filter << "*.dae";
    allFiles.filter << "*.sc2";
    allFiles.filter << sourceImagesList;

    
    fileTypeValues.reserve(10);
    
    fileTypeValues.push_back(allFiles);
    QStringList models;
    models << "*.dae" << "*.sc2";
    fileTypeValues.push_back(FileType("Models", models));
    
    fileTypeValues.push_back(FileType("Source Textures", sourceImagesList));
    fileTypeValues.push_back(FileType("Compressed Textures", compressedImagesList));

    fileTypeValues.push_back(FileType("DAE", "*.dae"));
    fileTypeValues.push_back(FileType("SC2", "*.sc2"));
    fileTypeValues.push_back(FileType("TEX", QString("*") + DAVA::TextureDescriptor::GetDescriptorExtension().c_str()));
    fileTypeValues.push_back(FileType("PNG", GetExtensions(DAVA::IMAGE_FORMAT_PNG)));
    fileTypeValues.push_back(FileType("TGA", GetExtensions(DAVA::IMAGE_FORMAT_TGA)));
    fileTypeValues.push_back(FileType("JPG", GetExtensions(DAVA::IMAGE_FORMAT_JPEG)));
}

QStringList LibraryWidget::GetExtensions(DAVA::ImageFormat imageFormat) const
{
    QStringList extList;
    auto extensions = DAVA::ImageSystem::Instance()->GetExtensionsFor(imageFormat);
    for(auto &ex: extensions)
    {
        extList << QString("*") + ex.c_str();
    }
    
    return extList;
}


void LibraryWidget::SetupSignals()
{
    QObject::connect(ProjectManager::Instance(), &ProjectManager::ProjectOpened, this, &LibraryWidget::ProjectOpened);
    QObject::connect(ProjectManager::Instance(), &ProjectManager::ProjectClosed, this, &LibraryWidget::ProjectClosed);
    
    QObject::connect(filesView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &LibraryWidget::SelectionChanged);

    QObject::connect(filesView, &QTreeView::customContextMenuRequested, this, &LibraryWidget::ShowContextMenu);
    QObject::connect(filesView, &QTreeView::doubleClicked, this, &LibraryWidget::fileDoubleClicked);
}

void LibraryWidget::SetupToolbar()
{
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setMovable(false);

    filesTypeFilter = new QComboBox(toolbar);
    filesTypeFilter->setEditable(false);
    filesTypeFilter->setMinimumWidth(100);
    filesTypeFilter->setMaximumWidth(100);
    for(int i = 0; i < fileTypeValues.size(); ++i)
    {
        filesTypeFilter->addItem(fileTypeValues[i].name);
    }
    filesTypeFilter->setCurrentIndex(0);
    
    
    QIcon asListIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_list.png"));
    actionViewAsList = new QAction(asListIcon, "View as list", toolbar);
    actionViewAsList->setCheckable(true);
    actionViewAsList->setChecked(true);

    QIcon asDetailedIcon(QString::fromUtf8(":/QtIcons/all.png"));
    actionViewDetailed = new QAction(asDetailedIcon, "View detailed", toolbar);
    actionViewDetailed->setCheckable(true);
    actionViewDetailed->setChecked(false);

    QObject::connect(filesTypeFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilesTypeChanged(int)));
    QObject::connect(actionViewAsList, SIGNAL(triggered()), this, SLOT(ViewAsList()));
    QObject::connect(actionViewDetailed, SIGNAL(triggered()), this, SLOT(ViewDetailed()));
    
    toolbar->addWidget(filesTypeFilter);
    
    toolbar->addAction(actionViewAsList);
    toolbar->addAction(actionViewDetailed);
}

void LibraryWidget::SetupView()
{
    filesModel = new LibraryFileSystemModel(this);

    filesView = new LibraryTreeView(this);
    filesView->setContextMenuPolicy(Qt::CustomContextMenu);
    filesView->setDragDropMode(QAbstractItemView::DragOnly);
	filesView->setDragEnabled(true);
    filesView->setUniformRowHeights(true);
    
    filesView->setModel(filesModel);
    
    QObject::connect(filesView, SIGNAL(DragStarted()), this, SLOT(OnTreeDragStarted()));
}

void LibraryWidget::SetupLayout()
{
    // put tab bar into vertical layout
	layout = new QVBoxLayout();
	layout->addWidget(toolbar);
	layout->addWidget(filesView);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
}


void LibraryWidget::ViewAsList()
{
    viewMode = VIEW_AS_LIST;
	filesView->header()->setVisible(false);

    
    HideDetailedColumnsAtFilesView(true);
    
    actionViewAsList->setChecked(true);
    actionViewDetailed->setChecked(false);
}

void LibraryWidget::ViewDetailed()
{
    viewMode = VIEW_DETAILED;
	filesView->header()->setVisible(true);

    // Magic trick for MacOS: call function twice
    HideDetailedColumnsAtFilesView(false);
    HideDetailedColumnsAtFilesView(false);
    //EndOftrick

    actionViewAsList->setChecked(false);
    actionViewDetailed->setChecked(true);
}

void LibraryWidget::HideDetailedColumnsAtFilesView(bool hide)
{
    int columns = (hide) ? 1 : filesModel->columnCount();
    int width = filesView->geometry().width() / columns;
    
    if(!hide)
    {
        filesView->setColumnWidth(0, width);
    }
    
    for(int i = 1; i < filesModel->columnCount(); ++i)
	{
        filesView->setColumnHidden(i, hide);
        filesView->setColumnWidth(i, width);
	}
}


void LibraryWidget::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(0 == selected.count()) return;

	const QModelIndex index = selected.indexes().first();

	QFileInfo fileInfo = filesModel->fileInfo(index);

    if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
    {
        ShowPreview(fileInfo.filePath());
    }
    else
    {
        HidePreview();
    }
}

void LibraryWidget::fileDoubleClicked(const QModelIndex & index)
{
    if(SettingsManager::GetValue(Settings::General_OpenByDBClick).AsBool())
    {
        HidePreview();
        QFileInfo fileInfo = filesModel->fileInfo(index);
        if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
        {
            QtMainWindow::Instance()->OpenScene(fileInfo.absoluteFilePath());
        }
    }
}


void LibraryWidget::ShowContextMenu(const QPoint & point)
{
    HidePreview();

	const QModelIndex index = filesView->indexAt(point);
    
	if(!index.isValid()) return;
    
    QFileInfo fileInfo = filesModel->fileInfo(index);
    if(!fileInfo.isFile()) return;

    QMenu contextMenu(this);
    QVariant fileInfoAsVariant = QVariant::fromValue<QFileInfo>(fileInfo);

    DAVA::FilePath pathname = fileInfo.absoluteFilePath().toStdString();
    if(pathname.IsEqualToExtension(".sc2"))
    {
        QAction * actionAdd = contextMenu.addAction("Add Model", this, SLOT(OnAddModel()));
        QAction * actionEdit = contextMenu.addAction("Edit Model", this, SLOT(OnEditModel()));
        
        actionAdd->setData(fileInfoAsVariant);
        actionEdit->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".dae"))
    {
        QAction * actionConvert = contextMenu.addAction("Convert", this, SLOT(OnConvertDae()));
        actionConvert->setData(fileInfoAsVariant);
    }
//TODO: disabled for future realization of this code
//    else if(pathname.IsEqualToExtension(".tex"))
//    {
//        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEditTextureDescriptor()));
//        actionEdit->setData(fileInfoAsVariant);
//    }
//    else if(pathname.IsEqualToExtension(".png"))
//    {
//        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEditTextureDescriptor()));
//        actionEdit->setData(fileInfoAsVariant);
//    }
//ENDOFTODO
    
    contextMenu.addSeparator();
    QAction * actionRevealAt = contextMenu.addAction("Reveal at folder", this, SLOT(OnRevealAtFolder()));
    actionRevealAt->setData(fileInfoAsVariant);

    
    contextMenu.exec(filesView->mapToGlobal(point));
}


void LibraryWidget::OnFilesTypeChanged(int typeIndex)
{
	if(curTypeIndex == typeIndex) return;

	curTypeIndex = typeIndex;
    
    filesModel->SetExtensionFilter(fileTypeValues[curTypeIndex].filter);
	filesView->setRootIndex(filesModel->index(rootPathname));
}


void LibraryWidget::ProjectOpened(const QString &path)
{
    rootPathname = path + "/DataSource/3d/";
    
    filesModel->Load(rootPathname);
	filesView->setRootIndex(filesModel->index(rootPathname));
}

void LibraryWidget::ProjectClosed()
{
    rootPathname = QDir::rootPath();
	filesView->setRootIndex(filesModel->index(rootPathname));
    filesView->collapseAll();
}

void LibraryWidget::OnAddModel()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    SceneEditor2 *scene = QtMainWindow::Instance()->GetCurrentScene();
    if(nullptr != scene)
    {
        QtMainWindow::Instance()->WaitStart("Add object to scene", fileInfo.absoluteFilePath());
        
        scene->structureSystem->Add(fileInfo.absoluteFilePath().toStdString());
        
        QtMainWindow::Instance()->WaitStop();
    }
}

void LibraryWidget::OnEditModel()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->OpenScene(fileInfo.absoluteFilePath());
}

void LibraryWidget::OnConvertDae()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->WaitStart("DAE to SC2 conversion", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}


void LibraryWidget::OnEditTextureDescriptor()
{
    
}

void LibraryWidget::OnRevealAtFolder()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();

    ShowFileInExplorer( fileInfo.absoluteFilePath() );
}

void LibraryWidget::HidePreview() const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->HideScenePreview();
}

void LibraryWidget::ShowPreview(const QString & pathname) const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->ShowScenePreview(pathname.toStdString());
}


void LibraryWidget::OnTreeDragStarted()
{
    HidePreview();
}

