#include "ConsoleDockWidget.h"
#include "ui_ConsoleDockWidget.h"
#include "StringUtils.h"
#include "Utils/QtDavaConvertion.h"
#include <Classes/Result.h>

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
    QString color;
    switch (ll)
    {
    default:                                color = "black"; break;
    case DAVA::Logger::LEVEL_DEBUG:         color = "dark green"; break;
    case DAVA::Logger::LEVEL_INFO:          color = "dark blue"; break;
    case DAVA::Logger::LEVEL_WARNING:       color = "orange"; break;
    case DAVA::Logger::LEVEL_ERROR:         color = "red"; break;
    }
    ui->plainTextEdit->appendHtml("<font color=\"" + color + "\">" + rawText.trimmed() + "</font>");
}
