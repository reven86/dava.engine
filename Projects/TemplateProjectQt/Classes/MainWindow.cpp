#include "MainWindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "QtTools/DavaGLWidget/davaglwidget.h"

namespace
{
const quint8 NUMBER_OF_SCREEN(0);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    DavaGLWidget* glWidget = new DavaGLWidget(this);
    ScopedPtr<UIScreen> davaUIScreen(new DAVA::UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(DAVA::Color(1.f, 0.f, 0.f, 1.f));
    UIScreenManager::Instance()->RegisterScreen(NUMBER_OF_SCREEN, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(NUMBER_OF_SCREEN);

    connect(glWidget, &DavaGLWidget::Resized, this, &MainWindow::OnGlWidgedResized);

    ui->verticalLayout->addWidget(glWidget);
}

void MainWindow::OnGlWidgedResized(int width, int height, int dpr)
{
    using namespace DAVA;
    UIScreen* screen = UIScreenManager::Instance()->GetScreen(NUMBER_OF_SCREEN);
    if (screen == nullptr)
    {
        return;
    }

    qint64 scaleWidth = width * dpr;
    qint64 scaleHeight = height * dpr;
    screen->SetSize(Vector2(scaleWidth, scaleHeight));
}
