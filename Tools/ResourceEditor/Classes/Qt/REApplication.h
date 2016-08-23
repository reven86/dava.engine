#pragma once

#include "CommandLine/CommandLineManager.h"
#include <QApplication>

class QtMainWindow;

class REApplication : public QApplication
{
public:
    REApplication(int& argc, char** argv);
    ~REApplication();

    int Run();

private:
    void RunWindow();
    void RunConsole();

private:
    QtMainWindow* mainWindow = nullptr;
    std::unique_ptr<CommandLineManager> cmdLineManager;
};
