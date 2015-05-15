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


#include "RemoteAssetCacheServer.h"
#include "ui_RemoteAssetCacheServer.h"

#include <QValidator>

ServerData::ServerData()
    : ip("127.0.0.1")
    , port(80)
{
}

ServerData::ServerData(QString newIp, quint16 newPort)
    : ip(newIp)
    , port(newPort)
{
}


RemoteAssetCacheServer::RemoteAssetCacheServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RemoteAssetCacheServer)
{
    ui->setupUi(this);

    QRegExp ipRegExp("(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])[.]){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegExp);
    ui->ipLineEdit->setValidator(ipValidator);
    ui->ipLineEdit->setText("127.0.0.1");

    connect(ui->removeServerButton, &QPushButton::clicked,
            this, &RemoteAssetCacheServer::RemoveLater);
    connect(ui->ipLineEdit, &QLineEdit::textChanged,
            this, &RemoteAssetCacheServer::OnParametersChanged);
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnParametersChanged()));
}

RemoteAssetCacheServer::RemoteAssetCacheServer(ServerData &newServer, QWidget *parent)
    : RemoteAssetCacheServer(parent)
{
    ui->ipLineEdit->setText(newServer.ip);
    ui->portSpinBox->setValue(newServer.port);
}

RemoteAssetCacheServer::~RemoteAssetCacheServer()
{
    delete ui;
}

ServerData RemoteAssetCacheServer::GetServerData() const
{
    return ServerData(ui->ipLineEdit->text(), ui->portSpinBox->value());
}

bool RemoteAssetCacheServer::IsCorrectData()
{
    QString ip(ui->ipLineEdit->text());
    QStringList ipList = ip.split(".", QString::SkipEmptyParts);
    if (ipList.count() != 4)
    {
        return false;
    }

    return true;
}

void RemoteAssetCacheServer::OnParametersChanged()
{
    emit ParametersChanged();
}
