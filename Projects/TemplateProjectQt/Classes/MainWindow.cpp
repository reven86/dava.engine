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
}

MainWindow::~MainWindow()
{
    delete ui;
}
