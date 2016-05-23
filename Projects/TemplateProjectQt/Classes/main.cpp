#include "Platform/Qt5/QtLayer.h"
#include "Core/Core.h"

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
#if defined(__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
#elif defined(__DAVAENGINE_WIN32__)
    DAVA::Core::Run(argc, argv);
#else
    static_assert(!"Wrong platform");
#endif
    new DAVA::QtLayer();

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return QApplication::exec();
}
