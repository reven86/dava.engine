#include "Application/QEApplication.h"
#include "Debug/DVAssertDefaultHandlers.h"

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
    DAVA::Assert::SetupDefaultHandlers();
    QEApplication app(std::move(cmdline));
    return app.Run();
}
