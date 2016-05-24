#include "RemoteServerWidget.h"
#include "ui_RemoteServerWidget.h"

#include <QValidator>

RemoteServerWidget::RemoteServerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RemoteServerWidget)
{
    ui->setupUi(this);

    ui->ipLineEdit->setText(DAVA::AssetCache::LOCALHOST.c_str());

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
