#ifndef __CONSOLEDOCKWIDGET_H__
#define __CONSOLEDOCKWIDGET_H__

#include <QDockWidget>
#include "DAVAEngine.h"

namespace Ui {
    class ConsoleDockWidget;
}
/*
class LoggerToConsole : public DAVA::LoggerOutput
{
public:
LoggerToConsole();
~LoggerToConsole();


};
*/
class ConsoleDockWidget : public QDockWidget
                        , private DAVA::LoggerOutput
{
public:
    explicit ConsoleDockWidget(QWidget *parent = NULL);
    ~ConsoleDockWidget();

private:
    // DAVA::LoggerOutput
    virtual void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) const override;
    virtual void Output(DAVA::Logger::eLogLevel ll, const DAVA::char16* text) const override;
private:
    void Output(DAVA::Logger::eLogLevel ll, const QString &text) const;
private:
    Ui::ConsoleDockWidget *ui;
};

#endif // __CONSOLEDOCKWIDGET_H__
