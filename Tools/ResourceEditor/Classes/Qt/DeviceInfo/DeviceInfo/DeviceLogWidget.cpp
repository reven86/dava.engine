#include "DeviceLogWidget.h"

#include "ui_DeviceLogWidget.h"

DeviceLogWidget::DeviceLogWidget(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::DeviceLogWidget())
{
    ui->setupUi(this);
}

DeviceLogWidget::~DeviceLogWidget() {}

void DeviceLogWidget::AppendText(const QString& text)
{
    ui->textEdit->append(text);
}
