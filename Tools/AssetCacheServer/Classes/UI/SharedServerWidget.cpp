#include "SharedServerWidget.h"
#include "ui_SharedServerWidget.h"

#include <QValidator>

SharedServerWidget::SharedServerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SharedServerWidget)
{
    ui->setupUi(this);

    ui->ipLineEdit->setText(DAVA::AssetCache::GetLocalHost().c_str());

    connect(ui->enabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnChecked(int)));
}

SharedServerWidget::SharedServerWidget(const SharedServer& server, QWidget* parent)
    : SharedServerWidget(parent)
{
    ui->enabledCheckBox->setChecked(server.remoteParams.enabled);
    ui->nameLineEdit->setText(server.serverName.c_str());
    ui->ipLineEdit->setText(server.remoteParams.ip.c_str());
    poolID = server.poolID;
    serverID = server.serverID;
}

SharedServerWidget::~SharedServerWidget()
{
    delete ui;
}

void SharedServerWidget::OnParametersChanged()
{
    emit ParametersChanged();
}

void SharedServerWidget::OnChecked(int val)
{
    emit ServerChecked(val == Qt::Checked);
}

bool SharedServerWidget::IsChecked() const
{
    return ui->enabledCheckBox->isChecked();
}

void SharedServerWidget::SetChecked(bool checked)
{
    ui->enabledCheckBox->setChecked(checked);
}
