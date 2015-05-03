#include "DAVAEngine.h"

#include "MainWindow.h"
#include <QApplication>

void FrameworkWillTerminate()
{
}

void FrameworkDidLaunched()
{
}

int main(int argc, char *argv[])
{
    DAVA::Core::Run(argc, argv);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
