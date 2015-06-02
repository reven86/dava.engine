#include "DeviceLogWidget.h"

#include "ui_DeviceLogWidget.h"

#include "Classes/Qt/DockConsole/LogModel.h"


DeviceLogWidget::DeviceLogWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::DeviceLogWidget())
{
    ui->setupUi(this);

    ui->logger->SetRegisterLoggerAsLocal(false);
}

DeviceLogWidget::~DeviceLogWidget() {}

void DeviceLogWidget::AppendText(const QString& text, DAVA::Logger::eLogLevel ll)
{
    LogModel *logModel = ui->logger->Model();
    if ( logModel != NULL )
    {
        logModel->AddMessage( ll, text );
    }
}
