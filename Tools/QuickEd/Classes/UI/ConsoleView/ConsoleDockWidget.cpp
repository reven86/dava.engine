#include "ConsoleDockWidget.h"
#include "ui_ConsoleDockWidget.h"
#include "StringUtils.h"
#include "Utils/QtDavaConvertion.h"

ConsoleDockWidget::ConsoleDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::ConsoleDockWidget())
{
    ui->setupUi(this);
    DAVA::Logger::AddCustomOutput(this);

}

ConsoleDockWidget::~ConsoleDockWidget()
{
    DAVA::Logger::RemoveCustomOutput(this);
    delete ui;
}

void ConsoleDockWidget::Output( DAVA::Logger::eLogLevel ll, const DAVA::char8* text )
{
    Output(ll, QString(text));
}

void ConsoleDockWidget::Output( DAVA::Logger::eLogLevel ll, const QString &rawText ) const
{
    QString text = rawText;
    if (text[text.length()-1] == '\n')
        text.remove(text.length()-1, 1);

    ui->plainTextEdit->appendPlainText(text);
}
