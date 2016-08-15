#include "RemoteServerWidget.h"
#include "ui_RemoteServerWidget.h"

#include <QValidator>

RemoteServerWidget::RemoteServerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RemoteServerWidget)
{
    ui->setupUi(this);

    ui->ipLineEdit->setText(DAVA::AssetCache::GetLocalHost().c_str());

    connect(ui->removeServerButton, &QPushButton::clicked,
            this, &RemoteServerWidget::RemoveLater);
    connect(ui->ipLineEdit, &QLineEdit::textChanged,
            this, &RemoteServerWidget::OnParametersChanged);
    connect(ui->enabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnChecked(int)));
}

RemoteServerWidget::RemoteServerWidget(const RemoteServerParams& newServer, QWidget* parent)
    : RemoteServerWidget(parent)
{
    ui->enabledCheckBox->setChecked(newServer.enabled);
    ui->ipLineEdit->setText(newServer.ip.c_str());
}

RemoteServerWidget::~RemoteServerWidget()
{
    delete ui;
}

RemoteServerParams RemoteServerWidget::GetServerData() const
{
    return RemoteServerParams(ui->ipLineEdit->text().toStdString(), ui->enabledCheckBox->isChecked());
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
