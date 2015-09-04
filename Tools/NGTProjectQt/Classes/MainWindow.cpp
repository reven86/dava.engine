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


#include "MainWindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "QtTools/FrameworkBinding/FrameworkLoop.h"

#include <QAbstractButton>
#include <QDir>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QFileSystemModel>

const quint8 MainWindow::NUMBER_OF_SCREEN(0);

namespace
{

char const * SCENE_EXTENSION = ".sc2";
char const * SCENE_FILE_FILTER = "Scene (*.sc2)";
char const * FILE_SYSTEM_ROOT = "d:\\dev\\dava.test\\SmokeTest\\";

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QFileSystemModel(this))
{
    ui->setupUi(this);

    QItemSelectionModel * selectionModel = new QItemSelectionModel(model);
    DVVERIFY(QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged,
                              this, &MainWindow::OnFileSelectionChanged));
    DVVERIFY(QObject::connect(ui->loadSceneButton, &QAbstractButton::clicked,
                              this, &MainWindow::OnLoadSceneButton));
    DVVERIFY(QObject::connect(ui->actionOpen_Scene, &QAction::triggered,
                              this, &MainWindow::OnLoadSceneAction));

    model->setRootPath(FILE_SYSTEM_ROOT);
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(FILE_SYSTEM_ROOT));
    ui->treeView->setSelectionModel(selectionModel);

    for (int i = 1; i < model->columnCount(QModelIndex()); ++i)
        ui->treeView->hideColumn(i);
    
    setWindowTitle("Template Project Qt");

    CreateGlWidget();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CreateGlWidget()
{
    using namespace DAVA;

    DavaGLWidget *glWidget = new DavaGLWidget(this);
    connect(glWidget, &DavaGLWidget::Initialized, this, &MainWindow::OnGlInitialized, Qt::QueuedConnection);
    connect(glWidget, &DavaGLWidget::Resized, this, &MainWindow::OnGlWidgedResized);
    FrameworkLoop::Instance()->SetOpenGLWindow(glWidget);

    ui->verticalLayout->addWidget(glWidget);
}

void MainWindow::LoadScene(QString const & scenePath)
{
    using namespace DAVA;

    DVASSERT(!scenePath.isEmpty());

    ScopedPtr<Scene> scene(new Scene());
    SceneFileV2::eError result = scene->LoadScene(FilePath(scenePath.toStdString()));
    if (result == DAVA::SceneFileV2::ERROR_NO_ERROR)
    {
        WASDControllerSystem * wasdSystem = new WASDControllerSystem(scene);
        wasdSystem->SetMoveSpeed(10.0f);
        uint64 wasdComponentFlag = MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT);
        uint32 wasdProcessFlag = Scene::SCENE_SYSTEM_REQUIRE_PROCESS;
        scene->AddSystem(wasdSystem, wasdComponentFlag, wasdProcessFlag);

        RotationControllerSystem * rotationSystem = new RotationControllerSystem(scene);
        uint64 rotationComponentFlag = MAKE_COMPONENT_MASK(Component::CAMERA_COMPONENT) | MAKE_COMPONENT_MASK(Component::WASD_CONTROLLER_COMPONENT);
        uint32 rotationProcessFlag = Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT;
        scene->AddSystem(rotationSystem, rotationComponentFlag, rotationProcessFlag);

        Camera * camera = new Camera();
        camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        camera->SetPosition(Vector3(-50.0f, 0.0f, 50.0f));
        camera->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        camera->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);

        Entity *cameraEntity = new Entity();
        cameraEntity->SetName("single-camera");
        cameraEntity->AddComponent(new CameraComponent(camera));
        cameraEntity->AddComponent(new WASDControllerComponent());
        cameraEntity->AddComponent(new RotationControllerComponent());
        scene->AddNode(cameraEntity);

        scene->AddCamera(camera);
        scene->SetCurrentCamera(camera);
        SafeRelease(camera);

        view->SetScene(scene);
        statusBar()->showMessage(scenePath);
    }
}

void MainWindow::OnGlInitialized()
{
    using namespace DAVA;

    ScopedPtr<UIScreen> screen(new UIScreen());
    view = new DAVA::UI3DView(screen->GetRect(), true);
    view->SetInputEnabled(true, true);

    screen->AddControl(view);
    screen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    screen->GetBackground()->SetColor(DAVA::Color(0.65f, 0.65f, 0.65f, 1.f));

    UIScreenManager::Instance()->RegisterScreen(NUMBER_OF_SCREEN, screen);
    UIScreenManager::Instance()->SetFirst(NUMBER_OF_SCREEN);
}

void MainWindow::OnGlWidgedResized(int width, int height, int dpr)
{
    using namespace DAVA;
    UIScreen *screen = UIScreenManager::Instance()->GetScreen(NUMBER_OF_SCREEN);
    if (screen == nullptr)
    {
        return;
    }

    qint64 scaleWidth = width * dpr;
    qint64 scaleHeight = height * dpr;
    screen->SetSize(Vector2(scaleWidth, scaleHeight));
}

void MainWindow::OnFileSelectionChanged(QItemSelection const & selected, QItemSelection const & deselected)
{
    DVASSERT(!selected.indexes().isEmpty());
    ui->loadSceneButton->setEnabled(model->fileName(selected.indexes().first()).endsWith(SCENE_EXTENSION));
}

void MainWindow::OnLoadSceneButton()
{
    QItemSelectionModel * selectionModel = ui->treeView->selectionModel();
    DVASSERT(nullptr != selectionModel);
    QItemSelection selection = selectionModel->selection();
    DVASSERT(!selection.indexes().isEmpty());
    QString scenePath = model->filePath(selection.indexes().first());
    DVASSERT(scenePath.endsWith(SCENE_EXTENSION));

    LoadScene(scenePath);
}

void MainWindow::OnLoadSceneAction()
{
    QString scenePath = QFileDialog::getOpenFileName(this, QStringLiteral("Choose your destiny"),
                                                     QString(FILE_SYSTEM_ROOT), QString(SCENE_FILE_FILTER));
    if (scenePath.isEmpty())
        return;

    LoadScene(scenePath);
}
