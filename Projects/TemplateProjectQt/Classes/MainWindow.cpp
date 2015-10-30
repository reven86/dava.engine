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
#include "QtTools/DavaGLWidget/davaglwidget.h"

namespace
{
const quint8 NUMBER_OF_SCREEN(0);
}

MainWindow::MainWindow(QWidget *parent)
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

    DavaGLWidget *glWidget = new DavaGLWidget(this);
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
    UIScreen *screen = UIScreenManager::Instance()->GetScreen(NUMBER_OF_SCREEN);
    if (screen == nullptr)
    {
        return;
    }

    qint64 scaleWidth = width * dpr;
    qint64 scaleHeight = height * dpr;
    screen->SetSize(Vector2(scaleWidth, scaleHeight));
}
