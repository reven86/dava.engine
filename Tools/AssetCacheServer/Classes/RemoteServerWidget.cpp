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

#include "RemoteServerWidget.h"
#include "ui_RemoteServerWidget.h"

#include <QValidator>

RemoteServerWidget::RemoteServerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RemoteServerWidget)
{
    ui->setupUi(this);

    ui->ipLineEdit->setText("127.0.0.1");

    connect(ui->removeServerButton, &QPushButton::clicked,
            this, &RemoteServerWidget::RemoveLater);
    connect(ui->ipLineEdit, &QLineEdit::textChanged,
            this, &RemoteServerWidget::OnParametersChanged);
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnParametersChanged()));
    connect(ui->enabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnChecked(int)));
}

RemoteServerWidget::RemoteServerWidget(const ServerData& newServer, QWidget* parent)
    : RemoteServerWidget(parent)
{
    ui->enabledCheckBox->setChecked(newServer.enabled);
    ui->ipLineEdit->setText(newServer.ip.c_str());
    ui->portSpinBox->setValue(newServer.port);
    ui->portSpinBox->setEnabled(true);
}

RemoteServerWidget::~RemoteServerWidget()
{
    delete ui;
}

ServerData RemoteServerWidget::GetServerData() const
{
    return ServerData(ui->ipLineEdit->text().toStdString(), ui->portSpinBox->value(), ui->enabledCheckBox->isChecked());
}

bool RemoteServerWidget::IsCorrectData()
{
    return true;
}

void RemoteServerWidget::OnParametersChanged()
{
    emit ParametersChanged();
}

void RemoteServerWidget::OnChecked(int val)
{
    emit ServerChecked(val == Qt::Checked);
}

bool RemoteServerWidget::IsChecked() const
{
    return ui->enabledCheckBox->isChecked();
}

void RemoteServerWidget::SetChecked(bool checked)
{
    ui->enabledCheckBox->setChecked(checked);
}
