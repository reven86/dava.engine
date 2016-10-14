#include "Classes/Qt/Application/REApplication.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    REApplication app(std::move(cmdline));
    return app.Run();
}
