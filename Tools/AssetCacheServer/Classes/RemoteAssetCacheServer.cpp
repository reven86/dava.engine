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
    connect(ui->ipLineEdit, &QLineEdit::editingFinished,
            this, &RemoteAssetCacheServer::OnParanetersChanged);
    connect(ui->portSpinBox, &QSpinBox::editingFinished,
            this, &RemoteAssetCacheServer::OnParanetersChanged);
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

void RemoteAssetCacheServer::OnParanetersChanged()
{
    emit ParametersChanged();
}
