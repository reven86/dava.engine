#include "Application/QEApplication.h"

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
    QEApplication app(std::move(cmdline));
    return app.Run();
}
