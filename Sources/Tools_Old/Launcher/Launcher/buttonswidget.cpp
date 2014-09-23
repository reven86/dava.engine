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


#include "buttonswidget.h"
#include "ui_buttonswidget.h"
#include "defines.h"
#include <QPushButton>

ButtonsWidget::ButtonsWidget(int rowNum, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ButtonsWidget),
    rowNumber(rowNum)
{
    ui->setupUi(this);

    SetButtonsState(BUTTONS_STATE_DISABLED_ALL);

    connect(this, SIGNAL(OnInstall(int)), parent, SLOT(OnInstall(int)));
    connect(this, SIGNAL(OnRun(int)), parent, SLOT(OnRun(int)));
    connect(this, SIGNAL(OnRemove(int)), parent, SLOT(OnRemove(int)));

    connect(ui->installButton, SIGNAL(clicked()), this, SLOT(OnInstallClicked()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(OnRemoveClicked()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(OnRunClicked()));
}

ButtonsWidget::~ButtonsWidget()
{
    SafeDelete(ui);
}

void ButtonsWidget::SetButtonsState(int state)
{
    ui->installButton->setEnabled(false);
    ui->runButton->setEnabled(false);
    ui->removeButton->setEnabled(false);

    if(state & BUTTONS_STATE_AVALIBLE)
        ui->installButton->setEnabled(true);

    if(state & BUTTONS_STATE_INSTALLED)
    {
        ui->runButton->setEnabled(true);
        ui->removeButton->setEnabled(true);
    }
}

void ButtonsWidget::OnRunClicked()
{
    emit OnRun(rowNumber);
}

void ButtonsWidget::OnRemoveClicked()
{
    emit OnRemove(rowNumber);
}

void ButtonsWidget::OnInstallClicked()
{
    emit OnInstall(rowNumber);
}
